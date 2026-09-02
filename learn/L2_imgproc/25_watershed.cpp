// LEARN: L2 鍒嗘按宀垎鍓?// OFFICIAL: samples/cpp/watershed.cpp銆乼utorial_code/ImgTrans/imageSegmentation.cpp
// THEORY: docs/ch02_imgproc.md 搂7
// TASK: 浜屽€?寮€杩愮畻+璺濈鍙樻崲鍒濆鍖?marker锛寃atershed 鍒嗗壊
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray, bin;
static int th = 100;
static int distTh = 50;   // 璺濈闃堝€?x0.01

static void onTrack(int, void*) {
    Mat g;
    if (src.channels() == 3) cvtColor(src, g, COLOR_BGR2GRAY);
    else g = src;
    threshold(g, bin, th, 255, THRESH_BINARY_INV + THRESH_OTSU);
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    Mat open;
    morphologyEx(bin, open, MORPH_OPEN, kernel, Point(-1, -1), 2);
    Mat bg;
    dilate(open, bg, kernel, Point(-1, -1), 3);
    Mat dist;
    distanceTransform(open, dist, DIST_L2, 3);
    normalize(dist, dist, 0, 1, NORM_MINMAX);
    double mxd;
    minMaxLoc(dist, 0, &mxd);
    Mat fg;
    threshold(dist, fg, mxd * (distTh / 100.0), 255, THRESH_BINARY);
    fg.convertTo(fg, CV_8U);
    Mat unknown;
    subtract(bg, fg, unknown);
    Mat labels, stats, cent;
    int n = connectedComponentsWithStats(fg, labels, stats, cent, 8);
    Mat marks = Mat::zeros(bin.size(), CV_32S);
    for (int i = 1; i < n; ++i)
        marks.setTo(Scalar(i), labels == i);
    marks.setTo(Scalar(n), unknown > 0);
    Mat mS = marks.clone();
    watershed(src, mS);
    Mat color = src.clone();
    for (int y = 0; y < mS.rows; ++y)
        for (int x = 0; x < mS.cols; ++x)
            if (mS.at<int>(y, x) == -1)
                color.at<Vec3b>(y, x) = Vec3b(0, 0, 255);
    Mat up;
    hconcat(bin, color, up);
    imshow("L2_25 watershed bin|result", up);
}

int main() {
    src = imread(getImagePath("VCG2.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_25 watershed bin|result", WINDOW_AUTOSIZE);
    createTrackbar("th",      "L2_25 watershed bin|result", &th, 255, onTrack);
    createTrackbar("distTh%", "L2_25 watershed bin|result", &distTh, 100, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
