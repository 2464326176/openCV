// LEARN: L2 鏋佸潗鏍囧彉鎹?// OFFICIAL: samples/cpp/polar_transforms.cpp
// THEORY: docs/ch02_imgproc.md 搂6
// TASK: warpPolar 涓?linearPolar 瀵规瘮
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int radius = 100;

static void onTrack(int, void*) {
    Point2f center(src.cols / 2.0f, src.rows / 2.0f);
    Mat polar, linear;
    warpPolar(src, polar, Size(radius, 360), center, radius,
              INTER_LINEAR | WARP_POLAR_LOG);
    warpPolar(src, linear, Size(radius, 360), center, radius,
              INTER_LINEAR | WARP_POLAR_LINEAR);
    Mat up;
    hconcat(polar, linear, up);
    imshow("L2_21 polar log|linear", up);
}

int main() {
    src = imread(getImagePath("OIP.png"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_21 polar log|linear", WINDOW_AUTOSIZE);
    int maxR = (std::min)(src.cols, src.rows) / 2;
    createTrackbar("radius", "L2_21 polar log|linear", &radius, maxR, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
