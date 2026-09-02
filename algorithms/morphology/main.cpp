// algorithms/morphology/main.cpp
// 形态学 (Morphology) 处理对比演示. 针对二值 mask 做全套形态学操作,
// 并演示两个实用场景: 用开运算去毛刺 / 用闭运算填补空洞.
//
// 覆盖算子 (cv::morphologyEx 与手工 erode/dilate):
//   基础: 腐蚀 erode / 膨胀 dilate
//   组合: 开 open = erode→dilate / 闭 close = dilate→erode
//         形态学梯度 gradient = dilate − erode
//         顶帽 tophat = src − open / 黑帽 blackhat = close − src
//   进阶: hit-or-miss (角点检测)
//   SE:  结构元素矩形/椭圆/十字 对比; 核大小 3/5/7 对比
//
// 输出: out/algorithms/morphology_compare.png 全景 + 像素变化统计.
// 用法: morphology.exe [input_img] [bin_threshold(缺省 Otsu)]
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// 对 8UC1 二值图做指定形态学操作.
static cv::Mat morph(const cv::Mat& bin, int op, int ksize = 3,
                     int shape = cv::MORPH_RECT) {
    cv::Mat kernel = cv::getStructuringElement(shape, cv::Size(ksize, ksize));
    cv::Mat out;
    cv::morphologyEx(bin, out, op, kernel);
    return out;
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    if (argc > 1) inPath = argv[1];

    cv::Mat src = cv::imread(inPath, cv::IMREAD_GRAYSCALE);
    if (src.empty()) { log("morphology", "input empty, synth"); src = cv::Mat(512, 512, CV_8UC1); cv::randu(src, 0, 256); }
    if (std::max(src.rows, src.cols) > 700) {
        double s = 700.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }

    // 二值化得到前景 mask (白色=前景). Otsu 自动选阈值.
    cv::Mat bin;
    double thr = (argc > 2) ? std::stod(argv[2]) :
                 cv::threshold(src, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    if (argc > 2) cv::threshold(src, bin, thr, 255, cv::THRESH_BINARY);
    std::printf("[morphology] threshold = %.1f\n", thr);

    ensureDir("../out/algorithms");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(src); labels.push_back("gray input");
    panels.push_back(bin); labels.push_back("binary(mask)");

    // ---- 1. 基础: 腐蚀 / 膨胀 (结构元素 3x3 矩形) ----
    cv::Mat k3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    cv::Mat er, di;
    cv::erode(bin, er, k3);
    cv::dilate(bin, di, k3);
    panels.push_back(er); labels.push_back("erode(3x3)");
    panels.push_back(di); labels.push_back("dilate(3x3)");

    // ---- 2. 组合: 开 / 闭 / 形态学梯度 / 顶帽 / 黑帽 ----
    panels.push_back(morph(bin, cv::MORPH_OPEN));   labels.push_back("open(3x3)");
    panels.push_back(morph(bin, cv::MORPH_CLOSE));  labels.push_back("close(3x3)");
    panels.push_back(morph(bin, cv::MORPH_GRADIENT)); labels.push_back("gradient(3x3)");
    panels.push_back(morph(bin, cv::MORPH_TOPHAT)); labels.push_back("tophat(3x3)");
    panels.push_back(morph(bin, cv::MORPH_BLACKHAT)); labels.push_back("blackhat(3x3)");

    // ---- 3. 结构元素形状对比 (开运算) ----
    {
        cv::Mat ker1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
        cv::Mat ker2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5));
        cv::Mat ker3 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(5,5));
        cv::Mat o1, o2, o3;
        cv::morphologyEx(bin, o1, cv::MORPH_OPEN, ker1);
        cv::morphologyEx(bin, o2, cv::MORPH_OPEN, ker2);
        cv::morphologyEx(bin, o3, cv::MORPH_OPEN, ker3);
        panels.push_back(o1); labels.push_back("open-ellipse(5)");
        panels.push_back(o2); labels.push_back("open-cross(5)");
        panels.push_back(o3); labels.push_back("open-rect(5)");
    }

    // ---- 4. 核大小对比 (矩形闭运算, 填补细小空洞) ----
    for (int k : {5, 9}) {
        panels.push_back(morph(bin, cv::MORPH_CLOSE, k, cv::MORPH_ELLIPSE));
        labels.push_back("close-Ell(" + std::to_string(k) + ")");
    }

    // ---- 5. hit-or-miss: 用一对互补核检测 2x2 全前景角点块 ----
    {
        // 真实 hit-or-miss 需要 fg 核命中前景且 bg 核命中背景 (背景核 = !内核)
        // 这里用 opencv 的 hit-or-miss (fg 归一化), 输出"角点"位置的白色标记.
        cv::Mat kHit = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2,2));
        cv::Mat hit;
        cv::morphologyEx(bin, hit, cv::MORPH_HITMISS, kHit);
        // 3x3 内只有中心为前景才命中 → 用于"像素完全孤立检测", 演示用途
        cv::Mat thin;
        cv::erode(bin, thin, kHit);
        panels.push_back(hit);   labels.push_back("hit-miss(2x2)");
        panels.push_back(thin);  labels.push_back("erode(2x2)");
    }

    // ---- 指标: 前景面积占比 / 连通域数量 / 变化量 ----
    std::printf("%-28s %14s %10s\n", "panel", "fg_pixels", "components");
    for (size_t i = 0; i < panels.size(); ++i) {
        if (i == 0) continue;  // 跳过 gray input
        cv::Mat g;
        if (panels[i].channels() == 3) cv::cvtColor(panels[i], g, cv::COLOR_BGR2GRAY);
        else g = panels[i];
        cv::Mat b = g >= 8;
        double fg = double(cv::countNonZero(b)) / double(b.total()) * 100.0;
        cv::Mat labelsMat, stats, cent;
        int n = cv::connectedComponents(b, labelsMat, 8);
        std::printf("%-28s %12.2f%% %10d\n",
                    labels[i].c_str(), fg, n - 1);
    }

    cv::Mat canvas = gridWithLabels(panels, labels, 4, 30);
    std::string out = "../out/algorithms/morphology_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[morphology] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}