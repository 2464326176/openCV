// LEARN: L4 image inpainting
// OFFICIAL: samples/cpp/inpaint.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.22
// TASK: draw a white horizontal line across the middle of lena as the inpainting region, compare INPAINT_TELEA and INPAINT_NS
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) { logInfo("imread failed: lena.jpg"); return -1; }
    dbgMatInfo("img", img);

    // 1) build damage mask: horizontal white line in the middle + square at top-right
    Mat mask(img.size(), CV_8UC1, Scalar(0));
    rectangle(mask, Rect(0, img.rows / 2 - 4, img.cols, 8), Scalar(255), -1);
    rectangle(mask, Rect(img.cols - 80, 10, 70, 70), Scalar(255), -1);

    Mat damaged;
    img.copyTo(damaged, mask.inv());     // mask area becomes black
    // draw white noise at mask area (simulate damage) for visual comparison
    damaged.setTo(Scalar(255, 255, 255), mask);

    // 2) compare two inpainting algorithms
    Mat telea, ns;
    inpaint(damaged, mask, telea, 5, INPAINT_TELEA);
    inpaint(damaged, mask, ns, 5, INPAINT_NS);
    logInfo("INPAINT_TELEA done; INPAINT_NS done");

    dbgShowMany({"L4_06 damaged", "L4_06 telea", "L4_06 ns"},
                {damaged, telea, ns}, 0);
    return 0;
}
