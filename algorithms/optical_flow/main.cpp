// algorithms/optical_flow/main.cpp
// 光流估计: 稀疏 Lucas-Kanade (金字塔) + 稠密 Farneback.
//
// 输入: 视频两帧 (默认 data/vtest.avi 第 A/B 帧), 或两张图片.
// 输出: out/algorithms/optical_flow_compare.png
//       - LK:   Shi-Tomasi 角点 + 位移向量箭头
//       - Farneback: HSV 速度场 (Hue=方向, Value=大小), 或箭头场
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// 稠密光流 → HSV 可视化 (H=D方向, S=1, V=归一幅度).
static cv::Mat flowToHSV(const cv::Mat& flow) {
    cv::Mat ch[2];
    cv::split(flow, ch);
    cv::Mat hsv(flow.size(), CV_8UC3);
    cv::Mat mag, ang;
    cv::cartToPolar(ch[0], ch[1], mag, ang, true);
    double mmax = 12.0; cv::minMaxLoc(mag, nullptr, &mmax);
    mmax = std::max(mmax, 1.0);
    for (int y = 0; y < flow.rows; ++y)
        for (int x = 0; x < flow.cols; ++x) {
            uchar h = (uchar)((ang.at<float>(y, x) / 360.0) * 180.0);
            uchar v = (uchar)std::min(255.0, 255.0 * mag.at<float>(y, x) / mmax);
            hsv.at<cv::Vec3b>(y, x) = cv::Vec3b(h, 255, v);
        }
    cv::Mat bgr;
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
    return bgr;
}

int main(int argc, char** argv) {
    std::string video = (argc > 1) ? argv[1] : "../../data/vtest.avi";
    int f1 = (argc > 2) ? std::atoi(argv[2]) : 5;
    int f2 = (argc > 3) ? std::atoi(argv[3]) : 15;

    cv::Mat prev8, next8;
    cv::VideoCapture cap(video);
    if (cap.isOpened()) {
        cap.set(cv::CAP_PROP_POS_FRAMES, f1);
        cap.read(prev8);
        cap.set(cv::CAP_PROP_POS_FRAMES, f2);
        cap.read(next8);
    } else {
        log("optical_flow", "video open failed, use synthetic affine motion");
        // 合成可控运动: 对基础图做"平移+轻微旋转"得到下一帧, 便于直观核验光流.
        prev8 = cv::imread("../../data/images/lena.jpg");
        if (prev8.empty()) { prev8 = cv::Mat(320, 400, CV_8UC3); cv::randu(prev8, 0, 256); }
        cv::Mat M = (cv::Mat_<double>(2, 3) <<
                    0.985, -0.035, 12.0,
                    0.035,  0.985, -8.0);
        cv::warpAffine(prev8, next8, M, prev8.size(), cv::INTER_LINEAR, cv::BORDER_REFLECT);
    }
    if (prev8.empty() || next8.empty()) {
        prev8 = cv::Mat(240, 320, CV_8UC3); cv::randu(prev8, 0, 256);
        next8 = prev8.clone();
        cv::warpAffine(next8, next8, cv::Matx23f(1,0,-6, 0,1,-4), next8.size());
        log("optical_flow", "empty input, synthetic motion");
    }
    if (prev8.size() != next8.size())
        cv::resize(next8, next8, prev8.size());

    cv::Mat gp, gn;
    cv::cvtColor(prev8, gp, cv::COLOR_BGR2GRAY);
    cv::cvtColor(next8, gn, cv::COLOR_BGR2GRAY);
    ensureDir("../out/algorithms");

    // ---- 1. LK 稀疏 ----
    cv::Mat prevPts;
    std::vector<cv::Point2f> p0;
    cv::goodFeaturesToTrack(gp, p0, 300, 0.01, 10);
    std::vector<cv::Point2f> p1;
    std::vector<uchar> status; std::vector<float> err;
    cv::calcOpticalFlowPyrLK(gp, gn, p0, p1, status, err,
                             cv::Size(21, 21), 3);
    cv::Mat lkImg; next8.copyTo(lkImg);
    int tracked = 0; double mlen = 0;
    for (size_t i = 0; i < p0.size(); ++i) {
        if (!status[i]) continue;
        tracked++;
        cv::Point a = p0[i], b = p1[i];
        double d = cv::norm(b - a);
        mlen += d;
        if (d < 0.01) continue;
        cv::arrowedLine(lkImg, a, b, cv::Scalar(0, 0, 255), 1, cv::LINE_AA, 0, 0.3);
        cv::circle(lkImg, b, 2, cv::Scalar(0, 255, 0), -1);
    }
    std::printf("LK tracked=%d/%zu  meanVecLen=%.2f px\n",
                tracked, p0.size(), tracked ? mlen / tracked : 0.0);

    // ---- 2. Farneback 稠密 ----
    cv::Mat flow;
    cv::calcOpticalFlowFarneback(gp, gn, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
    cv::Mat fHSV = flowToHSV(flow);
    cv::Mat arrows; next8.copyTo(arrows);
    const int step = 16;
    long total = 0; double angSum = 0;
    for (int y = 0; y < flow.rows; y += step)
        for (int x = 0; x < flow.cols; x += step) {
            cv::Point2f v = flow.at<cv::Point2f>(y, x);
            double d = cv::norm(v);
            if (d > 0.5) {
                total++;
                angSum += std::atan2(v.y, v.x);
                cv::arrowedLine(arrows, cv::Point(x, y), cv::Point(cvRound(x + v.x * 4), cvRound(y + v.y * 4)),
                                cv::Scalar(0, 0, 255), 1, cv::LINE_AA, 0, 0.4);
            }
        }
    std::printf("Farneback activeCells=%ld  meanAng=%.1f deg\n",
                total, total ? angSum / total * 180.0 / CV_PI : 0);

    std::vector<cv::Mat> panels = {prev8, next8, lkImg, fHSV, arrows};
    std::vector<std::string> labels = {"frameA", "frameB", "LK sparse",
                                       "Farneback(HSV flow)", "Farneback arrows"};
    cv::Mat canvas = gridWithLabels(panels, labels, 3, 26);
    std::string out = "../out/algorithms/optical_flow_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[optical_flow] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}