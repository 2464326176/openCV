//********************
// Author:  yh
// Time:    2022/8/5.
//  CamShift object tracking (adaptive mean shift)
//  Workflow: select ROI in first frame -> compute H-S histogram as target model ->
//            back-project + CamShift track per frame
//  Official example: camshift.cpp / camshiftdemo.cpp
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Rect g_selection;          // Mouse-selected region
bool g_selecting = false;  // Currently selecting

static void onMouse(int event, int x, int y, int, void *) {
    if (g_selecting) {
        g_selection.x = MIN(x, g_selection.x);
        g_selection.y = MIN(y, g_selection.y);
        g_selection.width = std::abs(x - g_selection.x);
        g_selection.height = std::abs(y - g_selection.y);
    }
    switch (event) {
        case EVENT_LBUTTONDOWN: g_selection = Rect(x, y, 0, 0); g_selecting = true; break;
        case EVENT_LBUTTONUP:   g_selecting = false; break;
    }
}

int main(int argc, char **argv) {
    String videoName("../data/vtest.avi");
    if (argc > 1) videoName = argv[1];

    VideoCapture capture(videoName);
    if (!capture.isOpened()) { cout << "could not open video" << endl; return -1; }

    namedWindow("camshift", WINDOW_AUTOSIZE);
    setMouseCallback("camshift", onMouse);

    Mat frame, hsv, hue, mask, hist, backproj;
    int hsize = 16;
    float hranges[] = {0, 180};
    const float *ranges = {hranges};
    bool init = false;
    Rect trackWindow;        // CamShift window state (Rect&)
    RotatedRect trackBox;    // CamShift return value: rotated rectangle with angle

    while (true) {
        capture >> frame;
        if (frame.empty()) break;
        frame.copyTo(hue);   // placeholder to avoid uninitialized hue reference

        if (!init) {
            // Initialize target histogram after the first selection
            if (g_selection.width > 0 && g_selection.height > 0) {
                cvtColor(frame, hsv, COLOR_BGR2HSV);
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                mixChannels(&hsv, 1, &hue, 1, ch, 1);   // extract H channel

                Mat roi(hue, g_selection), maskroi;
                inRange(hsv, Scalar(0, 30, 10), Scalar(180, 256, 255), mask);
                maskroi = mask(g_selection);
                calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &ranges);
                normalize(hist, hist, 0, 255, NORM_MINMAX);
                init = true;
                trackWindow = g_selection;   // initial window = selected rectangle
            }
        } else {
            // Per frame: back-project -> CamShift iterate to update window
            cvtColor(frame, hsv, COLOR_BGR2HSV);
            int ch[] = {0, 0};
            hue.create(hsv.size(), hsv.depth());
            mixChannels(&hsv, 1, &hue, 1, ch, 1);
            calcBackProject(&hue, 1, 0, hist, backproj, &ranges);
            // Median filter the backproj to denoise before handing it to CamShift
            medianBlur(backproj, backproj, 5);
            trackBox = CamShift(backproj, trackBox,
                                TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1));
            // Draw the rotated rectangle
            ellipse(frame, trackBox, Scalar(0, 0, 255), 2);
        }
        imshow("camshift", frame);
        if (waitKey(30) == 27) break;
    }
    return 0;
}
