//********************
// Author:  yh
// Time:    2022/8/5.
//  稀疏光流：Lucas-Kanade 金字塔光流 calcOpticalFlowPyrLK
//  流程：首帧 goodFeaturesToTrack 选角点 → 逐帧跟踪 → 绘制轨迹
//  对应官方示例: optical_flow.cpp / lkdemo.cpp
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

    // 首帧用 Shi-Tomasi 角点作为跟踪点
    goodFeaturesToTrack(old_gray, p0, 100, 0.3, 7, Mat(), 7, false, 0.04);

    while (true) {
        capture >> frame;
        if (frame.empty()) break;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

        // 金字塔 LK 光流：p0 是上一帧点，p1 是当前帧对应点
        TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 0.03);
        calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err,
                             Size(15, 15), 2, criteria);

        vector<Point2f> good_new;
        for (size_t i = 0; i < p0.size(); i++) {
            if (status[i]) {   // status=1 表示跟踪成功
                good_new.push_back(p1[i]);
                // 上一帧到当前帧连线，绘制轨迹
                line(mask, p0[i], p1[i], colors[i], 2);
                circle(frame, p1[i], 3, colors[i], -1);
            }
        }

        Mat img;
        add(frame, mask, img);            // 底图 + 轨迹叠加
        imshow("LK sparse optical flow", img);

        if (waitKey(30) == 27) break;     // ESC 退出

        // 交换：当前帧作为下一轮旧帧
        old_gray = frame_gray.clone();
        p0 = good_new;
    }
    return 0;
}
