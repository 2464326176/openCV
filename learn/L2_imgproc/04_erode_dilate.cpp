// LEARN: L2 Morphology erode and dilate
// OFFICIAL: samples/cpp/tutorial_code/ImgProc/Morphology_1.cpp
// THEORY: docs/ch02_imgproc.md §2
// TASK: erode/dilate + getStructuringElement(MORPH_RECT/CROSS/ELLIPSE)
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray, bin;
static int shape = MORPH_RECT;
static int kSize = 3;
static int op = 0;   // 0 erode 1 dilate

static void onTrack(int, void*) {
    int k = kSize | 1; if (k < 3) k = 3;
    Mat elem = getStructuringElement(shape, Size(k, k));
    Mat dst;
    if (op == 0) erode(bin, dst, elem);
    else         dilate(bin, dst, elem);
    Mat up;
    hconcat(bin, dst, up);
    imshow("L2_04 erode/dilate", up);
}

int main() {
    src = imread(getImagePath("VCG1.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    threshold(gray, bin, 128, 255, THRESH_BINARY_INV + THRESH_OTSU);
    namedWindow("L2_04 erode/dilate", WINDOW_AUTOSIZE);
    createTrackbar("shape 0rect 1cross 2ell",
                   "L2_04 erode/dilate", &shape, 2, onTrack);
    createTrackbar("ksize", "L2_04 erode/dilate", &kSize, 21, onTrack);
    createTrackbar("0erode 1dilate", "L2_04 erode/dilate", &op, 1, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
