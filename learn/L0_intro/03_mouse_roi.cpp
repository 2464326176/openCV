// LEARN: L0 榧犳爣鐢荤煩褰?ROI
// OFFICIAL: samples/cpp/drawing.cpp (榧犳爣浜嬩欢瀛愰泦)
// THEORY: docs/ch08_gui_gapi_gpu.md 搂highgui
// TASK: setMouseCallback 宸﹂敭鎷栨嫿鐢荤煩褰紱鏉惧紑鎶?ROI 澶嶅埗鍒版柊绐楀彛
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, show;
static Point pt1;
static bool dragging = false;

static void onMouse(int event, int x, int y, int, void*) {
    if (event == EVENT_LBUTTONDOWN) {
        pt1 = Point(x, y);
        dragging = true;
    } else if (event == EVENT_MOUSEMOVE && dragging) {
        src.copyTo(show);
        rectangle(show, pt1, Point(x, y), Scalar(0, 255, 0), 2);
        imshow("L0_03 mouse", show);
    } else if (event == EVENT_LBUTTONUP) {
        dragging = false;
        Rect roi = Rect(pt1, Point(x, y)) & Rect(0, 0, src.cols, src.rows);
        if (roi.area() > 0) {
            Mat crop = src(roi).clone();
            imshow("ROI", crop);
            dbgPrint("roi", roi);
        }
    }
}

int main() {
    src = imread(getImagePath("VCG2.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    src.copyTo(show);
    namedWindow("L0_03 mouse", WINDOW_AUTOSIZE);
    setMouseCallback("L0_03 mouse", onMouse);
    imshow("L0_03 mouse", show);
    waitKey(0);
    return 0;
}
