// LEARN: L2 Scharr gradient
// OFFICIAL: tutorial_code/ImgTrans/Sobel_Demo.cpp
// THEORY: docs/ch02_imgproc.md §4
// TASK: Scharr accurate 3x3 gradient, compared with Sobel
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);

    Mat gx3, gy3, gSx, gSy;
    Scharr(gray, gSx, CV_16S, 1, 0);
    Scharr(gray, gSy, CV_16S, 0, 1);
    Sobel(gray, gx3, CV_16S, 1, 0, 3);
    Sobel(gray, gy3, CV_16S, 0, 1, 3);

    Mat absSx, absSy, abs3x, abs3y, sch, sob;
    convertScaleAbs(gSx, absSx); convertScaleAbs(gSy, absSy);
    convertScaleAbs(gx3, abs3x); convertScaleAbs(gy3, abs3y);
    addWeighted(absSx, 0.5, absSy, 0.5, 0, sch);
    addWeighted(abs3x, 0.5, abs3y, 0.5, 0, sob);

    Mat r1, r2;
    hconcat(sch, sob, r1);
    hconcat(absSx, absSy, r2);
    Mat all;
    vconcat(r1, r2, all);
    dbgShow("L2_11 scharr | sobel | schX schY", all, 0);
    return 0;
}
