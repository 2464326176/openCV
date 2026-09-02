// LEARN: L1 LUT color reduction
// OFFICIAL: samples/cpp/color_reduce.cpp、samples/cpp/tutorial_code/core/how_to_use_OpenCV_parallel_for_/how_to_use_OpenCV_parallel_for_.cpp
// THEORY: docs/ch01_core.md §LUT
// TASK: LUT 256->32 levels; compare manual loop vs LUT performance; applyColorMap demo
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int g_div = 32; // NOTE: name 'div' conflicts with stdlib div()

static void onTrack(int, void*) {
    if (g_div < 2) g_div = 2;

    // 1) LUT path: build 1x256 lookup table
    Mat lut(1, 256, CV_8UC1);
    uchar* lp = lut.ptr();
    for (int i = 0; i < 256; ++i)
        lp[i] = (uchar)(i / g_div * g_div + g_div / 2);

    Mat reduced;
    double t0 = (double)getTickCount();
    LUT(src, lut, reduced);
    double t1 = (double)getTickCount();
    logInfo("LUT      %.3f ms", (t1 - t0) * 1000.0 / getTickFrequency());

    // 2) Manual loop (ptr row scan)
    Mat manual = src.clone();
    int n = manual.cols * manual.channels();
    double t2 = (double)getTickCount();
    for (int y = 0; y < manual.rows; ++y) {
        uchar* p = manual.ptr<uchar>(y);
        for (int x = 0; x < n; ++x)
            p[x] = (uchar)(p[x] / g_div * g_div + g_div / 2);
    }
    double t3 = (double)getTickCount();
    logInfo("manual   %.3f ms", (t3 - t2) * 1000.0 / getTickFrequency());

    // 3) applyColorMap: pseudo-color visualize the grayscale LUT result
    Mat colored;
    applyColorMap(reduced, colored, COLORMAP_JET);

    Mat diff;
    absdiff(reduced, manual, diff);
    logInfo("div=%d lut-vs-manual diff = %d", g_div, countNonZero(diff));

    dbgShowMany({"src", "lut", "manual", "colormap"},
                {src, reduced, manual, colored}, 0);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("src", WINDOW_AUTOSIZE);
    createTrackbar("div", "src", &g_div, 64, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
