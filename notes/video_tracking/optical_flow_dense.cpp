//********************
// Author:  yh
// 稠密光流（Farneback）：对每个像素求运动矢量，并用 HSV 编码可视化
//  HSV 编码规则：H=光流方向角度，S=饱和度固定为1，V=速度大小（归一化）
//  对应官方示例: tutorial_code/video/optical_flow/optical_flow_dense.cpp
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

    Mat frame1, prvs;
    capture >> frame1;
    if (frame1.empty()) return -1;
    cvtColor(frame1, prvs, COLOR_BGR2GRAY);

    while (true) {
        Mat frame2, next;
        capture >> frame2;
        if (frame2.empty()) break;
        cvtColor(frame2, next, COLOR_BGR2GRAY);

        // Farneback 稠密光流：输出 CV_32FC2（每像素 (dx, dy)）
        Mat flow(prvs.size(), CV_32FC2);
        calcOpticalFlowFarneback(prvs, next, flow,
                                 0.5,     // pyr_scale：金字塔缩放
                                 3,       // levels：金字塔层数
                                 15,      // winsize：平均窗口
                                 3,       // iterations：每层迭代次数
                                 5,       // poly_n：多项式邻域
                                 1.2,     // poly_sigma：高斯标准差
                                 0);      // flags

        // 拆分 dx、dy → 极坐标（magnitude 速度, angle 方向）
        Mat flow_parts[2];
        split(flow, flow_parts);
        Mat magnitude, angle, magn_norm;
        cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);

        // 速度归一化到 [0,1]，角度缩放到 H 通道范围
        normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
        angle *= ((1.f / 360.f) * (180.f / 255.f));

        // 组 HSV 并转 BGR 显示：H=方向、S=1、V=速度
        Mat _hsv[3], hsv, hsv8, bgr;
        _hsv[0] = angle;
        _hsv[1] = Mat::ones(angle.size(), CV_32F);
        _hsv[2] = magn_norm;
        merge(_hsv, 3, hsv);
        hsv.convertTo(hsv8, CV_8U, 255.0);
        cvtColor(hsv8, bgr, COLOR_HSV2BGR);

        imshow("dense optical flow", bgr);
        if (waitKey(30) == 27) break;
        prvs = next;
    }
    return 0;
}
