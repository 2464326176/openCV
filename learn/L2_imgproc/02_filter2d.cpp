// LEARN: L2 filter2D custom convolution kernel
// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/filter2D_demo.cpp
// THEORY: docs/ch02_imgproc.md §1
// TASK: filter2D with sharpen/blur/emboss kernels; flip kernel orientation
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int mode = 0;   // 0 blur 1 sharpen 2 emboss 3 sobel-x

static void onTrack(int, void*) {
    Mat kernel;
    if (mode == 0) {
        kernel = Mat::ones(3, 3, CV_32F) / 9.0f;
    } else if (mode == 1) {
        kernel = (Mat_<float>(3,3) <<  0, -1,  0,
                                       -1,  5, -1,
                                        0, -1,  0);
    } else if (mode == 2) {
        kernel = (Mat_<float>(3,3) << -2, -1,  0,
                                       -1,  1,  1,
                                        0,  1,  2);
    } else {
        kernel = (Mat_<float>(3,3) << -1, 0, 1,
                                       -2, 0, 2,
                                       -1, 0, 1);
    }
    Mat dst;
    filter2D(src, dst, src.depth(), kernel);
    imshow("L2_02 filter2d", dst);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_02 filter2d", WINDOW_AUTOSIZE);
    createTrackbar("0blur 1sharpen 2emboss 3sobelX",
                   "L2_02 filter2d", &mode, 3, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
