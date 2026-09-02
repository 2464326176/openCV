// LEARN: L3 CamShift Tracking
// OFFICIAL: samples/cpp/camshiftdemo.cpp、tutorial_code/video/meanshift/camshift.cpp
// THEORY: docs/ch04_video.md §tracking
// TASK: VCG1 get ROI, hue histogram, VCG2 backproject, CamShift converge to target
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
    TermCriteria tc(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);
    RotatedRect box = CamShift(back, roi, tc);

    Mat show = f2.clone();
    Point2f v[4]; box.points(v);
    for (int i = 0; i < 4; ++i) line(show, v[i], v[(i + 1) % 4], Scalar(0, 255, 0), 2);
    rectangle(show, Rect(180, 100, 120, 120), Scalar(0, 0, 255), 1); // initial ROI
    logInfo("camshift center=(%.1f,%.1f)", box.center.x, box.center.y);
    dbgShow("L3_20 camshift (red=init, green=result)", show, 0);
    return 0;
}

