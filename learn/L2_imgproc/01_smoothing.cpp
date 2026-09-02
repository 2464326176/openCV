// LEARN: L2 Smoothing — five-in-one comparison
// OFFICIAL: samples/cpp/tutorial_code/ImgProc/Smoothing/Smoothing.cpp
// THEORY: docs/ch02_imgproc.md §1
// TASK: blur/GaussianBlur/medianBlur/bilateralFilter hconcat comparison in one window
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int kSize = 5;

static void onTrack(int, void*) {
    int k = kSize | 1;              // ensure odd
    if (k < 3) k = 3;
    Mat b, g, m, bi;
    blur(src, b, Size(k, k));
    GaussianBlur(src, g, Size(k, k), 0);
    medianBlur(src, m, k);
    bilateralFilter(src, bi, k, k * 2, k / 2);

    Mat r1, r2;
    hconcat(src, b, r1);   hconcat(g, m, r2);
    Mat grid;
    vconcat(r1, r2, grid);
    Mat row2;
    hconcat(bi, Mat::zeros(src.size(), src.type()), row2);
    Mat all;
    vconcat(grid, row2, all);
    dbgMatInfo("smoothing", all);
    imshow("L2_01 smoothing", all);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_01 smoothing", WINDOW_AUTOSIZE);
    createTrackbar("ksize", "L2_01 smoothing", &kSize, 31, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
