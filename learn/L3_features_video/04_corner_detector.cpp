// LEARN: L3 瑙掔偣妫€娴嬬患鍚堝姣?Harris vs Shi-Tomasi
// OFFICIAL: samples/cpp/tutorial_code/TrackingMotion/cornerDetector_Demo.cpp
// THEORY: docs/ch03_features.md 搂瑙掔偣
// TASK: cornerHarris 涓?cornerMinEigenVal 涓ゅ紶鍝嶅簲鍥惧苟鎺掞紝褰掍竴鍖栨樉绀?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat harris, shi;
    int blockSize = 2, ksize = 3;
    cornerHarris(src, harris, blockSize, ksize, 0.04);
    cornerMinEigenVal(src, shi, blockSize, ksize);

    Mat hN, sN, hC, sC;
    normalize(harris, hN, 0, 255, NORM_MINMAX, CV_8U);
    normalize(shi,    sN, 0, 255, NORM_MINMAX, CV_8U);
    cvtColor(hN, hC, COLOR_GRAY2BGR);
    cvtColor(sN, sC, COLOR_GRAY2BGR);
    logInfo("harris+shi-tomasi done");
    dbgShowMany({"L3_04 harris", "L3_04 shi-tomasi"}, {hC, sC}, 0);
    return 0;
}
