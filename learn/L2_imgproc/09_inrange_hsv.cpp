// LEARN: L2 HSV inRange color segmentation
// OFFICIAL: samples/cpp/tutorial_code/ImgProc/Threshold_inRange.cpp
// THEORY: docs/ch02_imgproc.md §3
// TASK: cvtColor(BGR2HSV) then inRange to extract a color range
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, hsv;
static int hLow = 35, hHigh = 85;   // green range
static int sLow = 50,  sHigh = 255;
static int vLow = 50,  vHigh = 255;

static void onTrack(int, void*) {
    Scalar lo(hLow, sLow, vLow);
    Scalar hi(hHigh, sHigh, vHigh);
    Mat mask;
    inRange(hsv, lo, hi, mask);
    Mat color;
    src.copyTo(color, mask);
    Mat up;
    hconcat(src, color, up);
    hconcat(up, mask, up);
    imshow("L2_09 inRange HSV", up);
}

int main() {
    src = imread(getImagePath("VCG2.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, hsv, COLOR_BGR2HSV);
    namedWindow("L2_09 inRange HSV", WINDOW_AUTOSIZE);
    createTrackbar("H low",  "L2_09 inRange HSV", &hLow,  180, onTrack);
    createTrackbar("H high", "L2_09 inRange HSV", &hHigh, 180, onTrack);
    createTrackbar("S low",  "L2_09 inRange HSV", &sLow,  255, onTrack);
    createTrackbar("S high", "L2_09 inRange HSV", &sHigh, 255, onTrack);
    createTrackbar("V low",  "L2_09 inRange HSV", &vLow,  255, onTrack);
    createTrackbar("V high", "L2_09 inRange HSV", &vHigh, 255, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
