// algorithms/feature_detection/main.cpp
// 特征点检测与匹配对比演示.
//
// 覆盖检测器 (不依赖 opencv_contrib, 均为主库 features2d/imgproc):
//   角点:     Harris (cornerHarris) / Shi-Tomasi (goodFeaturesToTrack)
//   二进制:   FAST / ORB / BRISK / AKAZE
// 匹配:   ORB 描述符 → BFMatcher(HAMMING) + knn 比例测试 →
//         用 findHomography(RANSAC) 剔除离群点、计算单应性内点率.
//
// 输入: data/graf1.png 与 data/graf3.png 构成常见"视角变化匹配"样本对.
// 输出: out/algorithms/feature_detection_compare.png (检测可视化)
//       out/algorithms/feature_matching.png (匹配 + 单应内点)
#include "../common/algo_utils.hpp"

#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>

#include <cstdio>
#include <cmath>
#include <vector>

using namespace algo;

// 在一张图里画 keypoints, 返回 BGR 可视化.
static cv::Mat drawPts(const cv::Mat& gray, const std::vector<cv::KeyPoint>& kps,
                       const cv::Scalar& color = cv::Scalar(0, 0, 255)) {
    cv::Mat out;
    cv::cvtColor(gray, out, cv::COLOR_GRAY2BGR);
    cv::drawKeypoints(out, kps, out, color,
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    return out;
}

int main(int argc, char** argv) {
    std::string base = "../../data";
    std::string p1 = base + "/graf1.png";
    std::string p2 = base + "/graf3.png";
    if (argc > 1) p1 = argv[1];
    if (argc > 2) p2 = argv[2];

    cv::Mat g1 = cv::imread(p1, cv::IMREAD_GRAYSCALE);
    cv::Mat g2r = cv::imread(p2, cv::IMREAD_GRAYSCALE);
    if (g1.empty()) { log("feature_detection", "img1 empty: " + p1); g1 = cv::Mat(480,640,CV_8UC1); cv::randu(g1,0,256); }
    if (g2r.empty()) { log("feature_detection", "img2 empty: " + p2); g2r = g1; }
    // 统一尺寸便于拼接
    if (g1.size() != g2r.size()) {
        cv::resize(g2r, g2r, g1.size(), 0, 0, cv::INTER_AREA);
    }

    ensureDir("../out/algorithms");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(drawPts(g1, {})); labels.push_back("img1(graf1)");
    panels.push_back(drawPts(g2r, {})); labels.push_back("img2(graf3)");

    std::printf("%-22s %12s\n", "detector", "keypoints");

    // ---- 1. 角点类 ----
    {
        // Shi-Tomasi (goodFeaturesToTrack)
        std::vector<cv::Point2f> corners;
        cv::goodFeaturesToTrack(g1, corners, 400, 0.01, 10);
        std::vector<cv::KeyPoint> kps;
        for (auto& p : corners) kps.push_back(cv::KeyPoint(p, 7));
        panels.push_back(drawPts(g1, kps)); labels.push_back("Shi-Tomasi");
        std::printf("%-22s %12zu\n", "Shi-Tomasi", kps.size());

        // Harris
        cv::Mat harris;
        cv::cornerHarris(g1, harris, 2, 3, 0.04);
        cv::Mat harris_n;
        cv::normalize(harris, harris_n, 0, 255, cv::NORM_MINMAX, CV_8U);
        std::vector<cv::KeyPoint> hk;
        double maxv;
        cv::minMaxLoc(harris, 0, &maxv);
        for (int y = 1; y + 1 < harris.rows; ++y)
            for (int x = 1; x + 1 < harris.cols; ++x)
                if (harris.at<float>(y, x) > 0.01 * maxv) hk.push_back(cv::KeyPoint((float)x, (float)y, 5));
        panels.push_back(drawPts(g1, hk)); labels.push_back("Harris");
        std::printf("%-22s %12zu\n", "Harris", hk.size());
    }

    // ---- 2. 二进制检测器 ----
    auto detect_show = [&](const std::string& name,
                           const cv::Ptr<cv::Feature2D>& det) {
        std::vector<cv::KeyPoint> kps;
        det->detect(g1, kps);
        panels.push_back(drawPts(g1, kps)); labels.push_back(name);
        std::printf("%-22s %12zu\n", name.c_str(), kps.size());
    };
    detect_show("FAST", cv::FastFeatureDetector::create(20, true));
    detect_show("ORB(500)", cv::ORB::create(500));
    detect_show("BRISK", cv::BRISK::create());
    detect_show("AKAZE", cv::AKAZE::create());

    cv::Mat canvas = gridWithLabels(panels, labels, 3, 30);
    std::string out1 = "../out/algorithms/feature_detection_compare.png";
    cv::imwrite(out1, canvas);
    std::cout << "[feature_detection] wrote " << out1 << "\n";

    // ---- 3. ORB 匹配 + 单应性验证 ----
    cv::Mat d1, d2;
    std::vector<cv::KeyPoint> k1, k2;
    auto orb = cv::ORB::create(1000);
    orb->detectAndCompute(g1, cv::noArray(), k1, d1);
    orb->detectAndCompute(g2r, cv::noArray(), k2, d2);
    if (d1.empty() || d2.empty()) {
        std::cout << "[feature_detection] no descriptors, skip matching\n";
        return 0;
    }
    cv::BFMatcher bf(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn;
    bf.knnMatch(d1, d2, knn, 2);
    std::vector<cv::DMatch> good;
    for (auto& m : knn)
        if (m.size() == 2 && m[0].distance < 0.75 * m[1].distance) good.push_back(m[0]);

    std::vector<cv::Point2f> pts1, pts2;
    for (auto& m : good) { pts1.push_back(k1[m.queryIdx].pt); pts2.push_back(k2[m.trainIdx].pt); }
    std::vector<uchar> inliers;
    cv::Mat H;
    int nInlier = 0;
    if (pts1.size() >= 8) {
        H = cv::findHomography(pts1, pts2, cv::RANSAC, 3.0, inliers);
        for (auto v : inliers) nInlier += (v == 1);
    }
    std::printf("[feature_detection] ORB match: good=%zu inliers=%d ratio=%.1f%%\n",
                good.size(), nInlier, good.empty() ? 0.0 : 100.0 * nInlier / good.size());

    cv::Mat matchImg, matchInlier;
    cv::drawMatches(g1, k1, g2r, k2, good, matchImg, cv::Scalar::all(-1),
                    cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    // 仅内点
    std::vector<cv::DMatch> inl;
    for (size_t i = 0; i < good.size(); ++i) if (!inliers.empty() && inliers[i]) inl.push_back(good[i]);
    cv::drawMatches(g1, k1, g2r, k2, inl, matchInlier, cv::Scalar(0, 255, 0),
                    cv::Scalar(0, 0, 255), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    // 用 H 把 img1 的四个角重投影像到 img2 (拼接图右侧, 需偏移 img1 宽), 验证单应性.
    if (!H.empty()) {
        std::vector<cv::Point2f> corners = {
            cv::Point2f(0.f, 0.f), cv::Point2f((float)g1.cols, 0.f),
            cv::Point2f((float)g1.cols, (float)g1.rows), cv::Point2f(0.f, (float)g1.rows)};
        std::vector<cv::Point2f> dst;
        cv::perspectiveTransform(corners, dst, H);
        for (auto& p : dst) p.x += g1.cols;
        if (matchImg.channels() == 1) cv::cvtColor(matchImg, matchImg, cv::COLOR_GRAY2BGR);
        std::vector<cv::Point> poly;
        bool ok = true;
        for (auto& p : dst) {
            if (!std::isfinite(p.x) || !std::isfinite(p.y)) { ok = false; break; }  // 退化/噪声单应
            poly.push_back(cv::Point(cvRound(p.x), cvRound(p.y)));
        }
        if (ok) cv::polylines(matchImg, poly, true, cv::Scalar(0, 255, 255), 2, cv::LINE_AA);
    }
    // 直接拼两张匹配图
    std::vector<cv::Mat> m2 = {matchInlier, matchImg};
    cv::Mat matchCanvas = gridWithLabels(m2, {"inliers_only", "all_good + H-quad"}, 2, 30);
    std::string out2 = "../out/algorithms/feature_matching.png";
    cv::imwrite(out2, matchCanvas);
    std::cout << "[feature_detection] wrote " << out2 << "\n";
    return 0;
}