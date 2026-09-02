// algorithms/hough_transform/main.cpp
// Hough transform: line (HoughLinesP) + circle (HoughCircles) detection.
//
// Lines:   probabilistic Hough line detection on Canny edges, draw segments on sudoku/building.
// Circles: HOUGH_GRADIENT/(ALT) detection on round images such as smarties/star trails.
// Notes:   prints detection counts + line angle/length and circle radius statistics.
// Output:  out/algorithms/hough_lines.png, hough_circles.png.
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

static void drawLines(const cv::Mat& gray, const cv::Mat& rgbOut,
                      double canny1 = 50, double canny2 = 150, int minLen = 40) {
    cv::Mat edges;
    cv::Canny(gray, edges, canny1, canny2);
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180.0, 80, minLen, 20);
    int nBig = 0; double angSum = 0, lenSum = 0;
    for (auto& l : lines) {
        cv::line(rgbOut, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
                 cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
        double ang = std::atan2((double)l[3] - l[1], (double)l[2] - l[0]) * 180.0 / CV_PI;
        double len = std::hypot((double)l[3] - l[1], (double)l[2] - l[0]);
        if (len > 100) { nBig++; angSum += ang; lenSum += len; }
    }
    std::printf("lines total=%zu  long(>100px)=%d  meanAng=%.1f deg  meanLen=%.1f px\n",
                lines.size(), nBig, nBig ? angSum / nBig : 0, nBig ? lenSum / nBig : 0);
}

static void drawCircles(const cv::Mat& bgr, const cv::Mat& rgbOut,
                        double dp = 1.0, double minDist = 40,
                        double p1 = 100, double p2 = 30,
                        int minR = 20, int maxR = 200) {
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::Mat gb;
    cv::GaussianBlur(gray, gb, cv::Size(9, 9), 3.0, 3.0);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(gb, circles, cv::HOUGH_GRADIENT, dp, minDist, p1, p2, minR, maxR);
    double rSum = 0, rMax = 0;
    for (auto& c : circles) {
        cv::circle(rgbOut, cv::Point(cvRound(c[0]), cvRound(c[1])),
                   cvRound(c[2]), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
        cv::circle(rgbOut, cv::Point(cvRound(c[0]), cvRound(c[1])),
                   2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
        rSum += c[2]; rMax = std::max<double>(rMax, c[2]);
    }
    std::printf("circles=%zu  meanR=%.1f  maxR=%.1f\n",
                circles.size(), circles.empty() ? 0 : rSum / circles.size(), rMax);
}

int main(int argc, char** argv) {
    std::string lineImg = (argc > 1) ? argv[1] : "../../data/sudoku.png";
    std::string circleSrc = (argc > 2) ? argv[2] : "../../data/smarties.png";

    ensureDir("../out/algorithms");

    // ---- lines ----
    cv::Mat imgL = cv::imread(lineImg, cv::IMREAD_COLOR);
    if (imgL.empty()) { imgL = cv::Mat(400, 600, CV_8UC3, cv::Scalar(255,255,255));
                        cv::line(imgL, cv::Point(30,40), cv::Point(560,300), cv::Scalar(0,0,0), 3);
                        cv::line(imgL, cv::Point(560,40), cv::Point(40,320), cv::Scalar(0,0,0), 3); }
    if (std::max(imgL.rows, imgL.cols) > 900) {
        double s = 900.0 / std::max(imgL.rows, imgL.cols);
        cv::resize(imgL, imgL, cv::Size(), s, s, cv::INTER_AREA);
    }
    cv::Mat grayL; cv::cvtColor(imgL, grayL, cv::COLOR_BGR2GRAY);
    cv::Mat edgesL; cv::Canny(grayL, edgesL, 50, 150);
    cv::Mat linesOut; imgL.copyTo(linesOut);
    std::printf("[lines] %s\n", lineImg.c_str());
    drawLines(grayL, linesOut);
    std::vector<cv::Mat> panel1 = {imgL, edgesL, linesOut};
    std::vector<std::string> lab1 = {"input", "Canny", "HoughLinesP"};
    cv::Mat canvas1 = gridWithLabels(panel1, lab1, 3, 26);
    cv::imwrite("../out/algorithms/hough_lines.png", canvas1);

    // ---- circles ----
    cv::Mat imgC = cv::imread(circleSrc, cv::IMREAD_COLOR);
    if (imgC.empty()) { imgC = cv::Mat(360, 460, CV_8UC3, cv::Scalar(255,255,255));
                        for (int i=0;i<6;++i) cv::circle(imgC, cv::Point(60+i*74, 60+(i%2)*80),
                                                         cvRound(24+i*3), cv::Scalar(0,0,0), 2); }
    if (std::max(imgC.rows, imgC.cols) > 700) {
        double s = 700.0 / std::max(imgC.rows, imgC.cols);
        cv::resize(imgC, imgC, cv::Size(), s, s, cv::INTER_AREA);
    }
    cv::Mat circlesOut; imgC.copyTo(circlesOut);
    std::printf("[circles] %s\n", circleSrc.c_str());
    drawCircles(imgC, circlesOut);
    std::vector<cv::Mat> panel2 = {imgC, circlesOut};
    std::vector<std::string> lab2 = {"input", "HoughCircles"};
    cv::Mat canvas2 = gridWithLabels(panel2, lab2, 2, 26);
    cv::imwrite("../out/algorithms/hough_circles.png", canvas2);

    std::cout << "[hough_transform] wrote hough_lines.png + hough_circles.png\n";
    return 0;
}
