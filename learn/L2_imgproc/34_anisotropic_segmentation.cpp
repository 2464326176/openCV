// LEARN: L2 鍚勫悜寮傛€ф墿鏁ｅ垎鍓?
// OFFICIAL: tutorial_code/ImgProc/anisotropic_image_segmentation/anisotropic_image_segmentation.cpp
// THEORY: docs/ch02_imgproc.md 搂2.4.4
// TASK: 鐢ㄧ粨鏋勫紶閲忎及璁″眬閮ㄦ柟鍚?涓€鑷存€э紝鎸?coherence 闃堝€煎垎鍓茬嚎鎬х粨鏋?
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) src = makeSyntheticTestImage();
    if (src.channels() > 1) cvtColor(src, src, COLOR_BGR2GRAY);

    Mat blur, dx, dy, dxx, dxy, dyy;
    GaussianBlur(src, blur, Size(5, 5), 1.2);
    Sobel(blur, dx, CV_32F, 1, 0, 3);
    Sobel(blur, dy, CV_32F, 0, 1, 3);
    multiply(dx, dx, dxx);
    multiply(dy, dy, dyy);
    multiply(dx, dy, dxy);

    Mat J11, J12, J22, tmp1, tmp2, trace, det, coherence;
    GaussianBlur(dxx, J11, Size(7, 7), 2.0);
    GaussianBlur(dxy, J12, Size(7, 7), 2.0);
    GaussianBlur(dyy, J22, Size(7, 7), 2.0);
    add(J11, J22, trace);
    subtract(J11, J22, tmp1); multiply(tmp1, tmp1, tmp1);
    multiply(J12, J12, tmp2); multiply(tmp2, Scalar(4.0), tmp2);
    add(tmp1, tmp2, tmp1); sqrt(tmp1, det);
    divide(det, trace + 1e-6f, coherence);

    Mat mask;
    threshold(coherence, mask, 0.5, 255, THRESH_BINARY);
    mask.convertTo(mask, CV_8U);

    logInfo("coherence threshold=0.5: 瓒婇珮瓒婁繚鐣欏己鏂瑰悜缁撴瀯");
    dbgShowMany({"src", "coherence", "mask"}, {src, coherence, mask}, 0);
    return 0;
}
