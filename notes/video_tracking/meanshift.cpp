//********************
// Author:  yh
// MeanShift 目标跟踪：
//  流程：首帧固定窗口 → 计算目标 H 通道直方图 → 每帧反投影 + meanShift 迭代
//  与 CamShift 区别：窗口大小固定不变，不输出旋转矩形
//  对应官方示例: tutorial_code/video/meanshift/meanshift.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    String videoName("../data/vtest.avi");
    if (argc > 1) videoName = argv[1];

    VideoCapture capture(videoName);
    if (!capture.isOpened()) { cout << "无法打开视频" << endl; return -1; }

    Mat frame;
    capture >> frame;
    if (frame.empty()) return -1;

    // 初始跟踪窗口（硬编码位置，实际可鼠标框选）
    Rect track_window(300, 200, 100, 50);

    // 1. 用首帧窗口内像素建立目标颜色模型（H 通道直方图）
    Mat roi = frame(track_window);
    Mat hsv_roi, mask;
    cvtColor(roi, hsv_roi, COLOR_BGR2HSV);
    // 过滤低饱和/低亮度像素，减少背景干扰
    inRange(hsv_roi, Scalar(0, 60, 32), Scalar(180, 255, 255), mask);

    float range_[] = {0, 180};
    const float *range[] = {range_};
    Mat roi_hist;
    int histSize[] = {180};
    int channels[] = {0};   // 只用 H 通道
    calcHist(&hsv_roi, 1, channels, mask, roi_hist, 1, histSize, range);
    normalize(roi_hist, roi_hist, 0, 255, NORM_MINMAX);

    // 2. 每帧：反投影 + meanShift 迭代
    TermCriteria term_crit(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);
    while (true) {
        capture >> frame;
        if (frame.empty()) break;

        Mat hsv, dst;
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        calcBackProject(&hsv, 1, channels, roi_hist, dst, range);  // 概率图

        // 均值漂移：向概率图质心迭代移动窗口（窗口大小固定）
        meanShift(dst, track_window, term_crit);

        rectangle(frame, track_window, Scalar(0, 255, 0), 2);
        imshow("meanshift", frame);
        if (waitKey(30) == 27) break;
    }
    return 0;
}
