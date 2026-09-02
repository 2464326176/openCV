// algorithms/deblur/main.cpp
// 图像去模糊 (反卷积复原): 合成运动/散焦模糊 + 多种复原算法对比.
//
// 覆盖算法:
//   逆滤波      Inverse filter        (仅理论演示, 噪声下严重放大)
//   Wiener      Wiener 滤波           (频域, 带噪声正则)
//   RL          Richardson-Lucy       (迭代贝叶斯, 保真度好)
//   锐化基线    Unsharp Masking       (非盲锐化, 简单基线)
//
// 流程: 用已知 PSF (运动线性核/高斯散焦核) 对干净图卷积合成模糊,
//       再用各算法复原, 以 PSNR/SSIM 对干净参考做全参考评估.
// 输出: out/algorithms/deblur_compare.png + 指标表.
// 用法: deblur.exe [input_img] [mode=motion|defocus] [blur_len_or_sigma]
#include "../common/algo_utils.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// ---- PSF 构造 ------------------------------------------------------------
// 运动模糊 PSF: 沿 angle 方向 length 像素的线段, L1 归一化.
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

// 散焦模糊 PSF: 高斯近似 (径向对称), L1 归一化.
static cv::Mat defocusPSF(int ksize, double sigma) {
    cv::Mat g = cv::getGaussianKernel(ksize, sigma, CV_32F);
    cv::Mat p = g * g.t();
    cv::normalize(p, p, 1.0, 0.0, cv::NORM_L1);
    return p;
}

// 用 PSF 做循环卷积产生模糊观测 (BORDER_WRAP≈toroidal, 与频域反卷积模型一致).
static cv::Mat blurObserve(const cv::Mat& bgr8, const cv::Mat& psf) {
    cv::Mat blur;
    cv::filter2D(bgr8, blur, CV_32F, psf, cv::Point(-1, -1), 0, cv::BORDER_WRAP);
    cv::Mat out8;
    blur.convertTo(out8, CV_8U);
    return out8;
}

// ---- 频域辅助 -------------------------------------------------------------
// 把图像/PSF padding 到 DFT 最优尺寸并转到频域.
static void toDFT(const cv::Mat& f32, cv::Mat& fp, cv::Mat& Freq,
                  int& H, int& W) {
    H = cv::getOptimalDFTSize(f32.rows);
    W = cv::getOptimalDFTSize(f32.cols);
    fp = cv::Mat::zeros(H, W, CV_32F);
    f32.copyTo(fp(cv::Rect(0, 0, f32.cols, f32.rows)));
    cv::dft(fp, Freq, cv::DFT_COMPLEX_OUTPUT);
}

// Wiener 反卷积 (频域): O = conj(H)/( |H|^2 + K ) * G. K 为噪声正则.
// 输入输出均为单通道 32F [0,1].
static cv::Mat wienerDeconv(const cv::Mat& g32, const cv::Mat& psf, double K) {
    int H, W;
    cv::Mat gp, G, hp, Hs;
    toDFT(g32, gp, G, H, W);
    { cv::Mat hp32 = cv::Mat::zeros(H, W, CV_32F);
      psf.copyTo(hp32(cv::Rect(0, 0, psf.cols, psf.rows)));
      toDFT(hp32, hp, Hs, H, W); }

    cv::Mat Gc[2], Hc[2];
    cv::split(G, Gc); cv::split(Hs, Hc);
    // H^2 + K
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

// Richardson-Lucy 迭代反卷积.
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
        if (mn < 0) u = u - mn; // 保正约束
    }
    return u;
}

// Unsharp Masking 锐化 (非盲基线).
static cv::Mat unsharpBg(const cv::Mat gray8, double amt = 1.2, int ksize = 7) {
    cv::Mat g96, blur;
    gray8.convertTo(g96, CV_32F);
    cv::GaussianBlur(g96, blur, cv::Size(ksize, ksize), 2.0);
    cv::Mat sharp = g96 + amt * (g96 - blur);
    cv::Mat out8;
    cv::normalize(sharp, out8, 0, 255, cv::NORM_MINMAX, CV_8U);
    return out8;
}

// 把各通道去除模糊结果转回 8UC3.
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
    cv::Mat blurred = blurObserve(src, psf);          // 8UC3 模糊观测
    cv::Mat bGray, bg32;
    cv::cvtColor(blurred, bGray, cv::COLOR_BGR2GRAY);
    bGray.convertTo(bg32, CV_32F, 1.0 / 255.0);

    ensureDir("../out/algorithms");
    struct R { std::string tag; cv::Mat img8; double psnr, ssim; };
    std::vector<R> res;
    // 由单通道浮点还原结果得到 8U 灰度(用于指标/展示).
    auto gray8 = [](const cv::Mat& f) { cv::Mat n; cv::normalize(f, n, 0, 255, cv::NORM_MINMAX, CV_8U); return n; };
    // 参考用干净灰度图 (连续、无退化).
    cv::Mat refGray = gray;

    // 1) 逆滤波 (理论上无噪声可完全复原, 但会放大边界/数值误差)
    {
        cv::Mat inv = gray8(wienerDeconv(bg32, psf, 1e-8));
        res.push_back({"Inverse(bare)", toColorBGR({inv, inv, inv}), psnr(refGray, inv), ssim(refGray, inv)});
    }
    // 2) Wiener: 不同噪声正则
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
    // 4) 锐化基线
    {
        cv::Mat us = unsharpBg(bGray);
        res.push_back({"Unsharp(1.2)", toColorBGR({us, us, us}), psnr(refGray, us), ssim(refGray, us)});
    }

    std::printf("deblur mode=%s  PSF=%dx%d  (参考=干净图)\n", mode.c_str(), psf.rows, psf.cols);
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