// algorithms/inpaint/main.cpp
// 图像修复 (inpainting): 划痕/文本遮挡区域的恢复.
//
// 算法:
//   INPAINT_TELEA   Telea (基于快速步进法, 沿等照度线填充)
//   INPAINT_NS      Navier-Stokes (流场引导, 保留边缘)
//   半径参数 r_ 影响扩散范围
// 评估: 对干净图人为制造划痕/文本遮挡 → 修复后与干净图在"遮挡区"内做
//       MAE/PSNR, 同时给出整图 PSNR (内容被保持得如何).
// 输出: out/algorithms/inpaint_compare.png + 指标表.
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// 在 img 上画若干白色划痕 + 一块文本区域, 返回 mask (255=受损).
static void drawDamage(cv::Mat& img, cv::Mat& mask) {
    mask = cv::Mat::zeros(img.size(), CV_8UC1);
    cv::RNG rng(12345);
    // 几条随机细线划痕
    for (int i = 0; i < 5; ++i) {
        cv::Point p1(rng.uniform(5, img.cols - 5), rng.uniform(5, img.rows - 5));
        cv::Point p2(p1.x + rng.uniform(-150, 150), p1.y + rng.uniform(-60, 60));
        cv::line(img, p1, p2, cv::Scalar(255, 255, 255), 2);
        cv::line(mask, p1, p2, cv::Scalar(255), 3);
    }
    // 一块文本样式的遮挡块
    cv::Rect textBox(30, 30, 180, 46);
    cv::rectangle(img, textBox, cv::Scalar(255, 255, 255), cv::FILLED);
    cv::putText(img, "HELP?", cv::Point(38, 64), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                cv::Scalar(0, 0, 0), 2);
    cv::rectangle(mask, textBox, cv::Scalar(255), cv::FILLED);
}

// 只在 mask 区域内评估 MAE (重建质量).
static double maskedMAE(const cv::Mat& ref, const cv::Mat& out, const cv::Mat& mask) {
    cv::Mat df, ref32, out32, m32;
    ref.convertTo(ref32, CV_32F); out.convertTo(out32, CV_32F);
    mask.convertTo(m32, CV_32F);
    cv::absdiff(ref32, out32, df);
    df = df.mul(m32);  // 用 mask[0..1] 作为权重, 等价于只看受损区
    int n = cv::countNonZero(mask);
    return n ? cv::sum(df)[0] / (double)n : 0;
}

int main(int argc, char** argv) {
    std::string inPath = (argc > 1) ? argv[1] : "../../data/images/lena.jpg";
    cv::Mat clean = cv::imread(inPath, cv::IMREAD_COLOR);
    if (clean.empty()) { clean = cv::Mat(360, 480, CV_8UC3); cv::randu(clean, 0, 256); }
    if (std::max(clean.rows, clean.cols) > 520) {
        double s = 520.0 / std::max(clean.rows, clean.cols);
        cv::resize(clean, clean, cv::Size(), s, s, cv::INTER_AREA);
    }
    ensureDir("../out/algorithms");

    cv::Mat damaged, mask;
    clean.copyTo(damaged);
    drawDamage(damaged, mask);

    struct R { std::string tag; cv::Mat img8; double fullPSNR, maskMAE; };
    std::vector<R> res;
    for (int method : {cv::INPAINT_TELEA, cv::INPAINT_NS}) {
        for (int r : {3, 8}) {
            cv::Mat out;
            cv::inpaint(damaged, mask, out, r, method);
            const char* nm = (method == cv::INPAINT_TELEA) ? "TELEA" : "NS";
            res.push_back({std::string(nm) + " r=" + std::to_string(r), out,
                           psnr(clean, out), maskedMAE(clean, out, mask)});
        }
    }
    std::printf("damaged pixels = %.2f%%\n", 100.0 * cv::countNonZero(mask) / (double)(mask.total()));
    std::printf("%-14s %10s %10s\n", "method", "wholePSNR", "maskMAE");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(clean); labels.push_back("clean(ref)");
    panels.push_back(damaged); labels.push_back("damaged");
    panels.push_back(mask); labels.push_back("mask");
    for (auto& r : res) {
        std::printf("%-14s %10.2f %10.1f\n", r.tag.c_str(), r.fullPSNR, r.maskMAE);
        panels.push_back(r.img8); labels.push_back(r.tag);
    }
    cv::Mat canvas = gridWithLabels(panels, labels, 3, 30);
    std::string out = "../out/algorithms/inpaint_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[inpaint] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}