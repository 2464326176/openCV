//********************
// Author:  yh
// Time:    2022/8/5.
//  CamShift 目标跟踪（自适应均值漂移）
//  流程：首帧选 ROI → 计算 H-S 直方图作为目标模型 → 逐帧反投影 + CamShift 跟踪
//  对应官方示例: camshift.cpp / camshiftdemo.cpp
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Rect g_selection;          // 鼠标框选区域
bool g_selecting = false;  // 正在框选

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
    Rect trackWindow;        // CamShift 的窗口状态（Rect&）
    RotatedRect trackBox;    // CamShift 返回值：带角度的旋转矩形

    while (true) {
        capture >> frame;
        if (frame.empty()) break;
        frame.copyTo(hue);   // 占位，避免 hue 未初始化引用

        if (!init) {
            // 首次框选后初始化目标直方图
            if (g_selection.width > 0 && g_selection.height > 0) {
                cvtColor(frame, hsv, COLOR_BGR2HSV);
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                mixChannels(&hsv, 1, &hue, 1, ch, 1);   // 提取 H 通道

                Mat roi(hue, g_selection), maskroi;
                inRange(hsv, Scalar(0, 30, 10), Scalar(180, 256, 255), mask);
                maskroi = mask(g_selection);
                calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &ranges);
                normalize(hist, hist, 0, 255, NORM_MINMAX);
                init = true;
                trackWindow = g_selection;   // 初始窗口即框选矩形
            }
        } else {
            // 每帧：反投影 → CamShift 迭代更新窗口
            cvtColor(frame, hsv, COLOR_BGR2HSV);
            int ch[] = {0, 0};
            hue.create(hsv.size(), hsv.depth());
            mixChannels(&hsv, 1, &hue, 1, ch, 1);
            calcBackProject(&hue, 1, 0, hist, backproj, &ranges);
            // backproj 中值滤波去噪后交给 CamShift
            medianBlur(backproj, backproj, 5);
            trackBox = CamShift(backproj, trackBox,
                                TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1));
            // 绘制旋转矩形
            ellipse(frame, trackBox, Scalar(0, 0, 255), 2);
        }
        imshow("camshift", frame);
        if (waitKey(30) == 27) break;
    }
    return 0;
}
