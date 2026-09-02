// LEARN: L2 Flood fill
// OFFICIAL: samples/cpp/ffilldemo.cpp
// THEORY: docs/ch02_imgproc.md §7
// TASK: floodFill with trackbar-controlled lo/up difference tolerance
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, show;
static int loDiff = 20;
static int upDiff = 20;
static int conn = 4;

static void onMouse(int event, int x, int y, int, void*) {
    if (event != EVENT_LBUTTONDOWN) return;
    Point seed(x, y);
    int flags = conn | FLOODFILL_FIXED_RANGE;
    Rect rect;
    src.copyTo(show);
    floodFill(show, seed, Scalar(255, 0, 0), &rect,
              Scalar(loDiff, loDiff, loDiff),
              Scalar(upDiff, upDiff, upDiff), flags);
    logInfo("floodFill rect=%dx%d", rect.width, rect.height);
    imshow("L2_24 floodFill", show);
}

int main() {
    src = imread(getImagePath("VCG5.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    src.copyTo(show);
    namedWindow("L2_24 floodFill", WINDOW_AUTOSIZE);
    setMouseCallback("L2_24 floodFill", onMouse);
    createTrackbar("loDiff", "L2_24 floodFill", &loDiff, 100, nullptr);
    createTrackbar("upDiff", "L2_24 floodFill", &upDiff, 100, nullptr);
    createTrackbar("0conn4 1conn8", "L2_24 floodFill", &conn, 1, nullptr);
    imshow("L2_24 floodFill", show);
    waitKey(0);
    return 0;
}
