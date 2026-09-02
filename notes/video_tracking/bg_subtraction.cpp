//********************
// Author:  yh
// Time:    2022/8/5.
//  背景减除：MOG2 提取前景运动目标
//  流程：创建 BackgroundSubtractorMOG2 → 逐帧 apply 得到前景掩码 → 形态学去噪
//  对应官方示例: bg_sub.cpp / bgfg_segm.cpp
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
    // 500: 历史帧数; 16: 方差阈值(越大越敏感); true: 检测阴影

    Mat frame, fgMask, fgImg, kernel;
    kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));

    while (true) {
        capture >> frame;
        if (frame.empty()) break;

        pMOG2->apply(frame, fgMask);          // 输出前景二值掩码
        morphologyEx(fgMask, fgMask, MORPH_OPEN, kernel);  // 开运算去孤立噪点

        frame.copyTo(fgImg, fgMask);          // 用掩码抠出前景彩色图
        imshow("frame", frame);
        imshow("foreground mask", fgMask);
        imshow("foreground image", fgImg);

        if (waitKey(30) == 27) break;
    }
    return 0;
}
