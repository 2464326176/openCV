// LEARN: L0 婊戝姩鏉℃敼浜害瀵规瘮搴?// OFFICIAL: samples/cpp/tutorial_code/HighGUI/BasicLinearTransformsTrackbar.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md 搂highgui + ch01_core.md 搂1
// TASK: createTrackbar 璋?alpha/beta锛宑onvertScaleAbs 瀹炴椂鏄剧ず
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int alpha = 100; // 瀹為檯涔?0.01
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
