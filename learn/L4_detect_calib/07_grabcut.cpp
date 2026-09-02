// LEARN: L4 grabCut 鍓嶆櫙鍒嗗壊
// OFFICIAL: samples/cpp/grabcut.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.23
// TASK: 鐭╁舰鍒濆鍖?grabCut锛氬厛妗?lena 鑴搁儴鐭╁舰锛岃凯浠?5 娆★紝杈撳嚭鍓嶆櫙 mask 涓庢姞鍥剧粨鏋?#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) { logInfo("imread failed: lena.jpg"); return -1; }
    dbgMatInfo("img", img);

    Rect roi(60, 90, 380, 380);          // 鍖呬綇鑴搁儴鐨勭煩褰?    Mat bgd, fgd;                         // grabCut 鍐呴儴鐢紝鏃犻渶鍒濆鍖?    Mat mask(img.size(), CV_8UC1, Scalar(GC_BGD));
    rectangle(mask, roi, Scalar(GC_PR_FGD), -1);

    grabCut(img, mask, roi, bgd, fgd, 5, GC_INIT_WITH_RECT);
    logInfo("grabCut iter 5 done");

    // 鍙?GC_FGD | GC_PR_FGD 浣滀负鍓嶆櫙
    Mat fgMask = (mask == GC_FGD) | (mask == GC_PR_FGD);
    Mat cut;
    img.copyTo(cut, fgMask);

    Mat annot = img.clone();
    rectangle(annot, roi, Scalar(0, 255, 0), 2);
    dbgShowMany({"L4_07 roi", "L4_07 mask", "L4_07 cut"},
                {annot, fgMask, cut}, 0);
    return 0;
}
