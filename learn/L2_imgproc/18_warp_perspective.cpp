// LEARN: L2 warpPerspective perspective transform
// OFFICIAL: samples/cpp/warpPerspective_demo.cpp
// THEORY: docs/ch02_imgproc.md §6
// TASK: getPerspectiveTransform four points → warpPerspective rectify
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("1.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Point2f srcPts[4] = {
        Point2f(0,   0),
        Point2f((float)(src.cols - 1), 0),
        Point2f((float)(src.cols - 1), (float)(src.rows - 1)),
        Point2f(0,   (float)(src.rows - 1))
    };
    Size dstSize(src.cols, src.rows);
    Point2f dstPts[4] = {
        Point2f(0, 0),
        Point2f((float)(dstSize.width - 1), 0),
        Point2f((float)(dstSize.width - 1) * 0.85f, (float)(dstSize.height - 1)),
        Point2f((float)(dstSize.width - 1) * 0.15f, (float)(dstSize.height - 1))
    };
    Mat M = getPerspectiveTransform(srcPts, dstPts);
    Mat dst;
    warpPerspective(src, dst, M, dstSize);
    Mat up;
    hconcat(src, dst, up);
    dbgShow("L2_18 warpPerspective", up, 0);
    return 0;
}
