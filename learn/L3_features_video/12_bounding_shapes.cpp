// LEARN: L3 Minimum Enclosing Shapes
// OFFICIAL: samples/cpp/minarea.cpp、fitellipse.cpp
// THEORY: docs/ch03_features.md §轮廓
// TASK: for each contour draw boundingRect/minAreaRect/fitEllipse three enclosing shapes
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    Mat gray; cvtColor(src, gray, COLOR_BGR2GRAY);
    Mat edge; Canny(gray, edge, 100, 200);

    std::vector<std::vector<Point>> conts;
    findContours(edge, conts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    Mat show = src.clone();
    for (size_t i = 0; i < conts.size(); ++i) {
        if (contourArea(conts[i]) < 100) continue;
        Rect b = boundingRect(conts[i]);
        rectangle(show, b, Scalar(255, 0, 0), 2);                 // blue: upright rectangle
        RotatedRect mr = minAreaRect(conts[i]);
        Point2f v[4]; mr.points(v);
        for (int j = 0; j < 4; ++j) line(show, v[j], v[(j + 1) % 4], Scalar(0, 255, 0), 2); // green: minimum area rectangle
        if (conts[i].size() >= 5) {
            RotatedRect el = fitEllipse(conts[i]);
            ellipse(show, el, Scalar(0, 0, 255), 2);               // red: ellipse
        }
    }
    logInfo("contours=%zu (blue=rect green=minArea red=ellipse)", conts.size());
    dbgShow("L3_12 bounding shapes", show, 0);
    return 0;
}

