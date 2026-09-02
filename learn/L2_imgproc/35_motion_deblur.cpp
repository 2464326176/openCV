// LEARN: L2 Frequency-domain motion deblur (Wiener approximation)
// OFFICIAL: tutorial_code/ImgProc/motion_deblur_filter/motion_deblur_filter.cpp
// THEORY: docs/ch02_imgproc.md §2.9.5
// TASK: synthesize motion blur then DFT inverse filter approximate restoration; PSF length/angle affects quality
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat psfMotionBlur(const Size& sz, int len, double angleDeg) {
    Mat psf = Mat::zeros(sz, CV_32F);
    Point center(sz.width / 2, sz.height / 2);
    double rad = angleDeg * CV_PI / 180.0;
    Point2d dir(cos(rad), sin(rad));
    for (int i = -len / 2; i <= len / 2; ++i) {
        int x = cvRound(center.x + i * dir.x);
        int y = cvRound(center.y + i * dir.y);
        if (0 <= x && x < sz.width && 0 <= y && y < sz.height)
            psf.at<float>(y, x) = 1.f;
    }
    psf /= (float)sum(psf)[0];
    return psf;
}

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) src = makeSyntheticTestImage();
    if (src.channels() > 1) cvtColor(src, src, COLOR_BGR2GRAY);
    resize(src, src, Size(256, 256));

    const int psfLen = 15;
    const double psfAngle = 0.0;
    Mat psf = psfMotionBlur(src.size(), psfLen, psfAngle);

    Mat srcF, psfF, blurF, blur;
    dft(src, srcF, DFT_COMPLEX_OUTPUT);
    dft(psf, psfF, DFT_COMPLEX_OUTPUT);
    mulSpectrums(srcF, psfF, blurF, 0);
    dft(blurF, blur, DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);
    blur.convertTo(blur, CV_8U);

    Mat psfPad;
    copyMakeBorder(psf, psfPad, 0, src.rows - psf.rows, 0, src.cols - psf.cols,
                   BORDER_CONSTANT, Scalar::all(0));
    dft(psfPad, psfF, DFT_COMPLEX_OUTPUT);

    Mat planes[2];
    split(psfF, planes);
    Mat mag;
    magnitude(planes[0], planes[1], mag);
    Mat wiener;
    divide(1.0, mag + 0.05, wiener);

    Mat blurF2, restoredF, restored;
    dft(blur, blurF2, DFT_COMPLEX_OUTPUT);
    Mat blurPlanes[2], restPlanes[2];
    split(blurF2, blurPlanes);
    multiply(blurPlanes[0], wiener, restPlanes[0]);
    multiply(blurPlanes[1], wiener, restPlanes[1]);
    merge(restPlanes, 2, restoredF);
    dft(restoredF, restored, DFT_INVERSE | DFT_SCALE | DFT_REAL_OUTPUT);
    restored.convertTo(restored, CV_8U);

    logInfo("PSF len=%d angle=%.0f: larger len means stronger blur, harder to restore", psfLen, psfAngle);
    dbgShowMany({"src", "motion blur", "restored"}, {src, blur, restored}, 0);
    return 0;
}
