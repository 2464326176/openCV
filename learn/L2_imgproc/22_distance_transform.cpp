// LEARN: L2 Distance transform
// OFFICIAL: samples/cpp/distrans.cpp
// THEORY: docs/ch02_imgproc.md §7
// TASK: distanceTransform + connectedComponents separate foreground
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray, bin;
static int th = 100;

static void onTrack(int, void*) {
    threshold(gray, bin, th, 255, THRESH_BINARY_INV + THRESH_OTSU);
    Mat dist;
    distanceTransform(bin, dist, DIST_L2, 3);
    Mat distU8;
    normalize(dist, distU8, 0, 255, NORM_MINMAX, CV_8U);
    Mat localMax;
    threshold(dist, localMax, 0.4, 255, THRESH_BINARY);
    localMax.convertTo(localMax, CV_8U);
    Mat labels, stats, cent;
    int n = connectedComponentsWithStats(localMax, labels, stats, cent, 8);
    logInfo("fg-components=%d", n - 1);
    Mat marks = Mat::zeros(bin.size(), CV_32S);
    for (int i = 1; i < n; ++i) {
        marks.setTo(Scalar(i), labels == i);
    }
    marks.setTo(Scalar(n), ~bin);
    Mat marksU8;
    marks.convertTo(marksU8, CV_8U, 255.0 / n);
    Mat up;
    hconcat(bin, distU8, up);
    hconcat(up, marksU8, up);
    imshow("L2_22 dist bin|dist|marks", up);
}

int main() {
    src = imread(getImagePath("VCG2.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_22 dist bin|dist|marks", WINDOW_AUTOSIZE);
    createTrackbar("th", "L2_22 dist bin|dist|marks", &th, 255, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
