// LEARN: L4 ArUco marker generation
// OFFICIAL: tutorial_code/objectDetection/create_marker.cpp, create_board.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.11
// TASK: generate 4 DICT_4X4_50 markers arranged in a grid using new API aruco::getPredefinedDictionary + aruco::generateImageMarker
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv_utils.h>

using namespace cv;
using namespace cv::aruco;

int main() {
    Dictionary dict = getPredefinedDictionary(DICT_4X4_50);
    logInfo("dictionary loaded: DICT_4X4_50");

    const int side = 200;          // pixel side length of a single marker
    const int pad  = 20;           // grid padding
    const int grid = 420;          // 2x2 grid canvas side length
    Mat canvas(grid, grid, CV_8UC1, Scalar(255));

    int ids[] = {0, 1, 2, 3};
    Point tl[4] = {Point(pad, pad), Point(grid - pad - side, pad),
                   Point(pad, grid - pad - side), Point(grid - pad - side, grid - pad - side)};
    for (int k = 0; k < 4; ++k) {
        Mat marker;
        generateImageMarker(dict, ids[k], side, marker);
        Mat roi = canvas(Rect(tl[k], Size(side, side)));
        marker.copyTo(roi);
        logInfo("marker id=%d at (%d,%d)", ids[k], tl[k].x, tl[k].y);
    }
    imwrite("l4_04_aruco_grid.png", canvas);
    dbgMatInfo("canvas", canvas);
    dbgShow("L4_04 aruco create", canvas);
    return 0;
}
