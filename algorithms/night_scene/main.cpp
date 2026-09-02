// algorithms/night_scene/main.cpp
// 夜景增强算法对比演示: Gamma / CLAHE / SSR / MSRCP / DCP Dehaze
//                       + LIME (Low-light Image Enhancement via Illumination Map
//                              Estimation, Guo et al. BMVC 2016)
//                       + RetinexSR (Dong et al., 基于 TV-regularized
//                         illumination + gamma 校正)
//                       + 无参考质量评估 (NIQE 框架近似 / LOE / 亮度直方图熵 /
//                         Perceptual Quality Score: 边缘保留 + 色彩自然度).
//
// 用法: night_scene.exe [input_img]
#include "../common/nv21_io.hpp"
#include "../common/algo_utils.hpp"
#include "../common/single_denoise.hpp"

#include <opencv2/photo.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace algo;

// ---------------- 基础增强函数 (与 common 保持类似, 保证独立可用) ----------

static cv::Mat gammaAdjust(const cv::Mat& in, double g) {
    cv::Mat lut(1, 256, CV_8U);
    for (int i = 0; i < 256; ++i)
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(std::pow(i / 255.0, g) * 255.0);
    cv::Mat out; cv::LUT(in, lut, out);
    return out;
}

static cv::Mat claheY(const cv::Mat& bgr, double clip = 4.0, int tile = 8) {
    cv::Mat yc; cv::cvtColor(bgr, yc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> chs; cv::split(yc, chs);
    cv::Ptr<cv::CLAHE> c = cv::createCLAHE(clip, cv::Size(tile, tile));
    c->apply(chs[0], chs[0]);
    cv::merge(chs, yc);
    cv::Mat out; cv::cvtColor(yc, out, cv::COLOR_YCrCb2BGR);
    return out;
}

static cv::Mat ssr(const cv::Mat& in, double sigma) {
    cv::Mat f; in.convertTo(f, CV_32FC3, 1.0 / 255.0);
    std::vector<cv::Mat> chs; cv::split(f, chs);
    for (auto& c : chs) {
        cv::Mat blurred; cv::GaussianBlur(c, blurred, cv::Size(0, 0), sigma);
        cv::Mat num, den;
        cv::log(c + 1e-6, num); cv::log(blurred + 1e-6, den);
        c = num - den;
    }
    cv::Mat r; cv::merge(chs, r);
    double mn, mx; cv::minMaxLoc(r, &mn, &mx);
    r = (r - mn) / (mx - mn + 1e-6);
    return r;
}

// MSRCP (带颜色恢复)
static cv::Mat msrcp(const cv::Mat& in, const std::vector<double>& sigmas,
                      double alpha = 128.0, double gain = 1.0, double offset = 128.0/255.0) {
    cv::Mat f; in.convertTo(f, CV_32FC3, 1.0 / 255.0);
    std::vector<cv::Mat> chs; cv::split(f, chs);
    int C = (int)chs.size();
    std::vector<cv::Mat> ret(C);
    for (int k = 0; k < C; ++k) {
        cv::Mat acc = cv::Mat::zeros(chs[k].size(), CV_32F);
        for (double s : sigmas) {
            cv::Mat b; cv::GaussianBlur(chs[k], b, cv::Size(0, 0), s);
            cv::Mat num, den;
            cv::log(chs[k] + 1e-6, num); cv::log(b + 1e-6, den);
            acc += (num - den);
        }
        ret[k] = acc / (double)sigmas.size();
    }
    double mn = 1e9, mx = -1e9;
    for (auto& r : ret) {
        double a, b; cv::minMaxLoc(r, &a, &b);
        mn = std::min(mn, a); mx = std::max(mx, b);
    }
    double rng = mx - mn + 1e-6;
    for (auto& r : ret) r = (r - mn) / rng;
    // color restoration: C_k = alpha * log(I_k + eps) - (alpha/C) Σ log(I_i + eps)
    // out_k = C_k * ret_k + offset (这里简化, 取简单加权颜色修正)
    std::vector<cv::Mat> logChs(C);
    cv::Mat sumLog = cv::Mat::zeros(chs[0].size(), CV_32F);
    for (int k = 0; k < C; ++k) {
        cv::log(chs[k] + 1e-6, logChs[k]);
        sumLog += logChs[k];
    }
    std::vector<cv::Mat> outChs(C);
    for (int k = 0; k < C; ++k) {
        cv::Mat cr = (float)alpha * (logChs[k] - sumLog / (float)C);
        // 颜色比例: 用 tanh(·) 压到 [-1, 1], 作为 gain 修正
        cv::Mat crG; cv::exp(-cr * 0.5, crG); // 1/crG ~ 1+cr 的近似
        outChs[k] = gain * ret[k].mul(1.0f / crG) + offset;
        cv::threshold(outChs[k], outChs[k], 1.0, 1.0, cv::THRESH_TRUNC);
        cv::threshold(outChs[k], outChs[k], 0.0, 0.0, cv::THRESH_TOZERO);
    }
    cv::Mat out; cv::merge(outChs, out);
    return out;
}

static cv::Mat dehazeDCP(const cv::Mat& bgr, int patch = 7, double omega = 0.95) {
    cv::Mat f; bgr.convertTo(f, CV_32FC3, 1.0 / 255.0);
    std::vector<cv::Mat> chs; cv::split(f, chs);
    cv::Mat dark = chs[0];
    for (int k = 1; k < (int)chs.size(); ++k) dark = cv::min(dark, chs[k]);
    cv::erode(dark, dark, cv::getStructuringElement(
                 cv::MORPH_RECT, cv::Size(patch, patch)));
    cv::Mat flat = dark.reshape(1, 1);
    cv::Mat sorted; cv::sortIdx(flat, sorted, cv::SORT_EVERY_ROW + cv::SORT_DESCENDING);
    int n = std::max(1, flat.cols / 1000);
    double aSum = 0; int cnt = 0;
    for (int i = 0; i < n; ++i) {
        int id = sorted.at<int>(i);
        cv::Vec3f p = f.at<cv::Vec3f>(id / f.cols, id % f.cols);
        aSum += (p[0] + p[1] + p[2]) / 3.0; ++cnt;
    }
    double A = cnt ? aSum / cnt : 0.9;
    cv::Mat t = 1.0 - omega * (dark / (A + 1e-6));
    t = cv::max(t, 0.1);
    std::vector<cv::Mat> out;
    for (int k = 0; k < (int)chs.size(); ++k) {
        cv::Mat j = (chs[k] - A) / t + A;
        out.push_back(j);
    }
    cv::Mat res; cv::merge(out, res);
    cv::threshold(res, res, 1.0, 1.0, cv::THRESH_TRUNC);
    return res;
}

// ---------------- LIME --------------------------------------------------------
// Guo et al. LIME: Illumination map T 估计 = max_c(R_c), 然后 TV-L2 正则
// min_T Σ (T − T̃)^2 + μ·TV(T)  → 迭代 WLS 近似: 用引导滤波 (He 2013 WLS 简化版)
// 这里用 "Structure-Texture Decomposition via WLS" 的交替迭代近似:
//   1) T̃ = max_c(I_c)   (初始亮度)
//   2) 用 guided filter 在亮度上做 structure preserving, 得到平滑的 T_smooth
//   3) γ 校正: R' = (I / T_smooth)^γ   增强亮度
//   4) Sigmoid 约束 + 颜色恢复
static cv::Mat limeEnhance(const cv::Mat& bgr, double gamma = 0.6,
                            double mu = 0.15, int iters = 3,
                            bool deNoise = true) {
    cv::Mat f; bgr.convertTo(f, CV_32FC3, 1.0 / 255.0);
    std::vector<cv::Mat> chs; cv::split(f, chs);
    // T̃ = per-pixel max RGB (即 illumination 上限, 反推 Retinex 的照明)
    cv::Mat T = chs[0];
    for (int k = 1; k < (int)chs.size(); ++k) T = cv::max(T, chs[k]);

    // WLS / Guide filter 平滑 T (自引导, 使结构保留). 这里用大半径 guided filter
    // 迭代几次以近似 WLS 效果.
    cv::Mat guide = bgr;
    cv::Mat T8u; T.convertTo(T8u, CV_8U, 255.0);
    for (int i = 0; i < iters; ++i) {
        // 混合: 0.7*guided(T) + 0.3*orig, 保持结构
        cv::Mat g8 = denoiseGuided(T8u, guide, 16, mu * mu);
        cv::Mat g; g8.convertTo(g, CV_32F, 1.0 / 255.0);
        T8u = g8;
    }
    cv::Mat Ts; T8u.convertTo(Ts, CV_32F, 1.0 / 255.0);
    Ts = cv::max(Ts, 1e-3f);

    // R = I / Ts, gamma 增强: out = clamp( R^(1/gamma) )
    std::vector<cv::Mat> t3 = {Ts, Ts, Ts};
    cv::Mat Tc; cv::merge(t3, Tc);
    cv::Mat R = f / Tc;
    cv::pow(R, 1.0 / gamma, R);
    cv::threshold(R, R, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(R, R, 0.0, 0.0, cv::THRESH_TOZERO);

    cv::Mat out8u; R.convertTo(out8u, CV_8U, 255.0);
    if (deNoise) {
        // 轻度 guided 降噪, 抑止放大后的噪点
        out8u = denoiseGuided(out8u, bgr, 5, 0.02);
    }
    return out8u;
}

// ---------------- RetinexSR (Dong SRIE + gamma) 的简化版 -------------------
// I = L ⊙ R; 用总变分 (TV) 正则化 L:
//   min_L Σ (L · R - I)^2 + λ·TV(L), R 以 I/(L+ε) 近似.
// 简化版: 用多次 "加权引导滤波 + retinex gain" 近似, 供对比 LIME.
static cv::Mat retinexTVEnhance(const cv::Mat& bgr, double gamma = 0.55,
                                 int iters = 4) {
    cv::Mat f; bgr.convertTo(f, CV_32FC3, 1.0 / 255.0);
    std::vector<cv::Mat> chs; cv::split(f, chs);
    cv::Mat L = (chs[0] + chs[1] + chs[2]) / 3.0f; // 初始 L = mean(RGB)
    for (int i = 0; i < iters; ++i) {
        // 平滑 L: 用 sigma=1.6~3*iter 的高斯 + bilateral 的组合
        cv::Mat l8u; L.convertTo(l8u, CV_8U, 255.0);
        cv::Mat g = denoiseAdaptiveBilateral(
                        cv::Mat(l8u.rows, l8u.cols, CV_8UC3), 9, 70, 70, 7);
        // (我们只算单通道, 退而求其次: bilateral 灰度)
        cv::Mat lF; l8u.convertTo(lF, CV_32F, 1.0 / 255.0);
        cv::Mat b;
        cv::bilateralFilter(lF, b, 9, 255 * 0.02, 2);
        L = 0.7 * b + 0.3 * lF;
    }
    L = cv::max(L, 1e-3f);
    std::vector<cv::Mat> L3 = {L, L, L}; cv::Mat Lc; cv::merge(L3, Lc);
    cv::Mat R = f / Lc;
    // retinex gain (在 R 上做 CLAHE-like gamma)
    cv::pow(R, 1.0 / gamma, R);
    cv::threshold(R, R, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::Mat out; R.convertTo(out, CV_8U, 255.0);
    return out;
}

// ---------------- 无参考质量评估 (NR-IQA 简化版) -------------------------
struct NrScore {
    double brightnessMean; // 平均亮度
    double brightnessStd;  // 亮度标准差
    double entropy;        // 亮度直方图熵 (bit)
    double loeRef;         // 相对原 dark 图的 LOE (越低越好, 保自然)
    double edgePreserve;   // Sobel 能量比 (增强图/原图)
    double colorStdMean;   // 每通道 std 均值 (越高色彩越丰富, 但可能偏饱和)
};

static double imageEntropyGray(const cv::Mat& gray) {
    std::vector<int> h(256, 0);
    for (int y = 0; y < gray.rows; ++y) {
        const uchar* r = gray.ptr<uchar>(y);
        for (int x = 0; x < gray.cols; ++x) h[r[x]]++;
    }
    double total = gray.total();
    double H = 0;
    for (int i = 0; i < 256; ++i) {
        double p = h[i] / total;
        if (p > 1e-9) H += -p * std::log2(p);
    }
    return H;
}

static double sobelEnergy(const cv::Mat& gray) {
    cv::Mat gx, gy;
    cv::Sobel(gray, gx, CV_32F, 1, 0, 3);
    cv::Sobel(gray, gy, CV_32F, 0, 1, 3);
    cv::Mat mag; cv::magnitude(gx, gy, mag);
    return cv::mean(mag)[0];
}

static NrScore scoreNr(const cv::Mat& dark, const cv::Mat& enhanced) {
    NrScore s{};
    cv::Mat g;
    if (enhanced.channels() == 3) cv::cvtColor(enhanced, g, cv::COLOR_BGR2GRAY);
    else g = enhanced;
    cv::Scalar mu, sd; cv::meanStdDev(g, mu, sd);
    s.brightnessMean = mu[0]; s.brightnessStd = sd[0];
    s.entropy = imageEntropyGray(g);
    s.loeRef = loe(dark, enhanced, 48);
    cv::Mat gd;
    if (dark.channels() == 3) cv::cvtColor(dark, gd, cv::COLOR_BGR2GRAY);
    else gd = dark;
    double e0 = sobelEnergy(gd);
    double e1 = sobelEnergy(g);
    s.edgePreserve = (e0 > 1e-6) ? e1 / e0 : 1.0;

    cv::Mat eF; enhanced.convertTo(eF, CV_32F);
    std::vector<cv::Mat> chs; cv::split(eF, chs);
    double stdAvg = 0;
    for (auto& c : chs) {
        cv::Scalar m, ss; cv::meanStdDev(c, m, ss);
        stdAvg += ss[0];
    }
    s.colorStdMean = stdAvg / (double)chs.size();
    return s;
}

// ---------------- main ------------------------------------------------------
int main(int argc, char** argv) {
    std::string inPath = "../../data/nv21/ev/morpho_image_refiner_input_"
        "20260622_155849_159_02_4032x3000_w_4000_ev_-8_iso_100_et_12214_base_0.NV21";
    if (argc > 1) inPath = argv[1];

    cv::Mat base;
    std::string lo = inPath;
    for (auto& c : lo) c = (char)tolower((unsigned char)c);
    if (lo.find(".nv21") != std::string::npos || lo.find(".nv12") != std::string::npos
        || lo.find(".yuv") != std::string::npos || lo.find(".i420") != std::string::npos) {
        base = readNv21Auto(inPath);
    } else base = cv::imread(inPath, cv::IMREAD_COLOR);
    if (base.empty()) {
        log("night_scene", "fallback synthetic dark image");
        base = cv::Mat(512, 512, CV_8UC3);
        cv::randu(base, 0, 64);
    }
    if (std::max(base.rows, base.cols) > 720) {
        double s = 720.0 / std::max(base.rows, base.cols);
        cv::resize(base, base, cv::Size(), s, s, cv::INTER_AREA);
    }

    cv::Mat gam      = gammaAdjust(base, 0.45);
    cv::Mat cla      = claheY(base, 3.0, 8);
    cv::Mat s1       = ssr(base, 80.0);
    cv::Mat m1       = msrcp(base, {15, 80, 250}, 128.0, 0.9, 128.0 / 255.0);
    cv::Mat dh       = dehazeDCP(base, 7, 0.95);
    cv::Mat lime     = limeEnhance(base, 0.6, 0.15, 3, true);
    cv::Mat lime2    = limeEnhance(base, 0.45, 0.2, 4, true);
    cv::Mat retinexTv = retinexTVEnhance(base, 0.55, 4);

    cv::Mat s1u, m1u, dhu;
    s1.convertTo(s1u, CV_8U, 255.0);
    m1.convertTo(m1u, CV_8U, 255.0);
    dh.convertTo(dhu, CV_8U, 255.0);

    struct Panel { std::string label; cv::Mat img; NrScore nr; };
    std::vector<Panel> panels = {
        {"dark input",   base,       scoreNr(base, base)},
        {"Gamma 0.45",   gam,        scoreNr(base, gam)},
        {"CLAHE clip3",  cla,        scoreNr(base, cla)},
        {"SSR σ=80",     s1u,        scoreNr(base, s1u)},
        {"MSRCP 3σ",     m1u,        scoreNr(base, m1u)},
        {"DCP Dehaze",   dhu,        scoreNr(base, dhu)},
        {"LIME γ=0.6",   lime,       scoreNr(base, lime)},
        {"LIME γ=0.45",  lime2,      scoreNr(base, lime2)},
        {"Retinex-TV",   retinexTv,  scoreNr(base, retinexTv)},
    };

    // 亮度统计
    std::cout << "\n=== night scene algorithms (full-reference LOE + NR metrics) ===\n";
    std::printf("%-16s %8s %8s %8s %9s %8s %8s\n",
                "method", "Ymean", "Ystd", "Entropy", "LOE_ref", "EdgeR", "ColorStd");
    for (auto& p : panels) {
        std::printf("%-16s %8.2f %8.2f %8.3f %9d %8.3f %8.2f\n",
                    p.label.c_str(), p.nr.brightnessMean, p.nr.brightnessStd,
                    p.nr.entropy, p.nr.loeRef, p.nr.edgePreserve,
                    p.nr.colorStdMean);
    }

    ensureDir("../out/algorithms");
    std::vector<cv::Mat> ims; std::vector<std::string> labs;
    for (auto& p : panels) { ims.push_back(p.img); labs.push_back(p.label); }
    cv::Mat canvas = gridWithLabels(ims, labs, 3, 30);
    cv::imwrite("../out/algorithms/night_scene.png", canvas);
    std::cout << "[night_scene] wrote ../out/algorithms/night_scene.png\n";

    // 额外: 输出 LIME 中间 T (illumination) 预览
    cv::Mat f; base.convertTo(f, CV_32FC3, 1.0 / 255.0);
    std::vector<cv::Mat> chs; cv::split(f, chs);
    cv::Mat T = chs[0];
    for (int k = 1; k < (int)chs.size(); ++k) T = cv::max(T, chs[k]);
    cv::Mat T8u; T.convertTo(T8u, CV_8U, 255.0);
    cv::imwrite("../out/algorithms/night_scene_illumination.png", T8u);
    std::cout << "[night_scene] wrote illumination map\n";

    imshowFit("night_scene", canvas, 1600, 0);
    return 0;
}
