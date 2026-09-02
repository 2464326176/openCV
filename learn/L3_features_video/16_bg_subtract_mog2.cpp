// LEARN: L3 MOG2 Background Subtraction
// OFFICIAL: samples/cpp/tutorial_code/video/bg_sub.cpp、bgfg_segm.cpp
// THEORY: docs/ch04_video.md §背景
// TASK: use VCG1 as background, synthesize moving square sequence, createBackgroundSubtractorMOG2 get foreground
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat bg = imread(getImagePath("VCG1.jpg"));
    if (bg.empty()) { logInfo("imread failed"); return -1; }
    resize(bg, bg, Size(480, 320));

    Ptr<BackgroundSubtractorMOG2> mog = createBackgroundSubtractorMOG2(300, 16, false);
    Mat fg, show;
    for (int f = 0; f < 60; ++f) {
        Mat frame = bg.clone();
        Rect r(40 + f * 5, 100 + (int)(40 * sin(f * 0.2)), 60, 60);
        r = r & Rect(0, 0, frame.cols, frame.rows);
        rectangle(frame, r, Scalar(0, 0, 255), -1);

        mog->apply(frame, fg);
        threshold(fg, fg, 200, 255, THRESH_BINARY);

        std::vector<std::vector<Point>> conts;
        findContours(fg, conts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        frame.copyTo(show);
        for (auto& c : conts) {
            if (contourArea(c) < 200) continue;
            rectangle(show, boundingRect(c), Scalar(0, 255, 0), 2);
        }
        putText(show, format("frame=%d", f), Point(10, 20),
                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
        imshow("L3_16 mog2", show);
        if (waitKey(30) == 27) break;
    }
    Mat bgEst; mog->getBackgroundImage(bgEst);
    if (!bgEst.empty()) dbgShow("L3_16 background est", bgEst, 0);
    return 0;
}

