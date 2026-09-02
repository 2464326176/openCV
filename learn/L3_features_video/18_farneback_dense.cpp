// LEARN: L3 Farneback 稠密光流
// OFFICIAL: samples/cpp/tutorial_code/video/optical_flow/optical_flow_dense.cpp、fback.cpp
// THEORY: docs/ch04_video.md §光流
// TASK: calcOpticalFlowFarneback compute VCG1→VCG2 displacement field, HSV encoded display
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat f1 = imread(getImagePath("VCG1.jpg"));
    Mat f2 = imread(getImagePath("VCG2.jpg"));
    if (f1.empty() || f2.empty()) { logInfo("imread failed"); return -1; }
    resize(f1, f1, Size(384, 256)); resize(f2, f2, Size(384, 256));
    Mat g1, g2; cvtColor(f1, g1, COLOR_BGR2GRAY); cvtColor(f2, g2, COLOR_BGR2GRAY);

    Mat flow;
    calcOpticalFlowFarneback(g1, g2, flow, 0.5, 3, 15, 3, 5, 1.1, 0);

    Mat xy[2]; split(flow, xy);
    Mat mag, ang, magN;
    cartToPolar(xy[0], xy[1], mag, ang, true);
    normalize(mag, magN, 0.0, 1.0, NORM_MINMAX, CV_32F);
    ang *= (180.0 / (2.0 * CV_PI));
    Mat hsvCh[3] = { ang, Mat::ones(ang.size(), CV_32F), magN };
    Mat hsvF, hsv8, bgr;
    merge(hsvCh, 3, hsvF);
    hsvF.convertTo(hsv8, CV_8U, 255.0);
    cvtColor(hsv8, bgr, COLOR_HSV2BGR);

    logInfo("farneback flow ok");
    dbgShowMany({"L3_18 frame1", "L3_18 flow hsv"}, {f1, bgr}, 0);
    return 0;
}

