// LEARN: L3 Comprehensive Contour Metrics
// OFFICIAL: samples/cpp/tutorial_code/ShapeDescriptors/generalContours_demo1.cpp、generalContours_demo2.cpp
// THEORY: docs/ch03_features.md §轮廓
// TASK: for each contour output area/perimeter/boundingRect/circularity, and annotate on image
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
    int n = 0;
    for (size_t i = 0; i < conts.size(); ++i) {
        double area = contourArea(conts[i]);
        if (area < 100) continue;
        double per = arcLength(conts[i], true);
        double circ = per > 0 ? 4 * CV_PI * area / (per * per) : 0;
        Rect b = boundingRect(conts[i]);
        rectangle(show, b, Scalar(0, 255, 0), 1);
        Point tl = b.tl();
        putText(show, format("A=%.0f C=%.2f", area, circ),
                Point(tl.x, std::max(tl.y - 3, 12)),
                FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 255, 255), 1);
        logInfo("contour[%zu] area=%.1f per=%.1f circ=%.3f", i, area, per, circ);
        if (++n > 8) break;
    }
    dbgShow("L3_15 general contours", show, 0);
    return 0;
}

