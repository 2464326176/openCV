// LEARN: L4 seamless cloning
// OFFICIAL: tutorial_code/photo/seamless_cloning/cloning_demo.cpp, cloning_gui.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.25
// TASK: use a rectangular ROI from VCG1 as foreground, seamless clone to center of VCG2; compare NORMAL_CLONE vs MIXED_CLONE
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("VCG1.jpg"));
    Mat dst = imread(getImagePath("VCG2.jpg"));
    if (src.empty() || dst.empty()) { logInfo("imread failed"); return -1; }
    dbgMatInfo("src", src); dbgMatInfo("dst", dst);

    // take 200x200 ROI from center of src
    Rect roi(src.cols / 2 - 100, src.rows / 2 - 100, 200, 200);
    roi &= Rect(0, 0, src.cols, src.rows);
    Mat srcROI = src(roi).clone();
    Mat mask(roi.size(), CV_8UC1, Scalar(255));    // full foreground mask

    // target center as clone destination
    Point center(dst.cols / 2, dst.rows / 2);

    Mat normal, mixed;
    seamlessClone(srcROI, dst, mask, center, normal, NORMAL_CLONE);
    seamlessClone(srcROI, dst, mask, center, mixed, MIXED_CLONE);
    logInfo("NORMAL_CLONE + MIXED_CLONE done, center=(%d,%d)", center.x, center.y);

    dbgShowMany({"L4_08 dst", "L4_08 normal", "L4_08 mixed"},
                {dst, normal, mixed}, 0);
    return 0;
}
