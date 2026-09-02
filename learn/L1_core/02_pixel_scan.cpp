// LEARN: L1 鍍忕礌閬嶅巻
// OFFICIAL: samples/cpp/tutorial_code/core/how_to_scan_images/how_to_scan_images.cpp
// THEORY: docs/ch01_core.md 搂2.5
// TASK: at / ptr / iterator 涓夌鏂瑰紡鎵弿鍍忕礌鍋氶鑹茬缉鍑忥紱dbgTime 璁℃椂姣旇緝
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static const int DIV = 32;

// 鏂瑰紡 1: at<>
static void scanAt(Mat& img) {
    for (int y = 0; y < img.rows; ++y)
        for (int x = 0; x < img.cols; ++x) {
            Vec3b& p = img.at<Vec3b>(y, x);
            p[0] = (uchar)(p[0] / DIV * DIV + DIV / 2);
            p[1] = (uchar)(p[1] / DIV * DIV + DIV / 2);
            p[2] = (uchar)(p[2] / DIV * DIV + DIV / 2);
        }
}

// 鏂瑰紡 2: ptr<uchar> 琛屾寚閽?static void scanPtr(Mat& img) {
    int n = img.cols * img.channels();
    for (int y = 0; y < img.rows; ++y) {
        uchar* p = img.ptr<uchar>(y);
        for (int x = 0; x < n; ++x)
            p[x] = (uchar)(p[x] / DIV * DIV + DIV / 2);
    }
}

// 鏂瑰紡 3: MatIterator_
static void scanIter(Mat& img) {
    MatIterator_<Vec3b> it = img.begin<Vec3b>();
    MatIterator_<Vec3b> end = img.end<Vec3b>();
    for (; it != end; ++it) {
        (*it)[0] = (uchar)((*it)[0] / DIV * DIV + DIV / 2);
        (*it)[1] = (uchar)((*it)[1] / DIV * DIV + DIV / 2);
        (*it)[2] = (uchar)((*it)[2] / DIV * DIV + DIV / 2);
    }
}

static double costMs(int64 t0, int64 t1) {
    return (double)(t1 - t0) * 1000.0 / getTickFrequency();
}

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat a = src.clone(), b = src.clone(), c = src.clone();
    int64 t0 = getTickCount(); scanAt(a);   int64 t1 = getTickCount();
    int64 t2 = getTickCount(); scanPtr(b);  int64 t3 = getTickCount();
    int64 t4 = getTickCount(); scanIter(c); int64 t5 = getTickCount();

    logInfo("at        %.3f ms", costMs(t0, t1));
    logInfo("ptr       %.3f ms", costMs(t2, t3));
    logInfo("iterator  %.3f ms", costMs(t4, t5));

    dbgShowMany({"src", "at", "ptr", "iter"},
                {src, a, b, c}, 0);
    return 0;
}
