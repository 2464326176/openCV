// LEARN: L0 Trackbar for brightness and contrast adjustment
// OFFICIAL: samples/cpp/tutorial_code/HighGUI/BasicLinearTransformsTrackbar.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md §highgui + ch01_core.md §1
// TASK: createTrackbar to adjust alpha/beta, convertScaleAbs for real-time display
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int alpha = 100; // actual value multiplied by 0.01
static int beta  = 0;

static void onTrack(int, void*) {
    Mat dst;
    convertScaleAbs(src, dst, alpha / 100.0, beta);
    imshow("L0_02 trackbar", dst);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    namedWindow("L0_02 trackbar", WINDOW_AUTOSIZE);
    createTrackbar("Alpha x0.01", "L0_02 trackbar", &alpha, 300, onTrack);
    createTrackbar("Beta",         "L0_02 trackbar", &beta, 100, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}