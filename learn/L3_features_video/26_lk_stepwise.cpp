// LEARN: L3 LK Optical Flow Step-by-Step
// OFFICIAL: tutorial_code/video/optical_flow/optical_flow.cpp
// THEORY: docs/ch04_video.md §4.2
// TASK: goodFeaturesToTrack + calcOpticalFlowPyrLK stepwise tracking; winSize/maxLevel affect convergence
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat frame0 = imread(getImagePath("VCG1.jpg"), IMREAD_GRAYSCALE);
    Mat frame1 = imread(getImagePath("VCG2.jpg"), IMREAD_GRAYSCALE);
    if (frame0.empty() || frame1.empty()) {
        frame0 = makeSyntheticTestImage(320, 240);
        cvtColor(frame0, frame0, COLOR_BGR2GRAY);
        frame1 = frame0.clone();
        Mat M = (Mat_<float>(2, 3) << 1, 0, 8, 0, 1, 4);
        warpAffine(frame0, frame1, M, frame0.size());
    }

    std::vector<Point2f> pts0, pts1;
    goodFeaturesToTrack(frame0, pts0, 100, 0.3, 7);
    std::vector<uchar> status;
    std::vector<float> err;
    calcOpticalFlowPyrLK(frame0, frame1, pts0, pts1, status, err,
                         Size(21, 21), 3,
                         TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, 30, 0.01));

    Mat vis;
    cvtColor(frame0, vis, COLOR_GRAY2BGR);
    int tracked = 0;
    for (size_t i = 0; i < pts0.size(); ++i) {
        if (!status[i]) continue;
        ++tracked;
        circle(vis, pts0[i], 3, Scalar(0, 255, 0), -1);
        line(vis, pts0[i], pts1[i], Scalar(0, 0, 255), 1);
        circle(vis, pts1[i], 3, Scalar(255, 0, 0), -1);
    }
    logInfo("tracked %d / %zu points; winSize=21 maxLevel=3", tracked, pts0.size());
    dbgShow("L3_26 lk stepwise", vis, 0);
    return 0;
}

