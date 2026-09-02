// LEARN: L4 decolor
// OFFICIAL: tutorial_code/photo/decolorization/decolor.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.27
// TASK: decolor takes color input, outputs grayscale and contrast-enhanced images, compare with standard cvtColor grayscale
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("VCG1.jpg"));
    if (img.empty()) { logInfo("imread failed: VCG1.jpg"); return -1; }
    dbgMatInfo("img", img);

    Mat gray, boost;
    decolor(img, gray, boost);
    logInfo("decolor: gray + boost ok");

    Mat stdGray;
    cvtColor(img, stdGray, COLOR_BGR2GRAY);

    dbgShowMany({"L4_09 input", "L4_09 decolor-gray", "L4_09 decolor-boost", "L4_09 cvtColor-gray"},
                {img, gray, boost, stdGray}, 0);
    return 0;
}
