// LEARN: L2 浜害/瀵规瘮搴︿氦浜掕皟鑺?
// OFFICIAL: tutorial_code/ImgProc/changing_contrast_brightness_image/changing_contrast_brightness_image.cpp
// THEORY: docs/ch02_imgproc.md 搂1
// TASK: trackbar 璋?alpha(瀵规瘮搴?/beta(浜害)锛沘lpha>1 澧炲己瀵规瘮锛宐eta 鏁翠綋鍋忕Щ
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat gSrc;
static int gAlphaPct = 100; // 100 -> alpha=1.0
static int gBeta = 0;

static void onTrack(int, void*) {
    double alpha = gAlphaPct / 100.0;
    Mat dst;
    gSrc.convertTo(dst, -1, alpha, gBeta);
    imshow("L2_33 contrast_brightness", dst);
}

int main() {
    gSrc = imread(getImagePath("lena.jpg"));
    if (gSrc.empty()) gSrc = makeSyntheticTestImage();
    namedWindow("L2_33 contrast_brightness", WINDOW_AUTOSIZE);
    createTrackbar("alpha x0.01", "L2_33 contrast_brightness", &gAlphaPct, 300, onTrack);
    createTrackbar("beta", "L2_33 contrast_brightness", &gBeta, 100, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
