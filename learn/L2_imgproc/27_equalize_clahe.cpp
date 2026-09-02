// LEARN: L2 Histogram equalization and CLAHE
// OFFICIAL: samples/cpp/tutorial_code/Histograms_Matching/EqualizeHist_Demo.cpp、demhist.cpp
// THEORY: docs/ch02_imgproc.md §8
// TASK: equalizeHist vs CLAHE clipLimit/size comparison
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int clip = 2;
static int tile = 8;

static void onTrack(int, void*) {
    Mat eq;
    equalizeHist(gray, eq);
    int t = tile | 1; if (t < 2) t = 2;
    Ptr<CLAHE> clahe = createCLAHE(clip, Size(t, t));
    Mat ce;
    clahe->apply(gray, ce);
    Mat up;
    hconcat(gray, eq, up);
    hconcat(up, ce, up);
    imshow("L2_27 gray|eq|clahe", up);
}

int main() {
    src = imread(getImagePath("VCG6.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_27 gray|eq|clahe", WINDOW_AUTOSIZE);
    createTrackbar("clipLimit", "L2_27 gray|eq|clahe", &clip, 40, onTrack);
    createTrackbar("tileSize",  "L2_27 gray|eq|clahe", &tile, 32, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
