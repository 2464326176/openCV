// LEARN: L3 LK 绋€鐤忓厜娴?// OFFICIAL: samples/cpp/lkdemo.cpp
// THEORY: docs/ch04_video.md 搂鍏夋祦
// TASK: VCG1 鈫?VCG2锛実oodFeatures 鍙栫偣锛宑alcOpticalFlowPyrLK 璺熻釜锛岀敾绠ご
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat f1 = imread(getImagePath("VCG1.jpg"));
    Mat f2 = imread(getImagePath("VCG2.jpg"));
    if (f1.empty() || f2.empty()) { logInfo("imread failed"); return -1; }
    resize(f1, f1, Size(480, 320)); resize(f2, f2, Size(480, 320));
    Mat g1, g2; cvtColor(f1, g1, COLOR_BGR2GRAY); cvtColor(f2, g2, COLOR_BGR2GRAY);

    std::vector<Point2f> p1;
    goodFeaturesToTrack(g1, p1, 400, 0.01, 7);
    std::vector<Point2f> p2;
    std::vector<uchar> st; std::vector<float> err;
    TermCriteria tc(TermCriteria::COUNT | TermCriteria::EPS, 30, 0.01);
    calcOpticalFlowPyrLK(g1, g2, p1, p2, st, err, Size(15, 15), 2, tc);

    Mat show = f1.clone();
    int ok = 0;
    for (size_t i = 0; i < p1.size(); ++i) {
        if (!st[i]) continue;
        ++ok;
        arrowedLine(show, p1[i], p2[i], Scalar(0, 255, 0), 1);
        circle(show, p2[i], 2, Scalar(0, 0, 255), -1);
    }
    logInfo("tracked %d / %zu", ok, p1.size());
    dbgShow("L3_17 LK optical flow", show, 0);
    return 0;
}
