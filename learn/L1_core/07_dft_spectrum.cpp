// LEARN: L1 DFT magnitude spectrum

// OFFICIAL: samples/cpp/tutorial_code/core/discrete_fourier_transform/discrete_fourier_transform.cpp、dft.cpp
// THEORY: docs/ch01_core.md §2.7 Discrete Fourier Transform

// TASK: extend grayscale to optimal DFT size; DFT; magnitude+log for spectrum; quadrant swap display
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (img.empty()) { logInfo("imread failed"); return -1; }

    // 1) Extend to optimal DFT size, zero-pad
    int m = getOptimalDFTSize(img.rows);
    int n = getOptimalDFTSize(img.cols);
    Mat padded;
    copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols,
                   BORDER_CONSTANT, Scalar::all(0));

    // 2) Assemble two-channel complex matrix and DFT
    Mat planes[] = {
        Mat_<float>(padded),
        Mat::zeros(padded.size(), CV_32F)
    };
    Mat complex;
    merge(planes, 2, complex);
    dft(complex, complex);
    split(complex, planes);

    // 3) Magnitude spectrum: sqrt(re^2+im^2) -> +1 -> log
    Mat mag;
    magnitude(planes[0], planes[1], mag);
    mag += Scalar::all(1);
    log(mag, mag);

    // 4) Crop to even size, then quadrant swap (move low frequencies to center)
    mag = mag(Rect(0, 0, mag.cols & -2, mag.rows & -2));
    int cx = mag.cols / 2, cy = mag.rows / 2;
    Mat q0(mag, Rect(0,  0,  cx, cy));
    Mat q1(mag, Rect(cx, 0,  cx, cy));
    Mat q2(mag, Rect(0,  cy, cx, cy));
    Mat q3(mag, Rect(cx, cy, cx, cy));
    Mat tmp;
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);

    // 5) Normalize to 0~1 for display
    normalize(mag, mag, 0, 1, NORM_MINMAX);
    dbgMatInfo("mag", mag);
    dbgShowMany({"src", "padded", "spectrum"},
                {img, padded, mag}, 0);
    return 0;
}
