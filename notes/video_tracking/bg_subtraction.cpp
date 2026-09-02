//********************
// Author:  yh
// Time:    2022/8/5.
//  Background subtraction: MOG2 extracts the foreground moving object
//  Workflow: create BackgroundSubtractorMOG2 -> apply per frame to get foreground mask ->
//            morphological denoising
//  Official example: bg_sub.cpp / bgfg_segm.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    String videoName("../data/vtest.avi");
    if (argc > 1) videoName = argv[1];

    VideoCapture capture(videoName);
    if (!capture.isOpened()) { cout << "could not open video" << endl; return -1; }

    Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2(500, 16, true);
    // 500: history frame count; 16: variance threshold (higher = more sensitive); true: detect shadows

    Mat frame, fgMask, fgImg, kernel;
    kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));

    while (true) {
        capture >> frame;
        if (frame.empty()) break;

        pMOG2->apply(frame, fgMask);          // output foreground binary mask
        morphologyEx(fgMask, fgMask, MORPH_OPEN, kernel);  // opening removes isolated noise

        frame.copyTo(fgImg, fgMask);          // use mask to extract the colored foreground
        imshow("frame", frame);
        imshow("foreground mask", fgMask);
        imshow("foreground image", fgImg);

        if (waitKey(30) == 27) break;
    }
    return 0;
}
