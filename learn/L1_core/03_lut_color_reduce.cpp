// LEARN: L1 LUT 棰滆壊缂╁噺
// OFFICIAL: samples/cpp/color_reduce.cpp銆乻amples/cpp/tutorial_code/core/how_to_use_OpenCV_parallel_for_/how_to_use_OpenCV_parallel_for_.cpp
// THEORY: docs/ch01_core.md 搂LUT
// TASK: 鏌ヨ〃 256 鍘?32 绾э紱瀵规瘮鎵嬪啓寰幆涓?LUT 鎬ц兘锛沘pplyColorMap 婕旂ず
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int g_div = 32; // 娉ㄦ剰锛氬彉閲忓悕涓嶈兘鐢?div锛屼笌 stdlib div() 鍐茬獊

static void onTrack(int, void*) {
    if (g_div < 2) g_div = 2;

    // 1) LUT 璺緞锛氭瀯寤?1x256 鏌ヨ〃
    Mat lut(1, 256, CV_8UC1);
    uchar* lp = lut.ptr();
    for (int i = 0; i < 256; ++i)
        lp[i] = (uchar)(i / g_div * g_div + g_div / 2);

    Mat reduced;
    double t0 = (double)getTickCount();
    LUT(src, lut, reduced);
    double t1 = (double)getTickCount();
    logInfo("LUT      %.3f ms", (t1 - t0) * 1000.0 / getTickFrequency());

    // 2) 鎵嬪啓寰幆锛坧tr 琛屾壂鎻忥級
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

    // 3) applyColorMap锛氭妸鐏板害 LUT 缁撴灉浼僵鑹插彲瑙嗗寲
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
