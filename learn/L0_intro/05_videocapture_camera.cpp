// LEARN: L0 VideoCapture 鎽勫儚澶?// OFFICIAL: samples/cpp/videocapture_basic.cpp, videocapture_camera.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md 搂videoio
// TASK: 鎵撳紑鎽勫儚澶达紱澶辫触鍥為€€鐢ㄩ潤鎬佸浘寰幆鏄剧ず
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    VideoCapture cap(0);
    Mat frame;
    if (!cap.isOpened()) {
        logInfo("camera not available, fallback to static image");
        Mat img = imread(getImagePath("VCG1.jpg"));
        if (img.empty()) { logInfo("static image also failed"); return -1; }
        namedWindow("L0_05 fallback", WINDOW_AUTOSIZE);
        while (true) {
            imshow("L0_05 fallback", img);
            if (waitKey(30) == 27) break;
        }
        return 0;
    }
    namedWindow("L0_05 camera", WINDOW_AUTOSIZE);
    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        imshow("L0_05 camera", frame);
        if (waitKey(30) == 27) break;
    }
    return 0;
}
