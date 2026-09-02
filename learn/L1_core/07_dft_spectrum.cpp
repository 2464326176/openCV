// LEARN: L1 DFT 骞呭害璋?// OFFICIAL: samples/cpp/tutorial_code/core/discrete_fourier_transform/discrete_fourier_transform.cpp銆乨ft.cpp
// THEORY: docs/ch01_core.md 搂2.7 绂绘暎鍌呴噷鍙跺彉鎹?// TASK: 鐏板害鍥炬墿灞曟渶浼樺昂瀵革紱DFT锛沵agnitude+log 鍙栧箙搴﹁氨锛涘洓璞￠檺浜ゆ崲鏄剧ず
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (img.empty()) { logInfo("imread failed"); return -1; }

    // 1) 鎵╁睍涓?DFT 鏈€浼樺昂瀵革紝0 濉厖
    int m = getOptimalDFTSize(img.rows);
    int n = getOptimalDFTSize(img.cols);
    Mat padded;
    copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols,
                   BORDER_CONSTANT, Scalar::all(0));

    // 2) 缁勮鍙岄€氶亾澶嶆暟鐭╅樀骞?DFT
    Mat planes[] = {
        Mat_<float>(padded),
        Mat::zeros(padded.size(), CV_32F)
    };
    Mat complex;
    merge(planes, 2, complex);
    dft(complex, complex);
    split(complex, planes);

    // 3) 骞呭害璋憋細sqrt(re^2+im^2) -> +1 -> log
    Mat mag;
    magnitude(planes[0], planes[1], mag);
    mag += Scalar::all(1);
    log(mag, mag);

    // 4) 瑁佹垚鍋舵暟灏哄鍚庡仛鍥涜薄闄愪氦鎹紙鎶婁綆棰戞惉鍒颁腑蹇冿級
    mag = mag(Rect(0, 0, mag.cols & -2, mag.rows & -2));
    int cx = mag.cols / 2, cy = mag.rows / 2;
    Mat q0(mag, Rect(0,  0,  cx, cy));
    Mat q1(mag, Rect(cx, 0,  cx, cy));
    Mat q2(mag, Rect(0,  cy, cx, cy));
    Mat q3(mag, Rect(cx, cy, cx, cy));
    Mat tmp;
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);

    // 5) 褰掍竴鍖栧埌 0~1 渚夸簬鏄剧ず
    normalize(mag, mag, 0, 1, NORM_MINMAX);
    dbgMatInfo("mag", mag);
    dbgShowMany({"src", "padded", "spectrum"},
                {img, padded, mag}, 0);
    return 0;
}
