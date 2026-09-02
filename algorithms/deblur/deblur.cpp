// algorithms/deblur/main.cpp
// Image deblurring (deconvolution restoration): synthetic motion/defocus blur
// + comparison of multiple restoration algorithms.
//
// Algorithms covered:
//   Inverse     Inverse filter        (theoretical demo only, severely amplifies noise)
//   Wiener      Wiener filter         (frequency domain, with noise regularization)
//   RL          Richardson-Lucy       (iterative Bayesian, good fidelity)
//   Sharpen     Unsharp Masking       (non-blind sharpening, simple baseline)
//
// Pipeline: convolve a clean image with a known PSF (motion linear kernel / Gaussian defocus kernel)
// to synthesize blur, then restore with each algorithm, evaluated full-reference against
// the clean reference with PSNR/SSIM.
// Output: out/algorithms/deblur_compare.png + metrics table.
// Usage: deblur.exe [input_img] [mode=motion|defocus] [blur_len_or_sigma]
#include "../common/algo_utils.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// ---- PSF construction ----------------------------------------------------
// Motion blur PSF: a line segment of `length` pixels along `angle`, L1-normalized.
static cv::Mat motionPSF(int length, double angleDeg) {
    cv::Mat psf = cv::Mat::zeros(length, length, CV_32F);
    double c = (length - 1) / 2.0, rad = angleDeg * CV_PI / 180.0;
    for (int i = 0; i < length; ++i) {
        int x = (int)std::lround(c + (i - c) * std::cos(rad));
        int y = (int)std::lround(c + (i - c) * std::sin(rad));
        x = std::min(std::max(x, 0), length - 1);
        y = std::min(std::max(y, 0), length - 1);
        psf.at<float>(y, x) += 1.0f;
    }
    cv::normalize(psf, psf, 1.0, 0.0, cv::NORM_L1);
    return psf;
}

// Defocus blur PSF: Gaussian approximation (radially symmetric), L1-normalized.
static cv::Mat defocusPSF(int ksize, double sigma) {
    cv::Mat g = cv::getGaussianKernel(ksize, sigma, CV_32F);
    cv::Mat p = g * g.t();
    cv::normalize(p, p, 1.0, 0.0, cv::NORM_L1);
    return p;
}

// Produce a blurred observation via circular convolution with the PSF
// (BORDER_WRAP≈toroidal, consistent with the frequency-domain deconvolution model).
static cv::Mat blurObserve(const cv::Mat& bgr8, const cv::Mat& psf) {
    cv::Mat blur;
    cv::filter2D(bgr8, blur, CV_32F, psf, cv::Point(-1, -1), 0, cv::BORDER_WRAP);
    cv::Mat out8;
    blur.convertTo(out8, CV_8U);
    return out8;
}

// ---- frequency-domain helpers --------------------------------------------
// Pad the image/PSF to the optimal DFT size and transform to the frequency domain.
static void toDFT(const cv::Mat& f32, cv::Mat& fp, cv::Mat& Freq,
                  int& H, int& W) {
    H = cv::getOptimalDFTSize(f32.rows);
    W = cv::getOptimalDFTSize(f32.cols);
    fp = cv::Mat::zeros(H, W, CV_32F);
    f32.copyTo(fp(cv::Rect(0, 0, f32.cols, f32.rows)));
    cv::dft(fp, Freq, cv::DFT_COMPLEX_OUTPUT);
}

// Wiener deconvolution (frequency domain): O = conj(H)/( |H|^2 + K ) * G. K is the noise regularization.
// Input and output are both single-channel 32F [0,1].
static cv::Mat wienerDeconv(const cv::Mat& g32, const cv::Mat& psf, double K) {
    int H, W;
    cv::Mat gp, G, hp, Hs;
    toDFT(g32, gp, G, H, W);
    { cv::Mat hp32 = cv::Mat::zeros(H, W, CV_32F);
      psf.copyTo(hp32(cv::Rect(0, 0, psf.cols, psf.rows)));
      toDFT(hp32, hp, Hs, H, W); }

    cv::Mat Gc[2], Hc[2];
    cv::split(G, Gc); cv::split(Hs, Hc);
    // |H|^2 + K
    cv::Mat denom = Hc[0].mul(Hc[0]) + Hc[1].mul(Hc[1]) + cv::Scalar(K);
    // conj(H)*G
    cv::Mat re = (Hc[0].mul(Gc[0]) + Hc[1].mul(Gc[1])) / denom;
    cv::Mat im = (Hc[0].mul(Gc[1]) - Hc[1].mul(Gc[0])) / denom;
    cv::Mat C[2] = {re, im}, Cm;
    cv::merge(C, 2, Cm);
    cv::Mat outBig, out;
    cv::idft(Cm, outBig, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
    outBig(cv::Rect(0, 0, g32.cols, g32.rows)).copyTo(out);
    return out;
}

// Richardson-Lucy iterative deconvolution.
static cv::Mat richardsonLucy(const cv::Mat& g32, const cv::Mat& psf,
                              int iters = 25) {
    cv::Mat Hf;
    cv::flip(psf, Hf, -1);
    cv::Mat u = g32.clone();
    for (int i = 0; i < iters; ++i) {
        cv::Mat conv, ratio, corr;
        cv::filter2D(u, conv, CV_32F, psf, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);
        cv::divide(g32, conv + 1e-6, ratio);
        cv::filter2D(ratio, corr, CV_32F, Hf, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);
        u = u.mul(corr);
        double mx; cv::minMaxLoc(u, nullptr, &mx);
        if (mx > 1) u = u / mx;
        double mn; cv::minMaxLoc(u, &mn, nullptr);
        if (mn < 0) u = u - mn; // positivity constraint
    }
    return u;
}

// Unsharp Masking sharpening (non-blind baseline).
static cv::Mat unsharpBg(const cv::Mat gray8, double amt = 1.2, int ksize = 7) {
    cv::Mat g96, blur;
    gray8.convertTo(g96, CV_32F);
    cv::GaussianBlur(g96, blur, cv::Size(ksize, ksize), 2.0);
    cv::Mat sharp = g96 + amt * (g96 - blur);
    cv::Mat out8;
    cv::normalize(sharp, out8, 0, 255, cv::NORM_MINMAX, CV_8U);
    return out8;
}

// Convert per-channel deblurred results back to 8UC3.
static cv::Mat toColorBGR(const std::vector<cv::Mat>& chan) {
    std::vector<cv::Mat> r;
    for (auto& c : chan) {
        cv::Mat n; cv::normalize(c, n, 0, 255, cv::NORM_MINMAX, CV_8U);
        r.push_back(n);
    }
    cv::Mat out; cv::merge(r, out); return out;
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    if (argc > 1) inPath = argv[1];
    std::string mode = (argc > 2) ? argv[2] : "motion";
    double p1 = (argc > 3) ? std::atof(argv[3]) : (mode == "motion" ? 9 : 7.0);

    cv::Mat src = cv::imread(inPath, cv::IMREAD_COLOR);
    if (src.empty()) { src = cv::Mat(480, 640, CV_8UC3); cv::randu(src, 0, 256); }
    if (std::max(src.rows, src.cols) > 640) {
        double s = 640.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }
    cv::Mat gray, g32;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    gray.convertTo(g32, CV_32F, 1.0 / 255.0);

    cv::Mat psf = (mode == "motion") ? motionPSF((int)p1, 30.0)
                                     : defocusPSF((std::max)(3, (int)std::lround(2 * p1 + 1)), p1);
    cv::Mat blurred = blurObserve(src, psf);          // 8UC3 blurred observation
    cv::Mat bGray, bg32;
    cv::cvtColor(blurred, bGray, cv::COLOR_BGR2GRAY);
    bGray.convertTo(bg32, CV_32F, 1.0 / 255.0);

    ensureDir("../out/algorithms");
    struct R { std::string tag; cv::Mat img8; double psnr, ssim; };
    std::vector<R> res;
    // Convert a single-channel float restored result to 8U gray (for metrics/display).
    auto gray8 = [](const cv::Mat& f) { cv::Mat n; cv::normalize(f, n, 0, 255, cv::NORM_MINMAX, CV_8U); return n; };
    // Clean grayscale reference (continuous, no degradation).
    cv::Mat refGray = gray;

    // 1) Inverse filter (can fully restore without noise in theory, but amplifies boundary/numerical errors)
    {
        cv::Mat inv = gray8(wienerDeconv(bg32, psf, 1e-8));
        res.push_back({"Inverse(bare)", toColorBGR({inv, inv, inv}), psnr(refGray, inv), ssim(refGray, inv)});
    }
    // 2) Wiener: different noise regularizations
    for (double K : {0.001, 0.01, 0.1}) {
        cv::Mat w = gray8(wienerDeconv(bg32, psf, K));
        res.push_back({"Wiener(K=" + std::to_string(K) + ")", toColorBGR({w, w, w}),
                       psnr(refGray, w), ssim(refGray, w)});
    }
    // 3) Richardson-Lucy
    {
        cv::Mat rl = gray8(richardsonLucy(bg32, psf, 30));
        res.push_back({"RL(iter=30)", toColorBGR({rl, rl, rl}), psnr(refGray, rl), ssim(refGray, rl)});
    }
    // 4) sharpening baseline
    {
        cv::Mat us = unsharpBg(bGray);
        res.push_back({"Unsharp(1.2)", toColorBGR({us, us, us}), psnr(refGray, us), ssim(refGray, us)});
    }

    std::printf("deblur mode=%s  PSF=%dx%d  (reference=clean image)\n", mode.c_str(), psf.rows, psf.cols);
    std::printf("%-22s %12s %8s\n", "method", "PSNR vs clean", "SSIM");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(src); labels.push_back("clean");
    panels.push_back(blurred); labels.push_back("blurred(input)");
    for (auto& r : res) {
        std::printf("%-22s %12.2f %8.4f\n", r.tag.c_str(), r.psnr, r.ssim);
        panels.push_back(r.img8); labels.push_back(r.tag);
    }
    cv::Mat canvas = gridWithLabels(panels, labels, 3, 30);
    std::string out = "../out/algorithms/deblur_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[deblur] wrote " << out << " (cols=" << canvas.cols << " rows=" << canvas.rows << ")\n";
    return 0;
}
