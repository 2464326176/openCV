// LEARN: L2 Threshold — five types
// OFFICIAL: samples/cpp/tutorial_code/ImgProc/Threshold.cpp
// THEORY: docs/ch02_imgproc.md §3
// TASK: threshold five types (BINARY/TRUNC/TOZERO/INV+OTSU) trackbar
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int th = 100;
static int mode = 0;  // 0..4

static void onTrack(int, void*) {
    Mat dst;
    int type[] = { THRESH_BINARY, THRESH_BINARY_INV, THRESH_TRUNC,
                   THRESH_TOZERO, THRESH_TOZERO_INV };
    threshold(gray, dst, th, 255, type[mode]);
    Mat up;
    hconcat(gray, dst, up);
    imshow("L2_08 threshold", up);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_08 threshold", WINDOW_AUTOSIZE);
    createTrackbar("th", "L2_08 threshold", &th, 255, onTrack);
    createTrackbar("0bin 1binv 2trunc 3tozero 4tzinv",
                   "L2_08 threshold", &mode, 4, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
