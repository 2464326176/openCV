// LEARN: L2 杩為€氬煙
// OFFICIAL: samples/cpp/connected_components.cpp
// THEORY: docs/ch02_imgproc.md 搂7
// TASK: connectedComponents 涓?connectedComponentsWithStats 鐫€鑹?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray, bin;
static int th = 100;
static int useStats = 1;

static void onTrack(int, void*) {
    threshold(gray, bin, th, 255, THRESH_BINARY_INV + THRESH_OTSU);
    Mat labels, stats, cent;
    int n;
    if (useStats)
        n = connectedComponentsWithStats(bin, labels, stats, cent, 8, CV_32S);
    else
        n = connectedComponents(bin, labels, 8, CV_32S);
    logInfo("components=%d", n - 1);
    Mat color(src.size(), CV_8UC3, Scalar(0, 0, 0));
    RNG rng(12345);
    for (int i = 1; i < n; ++i) {
        Scalar c(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
        color.setTo(c, labels == i);
    }
    Mat up;
    hconcat(bin, color, up);
    imshow("L2_23 connectedComponents", up);
}

int main() {
    src = imread(getImagePath("VCG4.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_23 connectedComponents", WINDOW_AUTOSIZE);
    createTrackbar("th", "L2_23 connectedComponents", &th, 255, onTrack);
    createTrackbar("0plain 1stats", "L2_23 connectedComponents", &useStats, 1, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
