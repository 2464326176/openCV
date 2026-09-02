// LEARN: L2 warpAffine affine transform
// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/Geometric_Transforms_Demo.cpp
// THEORY: docs/ch02_imgproc.md §6
// TASK: getRotationMatrix2D + warpAffine rotate/scale
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int angle = 30;
static int scaleP = 100;   // *0.01

static void onTrack(int, void*) {
    Point2f center(src.cols / 2.0f, src.rows / 2.0f);
    Mat M = getRotationMatrix2D(center, angle, scaleP / 100.0);
    Mat dst;
    warpAffine(src, dst, M, src.size(), INTER_LINEAR, BORDER_REFLECT);
    imshow("L2_17 warpAffine", dst);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_17 warpAffine", WINDOW_AUTOSIZE);
    createTrackbar("angle", "L2_17 warpAffine", &angle, 360, onTrack);
    createTrackbar("scale x0.01", "L2_17 warpAffine", &scaleP, 200, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
