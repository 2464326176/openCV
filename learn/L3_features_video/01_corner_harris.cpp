// LEARN: L3 Harris Corner
// OFFICIAL: samples/cpp/tutorial_code/TrackingMotion/cornerHarris_Demo.cpp
// THEORY: docs/ch03_features.md §corners
// TASK: cornerHarris compute response map, normalize display; trackbar to change blockSize/kappa
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, dst;
static int blockSize = 2;
static int kappa_x100 = 4;  // actual *0.01

static void onTrack(int, void*) {
    int bs = std::max(blockSize, 1);
    cornerHarris(src, dst, bs, 3, kappa_x100 / 100.0);
    Mat normDst;
    normalize(dst, normDst, 0, 255, NORM_MINMAX, CV_8U);
    imshow("L3_01 harris", normDst);
}

int main() {
    src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    dst.create(src.size(), CV_32FC1);
    namedWindow("L3_01 harris", WINDOW_AUTOSIZE);
    createTrackbar("Block",      "L3_01 harris", &blockSize,   5,  onTrack);
    createTrackbar("Kappa*0.01", "L3_01 harris", &kappa_x100, 20, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}

