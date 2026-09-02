// LEARN: L2 鐩存柟鍥惧弽鍚戞姇褰?// OFFICIAL: samples/cpp/tutorial_code/Histograms_Matching/calcBackProject_Demo1.cpp銆乧alcBackProject_Demo2.cpp
// THEORY: docs/ch02_imgproc.md 搂8
// TASK: 鐢ㄧ洰鏍?ROI 鐨?HSV 鐩存柟鍥?calcBackProject 瀹氫綅鐩镐技鍖哄煙
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, hsv, hist;

static void updateHist(const Rect& roi) {
    Mat mask = Mat::ones(hsv.size(), CV_8U);
    int ch[] = {0, 1};
    int bins[] = {32, 32};
    float r0[] = {0, 180}; const float* rr0[] = {r0};
    float r1[] = {0, 256}; const float* rr1[] = {r1};
    const float* ranges[] = {r0, r1};
    calcHist(&hsv, 1, ch, mask(roi), hist, 2, bins, ranges, true, false);
    normalize(hist, hist, 0, 255, NORM_MINMAX);
}

int main() {
    src = imread(getImagePath("VCG2.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, hsv, COLOR_BGR2HSV);
    Rect roi(src.cols / 2 - 30, src.rows / 2 - 30, 60, 60);
    roi &= Rect(0, 0, src.cols, src.rows);
    updateHist(roi);
    int ch[] = {0, 1};
    float r0[] = {0, 180}; const float* rr0[] = {r0};
    float r1[] = {0, 256}; const float* rr1[] = {r1};
    const float* ranges[] = {r0, r1};
    Mat back;
    calcBackProject(&hsv, 1, ch, hist, back, ranges, 1, true);
    Mat disc = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    filter2D(back, back, -1, disc);
    Mat color;
    src.copyTo(color, back);
    rectangle(src, roi, Scalar(0, 255, 0), 2);
    Mat up;
    hconcat(src, back, up);
    hconcat(up, color, up);
    dbgShow("L2_29 backproject src|back|result", up, 0);
    return 0;
}
