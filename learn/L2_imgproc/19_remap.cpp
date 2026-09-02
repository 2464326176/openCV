// LEARN: L2 remap 鑷畾涔夋槧灏?// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/Remap_Demo.cpp
// THEORY: docs/ch02_imgproc.md 搂6
// TASK: 鑷缓 map_x/map_y 鍋氶暅鍍?娉㈡氮/缂╁皬
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int mode = 0;   // 0 mirror 1 wave 2 shrink

static void onTrack(int, void*) {
    Mat mapX(src.size(), CV_32FC1);
    Mat mapY(src.size(), CV_32FC1);
    for (int y = 0; y < src.rows; ++y) {
        for (int x = 0; x < src.cols; ++x) {
            if (mode == 0) {
                mapX.at<float>(y, x) = (float)(src.cols - x - 1);
                mapY.at<float>(y, x) = (float)y;
            } else if (mode == 1) {
                mapX.at<float>(y, x) = (float)x + 10 * sin(y / 8.0);
                mapY.at<float>(y, x) = (float)y + 10 * cos(x / 8.0);
            } else {
                mapX.at<float>(y, x) = (float)(x * 0.5 + src.cols * 0.25);
                mapY.at<float>(y, x) = (float)(y * 0.5 + src.rows * 0.25);
            }
        }
    }
    Mat dst;
    remap(src, dst, mapX, mapY, INTER_LINEAR, BORDER_REFLECT);
    imshow("L2_19 remap", dst);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_19 remap", WINDOW_AUTOSIZE);
    createTrackbar("0mirror 1wave 2shrink", "L2_19 remap", &mode, 2, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
