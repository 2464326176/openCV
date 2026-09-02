// LEARN: L4 grabCut foreground segmentation
// OFFICIAL: samples/cpp/grabcut.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.23
// TASK: rectangle initialization grabCut: initial face rectangle on lena, 5 iterations, output foreground mask and cutout
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) { logInfo("imread failed: lena.jpg"); return -1; }
    dbgMatInfo("img", img);

    Rect roi(60, 90, 380, 380);          // rectangle covering the face
    Mat bgd, fgd;                         // used internally by grabCut, no initialization needed
    Mat mask(img.size(), CV_8UC1, Scalar(GC_BGD));
    rectangle(mask, roi, Scalar(GC_PR_FGD), -1);

    grabCut(img, mask, roi, bgd, fgd, 5, GC_INIT_WITH_RECT);
    logInfo("grabCut iter 5 done");

    // take GC_FGD | GC_PR_FGD as foreground
    Mat fgMask = (mask == GC_FGD) | (mask == GC_PR_FGD);
    Mat cut;
    img.copyTo(cut, fgMask);

    Mat annot = img.clone();
    rectangle(annot, roi, Scalar(0, 255, 0), 2);
    dbgShowMany({"L4_07 roi", "L4_07 mask", "L4_07 cut"},
                {annot, fgMask, cut}, 0);
    return 0;
}
