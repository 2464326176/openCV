// LEARN: L2 Sobel 杈圭紭
// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/Sobel_Demo.cpp
// THEORY: docs/ch02_imgproc.md 搂4
// TASK: Sobel x/y 姹傛搴︼紝convertScaleAbs+addWeighted 鍚堟垚
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int kSize = 3;
static int scale = 1;
static int delta = 0;

static void onTrack(int, void*) {
    int k = kSize | 1; if (k < 3) k = 3;
    Mat gx, gy;
    Sobel(gray, gx, CV_16S, 1, 0, k, scale, delta, BORDER_DEFAULT);
    Sobel(gray, gy, CV_16S, 0, 1, k, scale, delta, BORDER_DEFAULT);
    Mat absX, absY, grad;
    convertScaleAbs(gx, absX);
    convertScaleAbs(gy, absY);
    addWeighted(absX, 0.5, absY, 0.5, 0, grad);
    Mat up;
    hconcat(absX, absY, up);
    hconcat(up, grad, up);
    imshow("L2_10 sobel X|Y|mag", up);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_10 sobel X|Y|mag", WINDOW_AUTOSIZE);
    createTrackbar("ksize", "L2_10 sobel X|Y|mag", &kSize, 7, onTrack);
    createTrackbar("scale", "L2_10 sobel X|Y|mag", &scale, 10, onTrack);
    createTrackbar("delta", "L2_10 sobel X|Y|mag", &delta, 100, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
