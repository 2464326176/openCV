// LEARN: L2 copyMakeBorder 杈圭晫濉厖
// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/copyMakeBorder_demo.cpp
// THEORY: docs/ch02_imgproc.md 搂6
// TASK: copyMakeBorder 浜旂 borderType trackbar
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int top = 20, bottom = 20, left = 20, right = 20;
static int btype = 0;

static void onTrack(int, void*) {
    int types[] = { BORDER_CONSTANT, BORDER_REPLICATE, BORDER_REFLECT,
                    BORDER_WRAP, BORDER_REFLECT_101 };
    int t = types[btype];
    static RNG rng(12345);
    Scalar value(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
    Mat dst;
    copyMakeBorder(src, dst, top, bottom, left, right, t, value);
    imshow("L2_20 copyMakeBorder", dst);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_20 copyMakeBorder", WINDOW_AUTOSIZE);
    createTrackbar("top",    "L2_20 copyMakeBorder", &top, 100, onTrack);
    createTrackbar("bottom", "L2_20 copyMakeBorder", &bottom, 100, onTrack);
    createTrackbar("left",   "L2_20 copyMakeBorder", &left, 100, onTrack);
    createTrackbar("right",  "L2_20 copyMakeBorder", &right, 100, onTrack);
    createTrackbar("0const 1rep 2refl 3wrap 4refl101",
                   "L2_20 copyMakeBorder", &btype, 4, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
