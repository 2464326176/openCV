// LEARN: L3 Subpixel Corner
// OFFICIAL: samples/cpp/tutorial_code/TrackingMotion/cornerSubPix_Demo.cpp
// THEORY: docs/ch03_features.md §corners
// TASK: goodFeaturesToTrack get corners, cornerSubPix refine, compare positions before/after
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }

    std::vector<Point2f> corners;
    goodFeaturesToTrack(src, corners, 50, 0.01, 10);
    std::vector<Point2f> refined = corners;

    Size win(5, 5), zeroZone(-1, -1);
    TermCriteria tc(TermCriteria::COUNT | TermCriteria::EPS, 40, 0.001);
    cornerSubPix(src, refined, win, zeroZone, tc);

    Mat show; cvtColor(src, show, COLOR_GRAY2BGR);
    for (size_t i = 0; i < corners.size(); ++i) {
        circle(show, corners[i], 4, Scalar(0, 0, 255), 1);     // red: raw
        circle(show, refined[i], 2, Scalar(0, 255, 0), -1);    // green: refined
    }
    logInfo("refined %zu corners", refined.size());
    dbgShow("L3_03 subpix (red=raw, green=refined)", show, 0);
    return 0;
}

