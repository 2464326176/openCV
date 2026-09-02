// LEARN: L4 鏃犵紳鍏嬮殕 seamlessClone
// OFFICIAL: tutorial_code/photo/seamless_cloning/cloning_demo.cpp, cloning_gui.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.25
// TASK: 鎶?VCG1 鐨勪竴涓煩褰?ROI 浣滀负鍓嶆櫙锛屾棤缂濆厠闅嗗埌 VCG2 涓ぎ锛汵ORMAL_CLONE 涓?MIXED_CLONE 瀵规瘮
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("VCG1.jpg"));
    Mat dst = imread(getImagePath("VCG2.jpg"));
    if (src.empty() || dst.empty()) { logInfo("imread failed"); return -1; }
    dbgMatInfo("src", src); dbgMatInfo("dst", dst);

    // 鍙?src 涓績 200x200 ROI
    Rect roi(src.cols / 2 - 100, src.rows / 2 - 100, 200, 200);
    roi &= Rect(0, 0, src.cols, src.rows);
    Mat srcROI = src(roi).clone();
    Mat mask(roi.size(), CV_8UC1, Scalar(255));    // 鍏ㄥ墠鏅?mask

    // 鐩爣涓績浣滀负鍏嬮殕钀界偣
    Point center(dst.cols / 2, dst.rows / 2);

    Mat normal, mixed;
    seamlessClone(srcROI, dst, mask, center, normal, NORMAL_CLONE);
    seamlessClone(srcROI, dst, mask, center, mixed, MIXED_CLONE);
    logInfo("NORMAL_CLONE + MIXED_CLONE done, center=(%d,%d)", center.x, center.y);

    dbgShowMany({"L4_08 dst", "L4_08 normal", "L4_08 mixed"},
                {dst, normal, mixed}, 0);
    return 0;
}
