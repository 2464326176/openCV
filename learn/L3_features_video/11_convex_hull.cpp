// LEARN: L3 Convex Hull and Concavity
// OFFICIAL: samples/cpp/tutorial_code/ShapeDescriptors/hull_demo.cpp、convexhull.cpp
// THEORY: docs/ch03_features.md §轮廓
// TASK: findContours get largest contour, convexHull draw convex hull, convexityDefects mark concavity points
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    Mat edge; Canny(src, edge, 100, 200);
    std::vector<std::vector<Point>> conts;
    findContours(edge, conts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (conts.empty()) { logInfo("no contour"); return -1; }

    // get largest contour
    size_t k = 0; double a = 0;
    for (size_t i = 0; i < conts.size(); ++i) {
        double ai = contourArea(conts[i]);
        if (ai > a) { a = ai; k = i; }
    }
    std::vector<Point> hull;
    convexHull(conts[k], hull);

    std::vector<int> hullIdx;
    convexHull(conts[k], hullIdx, false, false);
    std::vector<Vec4i> defects;
    if (hullIdx.size() > 3 && conts[k].size() > 3) {
        convexityDefects(conts[k], hullIdx, defects);
    }

    Mat show; cvtColor(src, show, COLOR_GRAY2BGR);
    drawContours(show, conts, (int)k, Scalar(0, 255, 0), 2);
    polylines(show, hull, true, Scalar(0, 0, 255), 2);
    for (auto& d : defects) {
        Point far = conts[k][d[2]];
        circle(show, far, 3, Scalar(255, 0, 0), -1);
    }
    logInfo("hull=%zu defects=%zu", hull.size(), defects.size());
    dbgShow("L3_11 convex hull", show, 0);
    return 0;
}

