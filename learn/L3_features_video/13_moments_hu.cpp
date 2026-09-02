// LEARN: L3 Moments and Hu Invariants
// OFFICIAL: samples/cpp/tutorial_code/ShapeDescriptors/moments_demo.cpp
// THEORY: docs/ch03_features.md §contours
// TASK: moments compute centroid, HuMoments output 7 invariants (log normalized)
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    resize(src, src, Size(256, 256));
    Mat edge; Canny(src, edge, 100, 200);
    std::vector<std::vector<Point>> conts;
    findContours(edge, conts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (conts.empty()) { logInfo("no contour"); return -1; }

    size_t k = 0; double a = 0;
    for (size_t i = 0; i < conts.size(); ++i) {
        double ai = contourArea(conts[i]);
        if (ai > a) { a = ai; k = i; }
    }
    Moments mu = moments(conts[k], false);
    float m00 = (float)mu.m00;
    Point2f centroid(m00 != 0 ? mu.m10 / m00 : 0,
                     m00 != 0 ? mu.m01 / m00 : 0);
    Mat hu; HuMoments(mu, hu);

    Mat show; cvtColor(src, show, COLOR_GRAY2BGR);
    circle(show, centroid, 5, Scalar(0, 0, 255), -1);
    drawContours(show, conts, (int)k, Scalar(0, 255, 0), 2);
    for (int i = 0; i < 7; ++i) {
        double v = hu.at<double>(i);
        double lv = std::abs(v) > 1e-12 ? std::log10(std::abs(v)) : 0.0;
        logInfo("hu[%d]=%.4e log10|.|=%.4f", i, v, lv);
    }
    dbgShow("L3_13 moments+hu", show, 0);
    return 0;
}

