// algorithms/sharpen/sharpen.cpp
// Image sharpening algorithm comparison demo (ISP Sharpen stage).
//
// Capabilities:
//   1. Simulate a "soft capture" input: mild Gaussian blur + slight noise,
//      the sharp original serves as ground truth (GT).
//   2. Algorithms (5 families, 9 variants):
//        - Laplacian enhancement (additive) k={0.5, 1.0}
//        - USM unsharp mask sigma={1,2} x amount={0.8, 1.5}
//        - Guided-filter edge-aware USM (r=8, eps=0.01) amount={1.0, 2.0}
//        - Halo-suppressed USM: gradient-masked USM to avoid sharpening flat noise
//        - High-boost bilateral (bilateral as base + residual boost)
//   3. Metrics: PSNR/SSIM vs GT (over-sharpening penalized), Tenengrad sharpness,
//      plus a USM radius/strength 2D parameter sweep table.
//   4. Output mosaic: GT -> degraded -> each algorithm result -> out/algorithms/sharpen_compare.png
//
// Usage: sharpen.exe [input_img] [blur_sigma=1.2] [noise_sigma=3]
#include "../common/algo_utils.hpp"
#include "../common/single_denoise.hpp"   // denoiseGuided (guided filter impl)

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace algo;

// ---------------------------------------------------------------------------
// Tenengrad sharpness: mean of squared Sobel gradient magnitude over the image,
// larger = sharper (classic focus / sharpness metric). Gray 8U input.
// ---------------------------------------------------------------------------
static double tenengrad(const cv::Mat& bgr) {
    cv::Mat g;
    if (bgr.channels() == 3) cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
    else g = bgr;
    cv::Mat gx, gy, mag2;
    cv::Sobel(g, gx, CV_32F, 1, 0, 3);
    cv::Sobel(g, gy, CV_32F, 0, 1, 3);
    cv::multiply(gx, gx, mag2);  // reuse buffer per-axis to save memory
    cv::Mat gy2; cv::multiply(gy, gy, gy2);
    mag2 += gy2;
    return cv::mean(mag2)[0];
}

struct ResultRow {
    std::string tag;
    cv::Mat img;
    double psnr = 0, ssim = 0, ten = 0;
};

static void scoreRow(ResultRow& r, const cv::Mat& gt) {
    r.psnr = psnr(gt, r.img);
    r.ssim = ssim(gt, r.img);
    r.ten = tenengrad(r.img);
}

// ---------------------------------------------------------------------------
// Algorithms
// ---------------------------------------------------------------------------

// Additive Laplacian enhancement: out = src + k * lap(src)
static cv::Mat sharpenLaplacian(const cv::Mat& src, double k) {
    cv::Mat lap;
    cv::Laplacian(src, lap, CV_16S, 3, 1, 0, cv::BORDER_DEFAULT);
    cv::Mat lapF, srcF;
    src.convertTo(srcF, CV_32F);
    lap.convertTo(lapF, CV_32F);
    cv::Mat out = srcF + k * lapF;
    return to8U(out, 1.0);
}

// Classic USM: out = src + amount * (src - gauss(src, sigma)).
// Float math: 8U subtraction would saturate and lose the dark side of edges.
static cv::Mat sharpenUSM(const cv::Mat& src, double sigma, double amount) {
    cv::Mat srcF;
    src.convertTo(srcF, CV_32F);
    cv::Mat blur;
    cv::GaussianBlur(srcF, blur, cv::Size(), sigma, sigma, cv::BORDER_REPLICATE);
    return to8U(srcF + amount * (srcF - blur), 1.0);
}

// Edge-aware USM with guided filter base (computed in [0,1]): keeps texture,
// fewer halos than plain Gaussian USM at the same amount.
// NOTE: denoiseGuided expects 8U input, so feed the 8U channels directly.
static cv::Mat sharpenGuided(const cv::Mat& src, int r, double eps, double amount) {
    cv::Mat srcF;
    src.convertTo(srcF, CV_32F, 1.0 / 255.0);
    std::vector<cv::Mat> ch8;
    cv::split(src, ch8);
    std::vector<cv::Mat> base(ch8.size());
    for (size_t c = 0; c < ch8.size(); ++c) {
        cv::Mat b8 = denoiseGuided(ch8[c], ch8[c], r, eps);   // 8U [0,255]
        b8.convertTo(base[c], CV_32F, 1.0 / 255.0);           // -> [0,1]
    }
    cv::Mat baseF;
    cv::merge(base, baseF);
    return to8U(srcF + amount * (srcF - baseF));       // [0,1] -> default scale 255
}

// Halo-suppressed USM: scale the high-frequency residual by a local-gradient
// mask ([0,1]) so flat/noisy areas are not amplified (less noise, fewer halos).
static cv::Mat sharpenMaskedUSM(const cv::Mat& src, double sigma, double amount) {
    cv::Mat srcF;
    src.convertTo(srcF, CV_32F);
    cv::Mat blur;
    cv::GaussianBlur(srcF, blur, cv::Size(), sigma, sigma, cv::BORDER_REPLICATE);
    cv::Mat hf = srcF - blur;
    cv::Mat g;
    if (src.channels() == 3) cv::cvtColor(src, g, cv::COLOR_BGR2GRAY);
    else g = src;
    cv::Mat grad;
    cv::Laplacian(g, grad, CV_32F, 3);
    cv::Mat mask = cv::abs(grad);                       // local activity
    cv::threshold(mask, mask, 24.0, 24.0, cv::THRESH_TRUNC);
    mask.convertTo(mask, CV_32F, 1.0 / 24.0);           // clamp to [0,1]
    cv::Mat mask3;
    if (src.channels() == 3) {
        std::vector<cv::Mat> mm(3, mask);
        cv::merge(mm, mask3);
    } else mask3 = mask;
    return to8U(srcF + amount * hf.mul(mask3), 1.0);
}

// High-boost bilateral: bilateral base (edge-preserving, 8U only) + boosted
// residual computed in float.
static cv::Mat sharpenHighBoostBilateral(const cv::Mat& src, double amount) {
    cv::Mat base;
    cv::bilateralFilter(src, base, 9, 60, 8);
    cv::Mat srcF, baseF;
    src.convertTo(srcF, CV_32F);
    base.convertTo(baseF, CV_32F);
    return to8U(srcF + amount * (srcF - baseF), 1.0);
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    double blurSigma = 1.2, noiseSigma = 3.0;
    if (argc > 1) inPath = argv[1];
    if (argc > 2) blurSigma = std::atof(argv[2]);
    if (argc > 3) noiseSigma = std::atof(argv[3]);

    cv::Mat gt = cv::imread(inPath, cv::IMREAD_COLOR);
    cv::Mat degraded;
    if (gt.empty()) {
        log("sharpen", "input missing, synthesizing a test pattern as GT");
        gt = cv::Mat(720, 1080, CV_8UC3);
        cv::randu(gt, cv::Scalar::all(30), cv::Scalar::all(220));
        cv::GaussianBlur(gt, gt, cv::Size(1, 1), 0);  // keep deterministic layout
        cv::rectangle(gt, {100, 100, 300, 300}, {255, 255, 255}, cv::FILLED);
        cv::circle(gt, {700, 400}, 180, {60, 60, 200}, cv::FILLED);
        for (int i = 40; i < 700; i += 12)           // fine stripes: sharpening test
            cv::line(gt, {i, 550}, {i, 690}, {0, 255, 0}, 2);
    }
    // Downscale very large inputs for a fast demo run.
    if (std::max(gt.rows, gt.cols) > 1600) {
        double s = 1600.0 / std::max(gt.rows, gt.cols);
        cv::resize(gt, gt, cv::Size(), s, s, cv::INTER_AREA);
    }

    // ---- Simulate soft capture: blur + slight noise ----
    cv::GaussianBlur(gt, degraded, cv::Size(), blurSigma, blurSigma, cv::BORDER_REPLICATE);
    if (noiseSigma > 0) {
        cv::Mat nz(degraded.size(), CV_16SC3);
        cv::randn(nz, cv::Scalar::all(0), cv::Scalar::all(noiseSigma));
        cv::Mat d16; degraded.convertTo(d16, CV_16SC3);
        d16 += nz;
        d16.convertTo(degraded, CV_8UC3);
    }
    log("sharpen", "GT " + std::to_string(gt.cols) + "x" + std::to_string(gt.rows) +
        ", degraded: gauss sigma=" + std::to_string(blurSigma) +
        ", noise sigma=" + std::to_string(noiseSigma));

    // ---- Methods ----
    std::vector<ResultRow> rows;
    rows.push_back({"GT(sharp)", gt, 1000, 1.0, 0});
    rows.push_back({"degraded(input)", degraded, 0, 0, 0});
    rows.push_back({"Laplacian k=0.5", sharpenLaplacian(degraded, 0.5), 0, 0, 0});
    rows.push_back({"Laplacian k=1.0", sharpenLaplacian(degraded, 1.0), 0, 0, 0});
    rows.push_back({"USM s=1.0 a=0.8", sharpenUSM(degraded, 1.0, 0.8), 0, 0, 0});
    rows.push_back({"USM s=1.0 a=1.5", sharpenUSM(degraded, 1.0, 1.5), 0, 0, 0});
    rows.push_back({"USM s=2.0 a=0.8", sharpenUSM(degraded, 2.0, 0.8), 0, 0, 0});
    rows.push_back({"USM s=2.0 a=1.5", sharpenUSM(degraded, 2.0, 1.5), 0, 0, 0});
    rows.push_back({"Guided r=8 eps=0.01 a=1.0", sharpenGuided(degraded, 8, 0.01, 1.0), 0, 0, 0});
    rows.push_back({"Guided r=8 eps=0.01 a=2.0", sharpenGuided(degraded, 8, 0.01, 2.0), 0, 0, 0});
    rows.push_back({"MaskedUSM s=1.0 a=2.0", sharpenMaskedUSM(degraded, 1.0, 2.0), 0, 0, 0});
    rows.push_back({"HighBoostBilateral a=1.5", sharpenHighBoostBilateral(degraded, 1.5), 0, 0, 0});
    for (size_t i = 1; i < rows.size(); ++i) scoreRow(rows[i], gt); // include degraded baseline
    rows[0].ten = tenengrad(gt);

    // ---- Print metric table ----
    std::printf("\n%-30s %8s %8s %10s\n", "method", "PSNR", "SSIM", "Tenengrad");
    for (auto& r : rows) {
        if (r.tag == "GT(sharp)")
            std::printf("%-30s %8s %8s %10.1f\n", r.tag.c_str(), "-", "-", r.ten);
        else
            std::printf("%-30s %8.2f %8.4f %10.1f\n",
                        r.tag.c_str(), r.psnr, r.ssim, r.ten);
    }
    std::printf("\n> PSNR/SSIM vs sharp GT: over-sharpening (halos) lowers both. "
                "Tenengrad: GT is the natural upper reference.\n");

    // ---- USM parameter sweep (sigma x amount): PSNR grid ----
    std::printf("\nUSM sweep (PSNR vs GT):\n%-8s", "sigma\\a");
    std::vector<double> amounts = {0.5, 0.8, 1.0, 1.5, 2.0, 3.0};
    for (double a : amounts) std::printf(" %7.1f", a);
    std::printf("\n");
    for (double s : {0.5, 1.0, 1.5, 2.0, 3.0}) {
        std::printf("%-8.1f", s);
        for (double a : amounts) {
            cv::Mat out = sharpenUSM(degraded, s, a);
            std::printf(" %7.2f", psnr(gt, out));
        }
        std::printf("\n");
    }

    // ---- Mosaic: GT / degraded / best subset ----
    ensureDir("../out/algorithms");
    std::vector<size_t> show = {0, 1, 6, 8, 9, 10, 11, rows.size() - 1};
    std::vector<cv::Mat> panels;
    std::vector<std::string> labels;
    for (size_t i : show) {
        panels.push_back(rows[i].img);
        char buf[96];
        std::snprintf(buf, sizeof(buf), "%s [%.1fdB]", rows[i].tag.c_str(), rows[i].psnr);
        labels.push_back(rows[i].tag == "GT(sharp)" ? rows[i].tag : buf);
    }
    cv::Mat board = gridWithLabels(panels, labels, 4, 30);
    const std::string outPng = "../out/algorithms/sharpen_compare.png";
    cv::imwrite(outPng, board);
    log("sharpen", "saved " + outPng);

    imshowFit("sharpen", board);
    return 0;
}
