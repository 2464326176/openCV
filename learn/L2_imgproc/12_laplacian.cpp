// LEARN: L2 Laplacian 浜岄樁瀵?// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/Laplace_Demo.cpp銆乴aplace.cpp
// THEORY: docs/ch02_imgproc.md 搂4
// TASK: 楂樻柉棰勫鐞嗗悗 Laplacian锛宎bs+threshold
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int kSize = 3;
static int blurK = 3;

static void onTrack(int, void*) {
    int k = kSize | 1; if (k < 1) k = 1;
    int bk = blurK | 1; if (bk < 3) bk = 3;
    Mat smooth, lap, absLap;
    GaussianBlur(gray, smooth, Size(bk, bk), 0);
    Laplacian(smooth, lap, CV_16S, k);
    convertScaleAbs(lap, absLap);
    Mat up;
    hconcat(gray, absLap, up);
    imshow("L2_12 laplacian", up);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_12 laplacian", WINDOW_AUTOSIZE);
    createTrackbar("lap ksize", "L2_12 laplacian", &kSize, 7, onTrack);
    createTrackbar("gauss k",   "L2_12 laplacian", &blurK, 11, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
