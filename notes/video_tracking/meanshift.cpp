//********************
// Author:  yh
// MeanShift object tracking:
//  Workflow: fixed window in first frame -> compute target H-channel histogram ->
//            back-project + meanShift iterate per frame
//  Difference from CamShift: window size is fixed and does not output a rotated rectangle
//  Official example: tutorial_code/video/meanshift/meanshift.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    String videoName("../data/vtest.avi");
    if (argc > 1) videoName = argv[1];

    VideoCapture capture(videoName);
    if (!capture.isOpened()) { cout << "无法打开视频" << endl; return -1; }

    Mat frame;
    capture >> frame;
    if (frame.empty()) return -1;

    // Initial tracking window (hardcoded position; could be mouse-selected in practice)
    Rect track_window(300, 200, 100, 50);

    // 1. Build the target color model (H-channel histogram) from the window in the first frame
    Mat roi = frame(track_window);
    Mat hsv_roi, mask;
    cvtColor(roi, hsv_roi, COLOR_BGR2HSV);
    // Filter low-saturation / low-brightness pixels to reduce background interference
    inRange(hsv_roi, Scalar(0, 60, 32), Scalar(180, 255, 255), mask);

    float range_[] = {0, 180};
    const float *range[] = {range_};
    Mat roi_hist;
    int histSize[] = {180};
    int channels[] = {0};   // Use only the H channel
    calcHist(&hsv_roi, 1, channels, mask, roi_hist, 1, histSize, range);
    normalize(roi_hist, roi_hist, 0, 255, NORM_MINMAX);

    // 2. Per frame: back-project + meanShift iterate
    TermCriteria term_crit(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);
    while (true) {
        capture >> frame;
        if (frame.empty()) break;

        Mat hsv, dst;
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        calcBackProject(&hsv, 1, channels, roi_hist, dst, range);  // probability map

        // Mean shift: iterate the window toward the centroid of the probability map (fixed window size)
        meanShift(dst, track_window, term_crit);

        rectangle(frame, track_window, Scalar(0, 255, 0), 2);
        imshow("meanshift", frame);
        if (waitKey(30) == 27) break;
    }
    return 0;
}
