// LEARN: L2 Histogram calculation
// OFFICIAL: samples/cpp/tutorial_code/Histograms_Matching/calcHist_Demo.cpp、snippets/imgproc_calc_hist.cpp
// THEORY: docs/ch02_imgproc.md §8
// TASK: calcHist grayscale 256 bins and draw
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;

static Mat drawHist(const Mat& src, int bins = 256) {
    int histSize[] = {bins};
    float range[] = {0, 256};
    const float* ranges[] = {range};
    int channels[] = {0};
    Mat hist;
    calcHist(&src, 1, channels, Mat(), hist, 1, histSize, ranges, true, false);
    double mx = 0; minMaxLoc(hist, 0, &mx);
    int w = 512, h = 300;
    Mat canvas(h, w, CV_8UC3, Scalar(0, 0, 0));
    int bw = cvRound(w * 1.0 / bins);
    for (int i = 0; i < bins; ++i) {
        double v = hist.at<float>(i) / (mx + 1e-6);
        rectangle(canvas, Point(i * bw, h),
                  Point((i + 1) * bw, (int)(h - v * h)),
                  Scalar(200, 200, 200), FILLED);
    }
    return canvas;
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    Mat hist = drawHist(gray);
    dbgMatInfo("gray", gray);
    Mat up;
    hconcat(gray, hist, up);
    dbgShow("L2_26 calcHist gray|hist", up, 0);
    return 0;
}
