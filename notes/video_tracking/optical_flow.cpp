//********************
// Author:  yh
// Time:    2022/8/5.
//  Sparse optical flow: Lucas-Kanade pyramidal flow calcOpticalFlowPyrLK
//  Workflow: first frame goodFeaturesToTrack picks corners -> track per frame -> draw trajectory
//  Official example: optical_flow.cpp / lkdemo.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    String videoName("../data/vtest.avi");
    if (argc > 1) videoName = argv[1];

    VideoCapture capture(videoName);
    if (!capture.isOpened()) { cout << "could not open video" << endl; return -1; }

    RNG rng;
    vector<Scalar> colors;
    for (int i = 0; i < 100; i++)
        colors.push_back(Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256)));

    Mat old_frame, old_gray, frame, frame_gray, mask;
    vector<Point2f> p0, p1;
    vector<uchar> status;
    vector<float> err;

    capture >> old_frame;
    if (old_frame.empty()) return -1;
    cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
    mask = Mat::zeros(old_frame.size(), old_frame.type());

    // First frame: use Shi-Tomasi corners as tracked points
    goodFeaturesToTrack(old_gray, p0, 100, 0.3, 7, Mat(), 7, false, 0.04);

    while (true) {
        capture >> frame;
        if (frame.empty()) break;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

        // Pyramidal LK flow: p0 = points in previous frame, p1 = corresponding points in current frame
        TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 0.03);
        calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err,
                             Size(15, 15), 2, criteria);

        vector<Point2f> good_new;
        for (size_t i = 0; i < p0.size(); i++) {
            if (status[i]) {   // status=1 means tracking succeeded
                good_new.push_back(p1[i]);
                // Draw line from previous frame to current frame to show the trajectory
                line(mask, p0[i], p1[i], colors[i], 2);
                circle(frame, p1[i], 3, colors[i], -1);
            }
        }

        Mat img;
        add(frame, mask, img);            // base image + trajectory overlay
        imshow("LK sparse optical flow", img);

        if (waitKey(30) == 27) break;     // ESC to exit

        // Swap: current frame becomes the next old frame
        old_gray = frame_gray.clone();
        p0 = good_new;
    }
    return 0;
}
