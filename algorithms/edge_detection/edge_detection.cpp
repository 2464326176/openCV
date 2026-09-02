// algorithms/edge_detection/main.cpp
// 边缘检测算法综合对比演示 (经典梯度类算子 + Laplacian/LoG + Canny + DoG).
//
// 覆盖算法:
//   梯度类:   Sobel / Scharr / Prewitt (2 方向梯度幅值)
//   二阶类:   Laplacian / LoG (高斯-拉普拉斯) / DoG (高斯差分)
//   最优化:   Canny (传统双阈值) 与 自动双阈值 Canny (Otsu)
//
// 指标: 边缘密度 (edge 像素占比), 分块边缘熵, 连通分量平均规模 (粗略连续性).
// 输出: 全景网格图 out/algorithms/edge_detection_<sub>.png + 指标表.
//
// 用法: edge_detection.exe [input_img] [sigma_or_threshold]
#include "../common/algo_utils.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// 基于导数的梯度幅值检测 (sx/sy 为两个方向的卷积核).
static cv::Mat gradientMag(const cv::Mat& gray, cv::InputArray sx, cv::InputArray sy) {
    cv::Mat gx, gy;
    cv::filter2D(gray, gx, CV_32F, sx);
    cv::filter2D(gray, gy, CV_32F, sy);
    cv::Mat mag;
    cv::magnitude(gx, gy, mag);
    cv::Mat m8;
    cv::normalize(mag, m8, 0, 255, cv::NORM_MINMAX, CV_8U);
    return m8;
}

// 边缘像素占比: gray > thr 视为边缘 (thr 针对已归一化 0~255 的响应).
static double edgeDensity(const cv::Mat& gray, int thr = 40) {
    return cv::countNonZero(gray > thr) / double(gray.total());
}

// 连通分量平均规模: 衡量分块边缘连续性 (越大连续性越好).
static double avgComponentSize(const cv::Mat& gray, int thr = 40) {
    cv::Mat bin = gray > thr;
    cv::Mat labels, stats, cent;
    int n = cv::connectedComponentsWithStats(bin, labels, stats, cent);
    if (n <= 1) return 0;
    double sum = 0, cnt = 0;
    for (int i = 1; i < n; ++i) {
        double area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area < 3) continue;  // 忽略孤立噪声点
        sum += area; cnt += 1;
    }
    return cnt ? sum / cnt : 0;
}

struct Result { std::string tag; cv::Mat img8; double density, continuity; };

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    if (argc > 1) inPath = argv[1];

    cv::Mat src = cv::imread(inPath, cv::IMREAD_COLOR);
    if (src.empty()) { log("edge_detection", "input empty, synth"); src = cv::Mat(512, 512, CV_8UC3); cv::randu(src, 0, 256); }
    if (std::max(src.rows, src.cols) > 900) {
        double s = 900.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::Mat g32; gray.convertTo(g32, CV_32F);

    ensureDir("../out/algorithms");
    std::vector<Result> res;

    // ---- 1. 基于导数的一阶梯度算子 ----
    {
        cv::Mat sx9 = (cv::Mat_<float>(3,3) << -1,0,1, -2,0,2, -1,0,1);   // Sobel X
        cv::Mat sy9 = (cv::Mat_<float>(3,3) << -1,-2,-1, 0,0,0, 1,2,1);   // Sobel Y
        res.push_back({"Sobel(3x3)", gradientMag(gray, sx9, sy9), 0, 0});

        cv::Mat sxS = (cv::Mat_<float>(3,3) << -3,0,3, -10,0,10, -3,0,3); // Scharr X
        cv::Mat syS = (cv::Mat_<float>(3,3) << -3,-10,-3, 0,0,0, 3,10,3); // Scharr Y
        res.push_back({"Scharr(3x3)", gradientMag(gray, sxS, syS), 0, 0});

        cv::Mat sxP = (cv::Mat_<float>(3,3) << -1,0,1, -1,0,1, -1,0,1);   // Prewitt X
        cv::Mat syP = (cv::Mat_<float>(3,3) << -1,-1,-1, 0,0,0, 1,1,1);   // Prewitt Y
        res.push_back({"Prewitt(3x3)", gradientMag(gray, sxP, syP), 0, 0});
    }

    // ---- 2. 二阶算子是零交叉 / 归一化拉普拉斯 ----
    {
        // 二阶响应带符号, 取绝对值得到与梯度一致的"边缘强度", 阈值才有意义.
        cv::Mat lap, lap8;
        cv::Laplacian(gray, lap, CV_32F, 3);
        cv::Mat lapAbs = cv::abs(lap);
        cv::normalize(lapAbs, lap8, 0, 255, cv::NORM_MINMAX, CV_8U);
        res.push_back({"Laplacian(3)", lap8, 0, 0});

        cv::Mat gblur;
        cv::GaussianBlur(gray, gblur, cv::Size(5,5), 3.0);
        cv::Mat logm, logm8;
        cv::Laplacian(gblur, logm, CV_32F, 3);
        cv::Mat logmAbs = cv::abs(logm);
        cv::normalize(logmAbs, logm8, 0, 255, cv::NORM_MINMAX, CV_8U);
        res.push_back({"LoG(sigma=3)", logm8, 0, 0});

        cv::Mat g1, g2;
        cv::GaussianBlur(gray, g1, cv::Size(), 1.0);
        cv::GaussianBlur(gray, g2, cv::Size(), 3.0);
        cv::Mat dog, dog8;
        dog = g1 - g2;   // DoG: 高斯差分近似 LoG, 边缘/角点响应
        cv::Mat dogAbs = cv::abs(dog);
        cv::normalize(dogAbs, dog8, 0, 255, cv::NORM_MINMAX, CV_8U);
        res.push_back({"DoG(1.0=>3.0)", dog8, 0, 0});
    }

    // ---- 3. Canny: 经典双阈值 & Otsu 自动阈值 ----
    {
        cv::Mat canny;
        cv::Canny(gray, canny, 100, 200);
        cv::Mat show; cv::cvtColor(canny, show, cv::COLOR_GRAY2BGR);
        res.push_back({"Canny(100,200)", show, 0, 0});

        cv::Mat otsuBin;
        double otsuThr = cv::threshold(gray, otsuBin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        double low  = std::max(0.0, otsuThr * 0.5);
        double high = std::min(255.0, otsuThr * 1.33);
        cv::Mat autoCanny;
        cv::Canny(gray, autoCanny, low, high);
        cv::Mat show2; cv::cvtColor(autoCanny, show2, cv::COLOR_GRAY2BGR);
        res.push_back({"Canny-Otsu(" + std::to_string((int)low) + "," + std::to_string((int)high) + ")", show2, 0, 0});
    }

    // ---- 指标: 边缘密度 + 连续性 (固定低阈值 40, 针对归一化 0~255 响应) ----
    std::printf("%-30s %10s %12s\n", "method", "density", "avg_comp_size");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(src); labels.push_back("input");
    for (auto& r : res) {
        cv::Mat gray2;
        if (r.img8.channels() == 3) cv::cvtColor(r.img8, gray2, cv::COLOR_BGR2GRAY);
        else gray2 = r.img8;
        r.density = edgeDensity(gray2, 40);
        r.continuity = avgComponentSize(gray2, 40);
        std::printf("%-30s %8.3f%% %12.1f\n",
                    r.tag.c_str(), r.density * 100.0, r.continuity);
        panels.push_back(r.img8); labels.push_back(r.tag);
    }
    cv::Mat canvas = gridWithLabels(panels, labels, 3, 30);
    std::string out = "../out/algorithms/edge_detection_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[edge_detection] wrote " << out << " (cols=" << canvas.cols << " rows=" << canvas.rows << ")\n";
    return 0;
}