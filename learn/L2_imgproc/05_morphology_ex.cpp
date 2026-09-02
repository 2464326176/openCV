// LEARN: L2 morphologyEx — seven operations
// OFFICIAL: samples/cpp/tutorial_code/ImgProc/Morphology_2.cpp
// THEORY: docs/ch02_imgproc.md §2
// TASK: morphologyEx open/close gradient/tophat/blackhat trackbar
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray, bin;
static int op = MORPH_OPEN;
static int kSize = 3;

static void onTrack(int, void*) {
    int k = kSize | 1; if (k < 3) k = 3;
    Mat elem = getStructuringElement(MORPH_RECT, Size(k, k));
    Mat dst;
    morphologyEx(bin, dst, op, elem);
    Mat up;
    hconcat(bin, dst, up);
    imshow("L2_05 morphologyEx", up);
}

int main() {
    src = imread(getImagePath("VCG1.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    threshold(gray, bin, 128, 255, THRESH_BINARY_INV + THRESH_OTSU);
    namedWindow("L2_05 morphologyEx", WINDOW_AUTOSIZE);
    createTrackbar("0open 1close 2grad 3tophat 4blackhat",
                   "L2_05 morphologyEx", &op, 4, onTrack);
    createTrackbar("ksize", "L2_05 morphologyEx", &kSize, 21, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
