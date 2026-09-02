// LEARN: L3 Corner Detection Comprehensive Comparison
// OFFICIAL: samples/cpp/tutorial_code/TrackingMotion/cornerDetector_Demo.cpp
// THEORY: docs/ch03_features.md §角点
// TASK: cornerHarris compute response map; cornerMinEigenVal two response maps side-by-side, normalized display
#include <opencv2/opencv.hpp>
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

