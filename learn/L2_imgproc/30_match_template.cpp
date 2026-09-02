// LEARN: L2 Template matching
// OFFICIAL: samples/cpp/tutorial_code/Histograms_Matching/MatchTemplate_Demo.cpp、mask_tmpl.cpp
// THEORY: docs/ch02_imgproc.md §8
// TASK: crop ROI from source as template, matchTemplate six methods find best location
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray, tmpl, gtmpl;
static int mode = 0;

static void onTrack(int, void*) {
    int m[] = { TM_SQDIFF, TM_SQDIFF_NORMED, TM_CCORR,
                TM_CCORR_NORMED, TM_CCOEFF, TM_CCOEFF_NORMED };
    Mat result;
    matchTemplate(gray, gtmpl, result, m[mode]);
    normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
    bool sq = (m[mode] == TM_SQDIFF || m[mode] == TM_SQDIFF_NORMED);
    Point mLoc;
    if (sq) { double mn; minMaxLoc(result, &mn, 0, &mLoc, 0); }
    else    { double mx; minMaxLoc(result, 0, &mx, 0, &mLoc); }
    Mat disp = src.clone();
    rectangle(disp, mLoc, Point(mLoc.x + tmpl.cols, mLoc.y + tmpl.rows),
              Scalar(0, 0, 255), 2);
    Mat up;
    hconcat(tmpl, disp, up);
    imshow("L2_30 matchTemplate tmpl|result", up);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    // crop ROI from source as template
    Rect roi(80, 100, 80, 100);
    roi &= Rect(0, 0, src.cols, src.rows);
    tmpl = src(roi).clone();
    gtmpl = gray(roi).clone();
    namedWindow("L2_30 matchTemplate tmpl|result", WINDOW_AUTOSIZE);
    createTrackbar("0sq 1sqn 2ccorr 3ccorrn 4ccoeff 5ccoeffn",
                   "L2_30 matchTemplate tmpl|result", &mode, 5, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
