// LEARN: L1 create mask
// OFFICIAL: samples/cpp/create_mask.cpp
// THEORY: docs/ch01_core.md §2.15 create_mask
// TASK: Mat::setTo + bitwise_and/or/not for circular mask extraction and background composition
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    // 1) Create circular mask: setTo zeros the whole image, circle fills 255 inside
    Mat mask = Mat::zeros(src.size(), CV_8UC1);
    circle(mask, Point(src.cols / 2, src.rows / 2),
           std::min(src.cols, src.rows) / 3, Scalar(255), -1);

    // 2) bitwise_and: keep only pixels inside the circle
    Mat masked;
    bitwise_and(src, src, masked, mask);

    // 3) Inverse mask + setTo: fill outside area with green background
    Mat invMask;
    bitwise_not(mask, invMask);
    Mat bg = Mat::zeros(src.size(), src.type());
    bg.setTo(Scalar(0, 255, 0));               // fill background color with setTo
    Mat bgOut;
    bitwise_and(bg, bg, bgOut, invMask);      // outside circle: green, inside: 0

    // 4) bitwise_or to compose the final image
    Mat composed;
    bitwise_or(masked, bgOut, composed);

    dbgPrint("mask size", mask.size());
    dbgShowMany({"src", "mask", "masked", "bg", "composed"},
                {src, mask, masked, bgOut, composed}, 0);
    return 0;
}
