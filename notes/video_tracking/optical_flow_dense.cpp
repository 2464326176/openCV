//********************
// Author:  yh
// Dense optical flow (Farneback): estimate a motion vector for every pixel,
//   visualized with HSV encoding
//  HSV encoding rule: H = flow direction angle, S = fixed saturation 1,
//                     V = speed magnitude (normalized)
//  Official example: tutorial_code/video/optical_flow/optical_flow_dense.cpp
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

        // Farneback dense optical flow: outputs CV_32FC2 (per-pixel (dx, dy))
        Mat flow(prvs.size(), CV_32FC2);
        calcOpticalFlowFarneback(prvs, next, flow,
                                 0.5,     // pyr_scale: pyramid scaling
                                 3,       // levels: number of pyramid layers
                                 15,      // winsize: averaging window size
                                 3,       // iterations: iterations per layer
                                 5,       // poly_n: polynomial neighborhood size
                                 1.2,     // poly_sigma: Gaussian standard deviation
                                 0);      // flags

        // Split dx, dy -> polar (magnitude = speed, angle = direction)
        Mat flow_parts[2];
        split(flow, flow_parts);
        Mat magnitude, angle, magn_norm;
        cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);

        // Normalize speed to [0,1]; scale angle to the H channel range
        normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
        angle *= ((1.f / 360.f) * (180.f / 255.f));

        // Build HSV and convert to BGR for display: H=direction, S=1, V=speed
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
