// algorithms/frequency_domain/main.cpp
// 频域分析与滤波: 二维 DFT、幅值谱可视化、低通/高通/陷波滤波.
//
// 覆盖:
//   DFT 幅值谱 (中心化 + log 拉伸)
//   高斯低通   (平滑/去噪)
//   高斯高通   (边缘/细节提取)
//   陷波(带阻) (去除周期性正弦噪声)
// 评估: 低通/陷波与参考的 PSNR; 高通输出边缘二值密度.
// 输出: out/algorithms/frequency_domain_compare.png.
#include "../common/algo_utils.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>

using namespace algo;

// 中心化幅值谱 → 8U (log 压缩便于观察).
static cv::Mat magnitudeSpectrum(const cv::Mat& complexI) {
    cv::Mat planes[2];
    cv::split(complexI, planes);
    cv::Mat mag;
    cv::magnitude(planes[0], planes[1], mag);
    mag += 1.0;
    cv::log(mag, mag);
    cv::Mat centered = mag(cv::Rect(0, 0, mag.cols & -2, mag.rows & -2)).clone();
    int cx = centered.cols / 2, cy = centered.rows / 2;
    cv::Mat q0, q1, q2, q3;
    q0 = centered(cv::Rect(0, 0, cx, cy)); q1 = centered(cv::Rect(cx, 0, cx, cy));
    q2 = centered(cv::Rect(0, cy, cx, cy)); q3 = centered(cv::Rect(cx, cy, cx, cy));
    q0.copyTo(cv::Mat(centered, cv::Rect(cx, cy, cx, cy)));
    q3.copyTo(cv::Mat(centered, cv::Rect(0, 0, cx, cy)));
    q1.copyTo(cv::Mat(centered, cv::Rect(0, cy, cx, cy)));
    q2.copyTo(cv::Mat(centered, cv::Rect(cx, 0, cx, cy)));
    cv::Mat mag8;
    cv::normalize(centered, mag8, 0, 255, cv::NORM_MINMAX, CV_8U);
    return mag8;
}

// 象限交换 (中心化 ↔ 角落布局). DFT 未移位时低频在四角, 掩膜/谱展示需换算.
static cv::Mat swapQuadrants(const cv::Mat& src) {
    cv::Mat centered = src(cv::Rect(0, 0, src.cols & -2, src.rows & -2)).clone();
    int cx = centered.cols / 2, cy = centered.rows / 2;
    cv::Mat q0, q1, q2, q3;
    q0 = centered(cv::Rect(0, 0, cx, cy)); q1 = centered(cv::Rect(cx, 0, cx, cy));
    q2 = centered(cv::Rect(0, cy, cx, cy)); q3 = centered(cv::Rect(cx, cy, cx, cy));
    q0.copyTo(cv::Mat(centered, cv::Rect(cx, cy, cx, cy)));
    q3.copyTo(cv::Mat(centered, cv::Rect(0, 0, cx, cy)));
    q1.copyTo(cv::Mat(centered, cv::Rect(0, cy, cx, cy)));
    q2.copyTo(cv::Mat(centered, cv::Rect(cx, 0, cx, cy)));
    return centered;
}

// 构造高斯掩膜 (32F, 尺寸同 dftSize, 中心化布局): D=1 低通保存低频, D=0 高通.
template <bool LOW>
static cv::Mat gaussFilterMask(int rows, int cols, double sigma) {
    cv::Mat H(rows, cols, CV_32F);
    int cx = cols / 2, cy = rows / 2;
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < cols; ++x)
            H.at<float>(y, x) = (float)std::exp(-((x - cx) * (x - cx) + (y - cy) * (y - cy))
                                                  / (2.0 * sigma * sigma));
    if (!LOW) H = 1.0 - H;
    return H;
}

// 频域滤波: src8 灰度/彩色 → 应用掩膜 → 逆 DFT.
static cv::Mat applyFreqFilter(const cv::Mat& img8, const cv::Mat& maskCentered,
                               int H, int W) {
    cv::Mat mask = swapQuadrants(maskCentered); // 转成未移位布局, 与 dft 对齐
    std::vector<cv::Mat> ch;
    cv::split(img8, ch);
    std::vector<cv::Mat> outCh;
    for (auto& c : ch) {
        cv::Mat f; c.convertTo(f, CV_32F);
        cv::Mat padded = cv::Mat::zeros(H, W, CV_32F);
        f.copyTo(padded(cv::Rect(0, 0, f.cols, f.rows)));
        cv::Mat F; cv::dft(padded, F, cv::DFT_COMPLEX_OUTPUT);
        cv::Mat planes[2]; cv::split(F, planes);
        planes[0] = planes[0].mul(mask); planes[1] = planes[1].mul(mask);
        cv::Mat filtered; cv::merge(planes, 2, filtered);
        cv::Mat outBig, out;
        cv::idft(filtered, outBig, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
        outBig(cv::Rect(0, 0, img8.cols, img8.rows)).copyTo(out);
        outCh.push_back(out);
    }
    std::vector<cv::Mat> out8ch;
    for (auto& o : outCh) {
        // 直接饱和转 8U (保留真实幅度, 便于 PSNR 与干净图比较);
        // 高通等负/超范围值被裁剪.
        cv::Mat n; o.convertTo(n, CV_8U); out8ch.push_back(n);
    }
    cv::Mat result;
    if (out8ch.size() == 1) result = out8ch[0];
    else { cv::merge(out8ch, result); return result; }
    return result;
}

int main(int argc, char** argv) {
    std::string inPath = (argc > 1) ? argv[1] : "../../data/images/lena.jpg";
    cv::Mat img = cv::imread(inPath, cv::IMREAD_COLOR);
    if (img.empty()) { img = cv::Mat(384, 512, CV_8UC3); cv::randu(img, 0, 256); }
    cv::Mat gray, g32;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    int H = cv::getOptimalDFTSize(gray.rows);
    int W = cv::getOptimalDFTSize(gray.cols);

    ensureDir("../out/algorithms");

    // 中心化幅值谱展示 (用灰度)
    cv::Mat grayF;
    gray.convertTo(grayF, CV_32F);
    cv::Mat padded = cv::Mat::zeros(H, W, CV_32F);
    grayF.copyTo(padded(cv::Rect(0, 0, grayF.cols, grayF.rows)));
    cv::Mat complexI; cv::dft(padded, complexI, cv::DFT_COMPLEX_OUTPUT);
    cv::Mat spec = magnitudeSpectrum(complexI);

    // 低通与高通
    cv::Mat lowMask = gaussFilterMask<true>(H, W, 40.0);
    cv::Mat highMask = gaussFilterMask<false>(H, W, 30.0);
    cv::Mat lowOut = applyFreqFilter(img, lowMask, H, W);
    cv::Mat highOut = applyFreqFilter(gray, highMask, H, W);
    // 高通边缘密度
    cv::Mat hg; highOut.convertTo(hg, CV_8U);
    double edgeDW = cv::countNonZero(hg > 40) / (double)hg.total();

    // 周期性噪声 + 陷波去除: 叠加正弦纹波 → 谱上出现两个亮点 → 陷波带阻.
    cv::Mat noisy = img.clone();
    cv::Mat rip = cv::Mat::zeros(img.size(), CV_32FC3);
    double amp = 42.0, fx = 0.08, fy = 0.05;
    for (int y = 0; y < img.rows; ++y)
        for (int x = 0; x < img.cols; ++x) {
            double v = amp * std::sin(2 * CV_PI * (fx * x + fy * y));
            rip.at<cv::Vec3f>(y, x) = cv::Vec3f((float)v, (float)v, (float)v);
        }
    cv::Mat nf32; noisy.convertTo(nf32, CV_32F);
    cv::Mat noisyF = nf32 + rip;
    cv::Mat noisy8; cv::normalize(noisyF, noisy8, 0, 255, cv::NORM_MINMAX, CV_8U);

    // 陷波带阻: 在两噪声峰位置设 0 (近似以中心对称的带阻坑)
    cv::Mat notch = cv::Mat::ones(H, W, CV_32F);
    int px = (int)std::lround(fx * W), py = (int)std::lround(fy * H);
    // 直接置零两个峰值附近区域 (中心化坐标)
    for (int dy = -3; dy <= 3; ++dy) for (int dx = -3; dx <= 3; ++dx) {
        int y0 = H/2 - py + dy, x0 = W/2 - px + dx;
        if (y0 >= 0 && y0 < H && x0 >= 0 && x0 < W) notch.at<float>(y0, x0) = 0.f;
        y0 = H/2 + py + dy; x0 = W/2 + px + dx;
        if (y0 >= 0 && y0 < H && x0 >= 0 && x0 < W) notch.at<float>(y0, x0) = 0.f;
    }
    cv::Mat denoised = applyFreqFilter(noisy8, notch, H, W);

    std::printf("%-18s %10s\n", "metric", "value");
    std::printf("%-18s %10.2f dB\n", "lowpass PSNR(clean)", psnr(img, lowOut));
    std::printf("%-18s %10.4f\n", "highpass edge density", edgeDW);
    std::printf("%-18s %10.2f dB\n", "notch PSNR(clean)", psnr(img, denoised));
    std::printf("%-18s %10.2f dB\n", "noisy PSNR(clean)", psnr(img, noisy8));

    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(img); labels.push_back("input");
    panels.push_back(spec); labels.push_back("DFT magnitude");
    panels.push_back(lowOut); labels.push_back("gaussian lowpass");
    panels.push_back(highOut); labels.push_back("gaussian highpass");
    panels.push_back(noisy8); labels.push_back("+periodic noise");
    panels.push_back(denoised); labels.push_back("notch removed");
    cv::Mat canvas = gridWithLabels(panels, labels, 3, 28);
    std::string out = "../out/algorithms/frequency_domain_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[frequency_domain] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}