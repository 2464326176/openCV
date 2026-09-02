// LEARN: L3 MeanShift 璺熻釜
// OFFICIAL: samples/cpp/tutorial_code/video/meanshift/meanshift.cpp
// THEORY: docs/ch04_video.md 搂璺熻釜
// TASK: VCG1 ROI 绠?hue 鐩存柟鍥撅紝VCG2 鍙嶆姇褰憋紝meanShift 杩唬鏀舵暃
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat f1 = imread(getImagePath("VCG1.jpg"));
    Mat f2 = imread(getImagePath("VCG2.jpg"));
    if (f1.empty() || f2.empty()) { logInfo("imread failed"); return -1; }
    resize(f1, f1, Size(480, 320)); resize(f2, f2, Size(480, 320));

    Rect roi(180, 100, 120, 120);
    Mat hsv1, hsv2;
    cvtColor(f1, hsv1, COLOR_BGR2HSV);
    cvtColor(f2, hsv2, COLOR_BGR2HSV);
    int bins = 16; float hr[] = {0, 180};
    const float* ranges[] = {hr};
    int ch[] = {0};

    Mat roiMat = hsv1(roi);
    Mat hist; calcHist(&roiMat, 1, ch, Mat(), hist, 1, &bins, ranges);
    normalize(hist, hist, 0, 255, NORM_MINMAX);

    Mat back; calcBackProject(&hsv2, 1, ch, hist, back, ranges);
    Rect win = roi;
    TermCriteria tc(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);
    meanShift(back, win, tc);

    Mat show = f2.clone();
    rectangle(show, roi, Scalar(0, 0, 255), 1); // 鍒濆
    rectangle(show, win, Scalar(0, 255, 0), 2); // 鏀舵暃
    logInfo("meanshift init=(%d,%d) -> (%d,%d)", roi.x, roi.y, win.x, win.y);
    dbgShow("L3_21 meanshift (red=init, green=result)", show, 0);
    return 0;
}
