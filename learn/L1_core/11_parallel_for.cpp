// LEARN: L1 parallel_for parallel pixel processing
// OFFICIAL: samples/cpp/tutorial_code/core/how_to_use_OpenCV_parallel_for_/how_to_use_OpenCV_parallel_for_.cpp、how_to_use_OpenCV_parallel_for_new.cpp
// THEORY: docs/ch01_core.md §2.8 parallel computing
// TASK: parallel_for_ + Range for color reduction; compare serial vs parallel performance (optional)
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static const int DIV = 32;

struct ColorReduceBody : public ParallelLoopBody {
    Mat& img;
    int  div;
    ColorReduceBody(Mat& i, int d) : img(i), div(d) {}

    void operator()(const Range& r) const override {
        int n = img.cols * img.channels();
        for (int y = r.start; y < r.end; ++y) {
            uchar* p = img.ptr<uchar>(y);
            for (int x = 0; x < n; ++x)
                p[x] = (uchar)(p[x] / div * div + div / 2);
        }
    }
};

static void colorReduceSerial(Mat& img, int div) {
    int n = img.cols * img.channels();
    for (int y = 0; y < img.rows; ++y) {
        uchar* p = img.ptr<uchar>(y);
        for (int x = 0; x < n; ++x)
            p[x] = (uchar)(p[x] / div * div + div / 2);
    }
}

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat serial = src.clone();
    Mat par    = src.clone();

    int64 t0 = getTickCount();
    colorReduceSerial(serial, DIV);
    int64 t1 = getTickCount();
    parallel_for_(Range(0, par.rows), ColorReduceBody(par, DIV));
    int64 t2 = getTickCount();

    logInfo("serial   %.3f ms",
            (double)(t1 - t0) * 1000.0 / getTickFrequency());
    logInfo("parallel %.3f ms",
            (double)(t2 - t1) * 1000.0 / getTickFrequency());

    Mat diff;
    absdiff(serial, par, diff);
    logInfo("serial vs parallel diff = %d", countNonZero(diff));

    dbgShowMany({"src", "serial", "parallel"},
                {src, serial, par}, 0);
    return 0;
}
