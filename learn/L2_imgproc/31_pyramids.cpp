// LEARN: L2 鍥惧儚閲戝瓧濉?// OFFICIAL: samples/cpp/tutorial_code/ImgProc/Pyramids/Pyramids.cpp
// THEORY: docs/ch02_imgproc.md 搂閲戝瓧濉?// TASK: pyrDown/pyrUp 澶氬眰涓嬮噰鏍蜂笌涓婇噰鏍?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int level = 1;

static void onTrack(int, void*) {
    Mat g = src.clone();
    std::vector<Mat> down;
    down.push_back(g);
    for (int i = 0; i < level; ++i) {
        Mat d;
        pyrDown(down.back(), d);
        down.push_back(d);
        if (d.cols < 8 || d.rows < 8) break;
    }
    std::vector<Mat> up;
    Mat cur = down.back();
    up.push_back(cur);
    for (int i = 0; i < level; ++i) {
        Mat u;
        pyrUp(cur, u);
        up.push_back(u);
        cur = u;
    }
    Mat out = up[0].clone();
    for (size_t i = 1; i < up.size(); ++i) {
        int h = std::max(out.rows, up[i].rows);
        Mat ca = Mat::zeros(h, out.cols, out.type());
        out.copyTo(ca(Rect(0, 0, out.cols, out.rows)));
        Mat cb = Mat::zeros(h, up[i].cols, up[i].type());
        up[i].copyTo(cb(Rect(0, 0, up[i].cols, up[i].rows)));
        hconcat(ca, cb, out);
    }
    logInfo("levels=%d images=%d", level, (int)up.size());
    imshow("L2_31 pyramids", out);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_31 pyramids", WINDOW_AUTOSIZE);
    createTrackbar("level", "L2_31 pyramids", &level, 4, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
