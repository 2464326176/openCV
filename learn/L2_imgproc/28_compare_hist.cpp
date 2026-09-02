// LEARN: L2 Histogram comparison
// OFFICIAL: samples/cpp/tutorial_code/Histograms_Matching/compareHist_Demo.cpp
// THEORY: docs/ch02_imgproc.md §8
// TASK: compareHist four methods (CORREL/CHISQR/INTERSECT/BHATTACHARYYA)
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat calcHist1D(const Mat& src, int bins = 256) {
    int histSize[] = {bins};
    float range[] = {0, 256};
    const float* ranges[] = {range};
    int channels[] = {0};
    Mat h;
    calcHist(&src, 1, channels, Mat(), h, 1, histSize, ranges, true, false);
    normalize(h, h, 0, 1, NORM_MINMAX, -1, Mat());
    return h;
}

int main() {
    Mat base = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    Mat t1   = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    Mat t2   = imread(getImagePath("OIP.png"), IMREAD_GRAYSCALE);
    if (base.empty() || t1.empty() || t2.empty()) {
        logInfo("imread failed"); return -1;
    }
    Mat hB = calcHist1D(base), h1 = calcHist1D(t1), h2 = calcHist1D(t2);
    int methods[] = { HISTCMP_CORREL, HISTCMP_CHISQR, HISTCMP_INTERSECT, HISTCMP_BHATTACHARYYA };
    const char* names[] = { "CORREL", "CHISQR", "INTERSECT", "BHATT" };
    for (int i = 0; i < 4; ++i) {
        double dSame = compareHist(hB, h1, methods[i]);
        double dDiff = compareHist(hB, h2, methods[i]);
        logInfo("method=%s  same=%f  diff=%f", names[i], dSame, dDiff);
    }
    Mat up;
    hconcat(base, t2, up);
    dbgShow("L2_28 compareHist lena|OIP", up, 0);
    return 0;
}
