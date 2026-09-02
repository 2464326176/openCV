// LEARN: L2 Canny 杈圭紭
// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/CannyDetector_Demo.cpp銆乪dge.cpp
// THEORY: docs/ch02_imgproc.md 搂4
// TASK: 楂樻柉棰勫鐞?+ 鍙岄槇鍊兼粦鍔ㄦ潯
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int lowTh = 50;
static int ratio = 3;     // high = low * ratio
static int kSize = 3;

static void onTrack(int, void*) {
    int k = kSize | 1; if (k < 3) k = 3;
    Mat smooth;
    GaussianBlur(gray, smooth, Size(k, k), 0);
    Mat edges;
    Canny(smooth, edges, lowTh, lowTh * ratio, 3);
    Mat color;
    src.copyTo(color, edges);
    Mat up;
    hconcat(src, color, up);
    hconcat(up, edges, up);
    imshow("L2_13 canny src|color|edge", up);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_13 canny src|color|edge", WINDOW_AUTOSIZE);
    createTrackbar("low th", "L2_13 canny src|color|edge", &lowTh, 200, onTrack);
    createTrackbar("ratio",  "L2_13 canny src|color|edge", &ratio,  5,   onTrack);
    createTrackbar("gauss k","L2_13 canny src|color|edge", &kSize, 11,  onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
