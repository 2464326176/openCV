#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <opencv_utils.h>

using namespace cv;

int boxFilterValue = 6;
int meanFilterValue = 10;
int gaussianFilterValue = 6;
int medianFilterValue = 10;
int bilateralFilterValue = 10;
Mat srcImage, boxFilterImage, meanFilterImage, gaussianFilterImage, medianFilterImage,
    bilateralFilterImage;

void on_boxFilterTrackbar(int, void*) {
    boxFilter(srcImage, boxFilterImage, -1, Size(boxFilterValue, boxFilterValue));
    imshow("box filter", boxFilterImage);
}

void on_meanFilterTrackbar(int, void*) {
    blur(srcImage, meanFilterImage, Size(meanFilterValue, meanFilterValue), Point(-1, -1));
    imshow("mean filter", meanFilterImage);
}

void on_gaussianFilterTrackbar(int, void*) {
    GaussianBlur(srcImage, gaussianFilterImage, Size(gaussianFilterValue * 2 + 1, gaussianFilterValue * 2 + 1), 0, 0);
    imshow("gaussian filter", gaussianFilterImage);
}

void on_medianFilterTrackbar(int, void*) {
    medianBlur(srcImage, medianFilterImage, medianFilterValue * 2 + 1);
    imshow("median filter", medianFilterImage);
}

void on_bilateralFilterTrackbar(int, void*) {
    bilateralFilter(srcImage, bilateralFilterImage, bilateralFilterValue, bilateralFilterValue * 2.0, bilateralFilterValue / 2.0);
    imshow("bilateral filter", bilateralFilterImage);
}

int main() {
    srcImage = imread(getImagePath("OIP.png"));
    if (srcImage.empty()) {
        logInfo("imread failed");
        return -1;
    }

    namedWindow("ori image", WINDOW_AUTOSIZE);
    imshow("ori image", srcImage);

    namedWindow("box filter", WINDOW_AUTOSIZE);
    createTrackbar("ksize", "box filter", &boxFilterValue, 50, on_boxFilterTrackbar);
    on_boxFilterTrackbar(0, nullptr);

    namedWindow("mean filter", WINDOW_AUTOSIZE);
    createTrackbar("ksize", "mean filter", &meanFilterValue, 50, on_meanFilterTrackbar);
    on_meanFilterTrackbar(0, nullptr);

    namedWindow("gaussian filter", WINDOW_AUTOSIZE);
    createTrackbar("ksize", "gaussian filter", &gaussianFilterValue, 50, on_gaussianFilterTrackbar);
    on_gaussianFilterTrackbar(0, nullptr);

    namedWindow("median filter", WINDOW_AUTOSIZE);
    createTrackbar("ksize", "median filter", &medianFilterValue, 50, on_medianFilterTrackbar);
    on_medianFilterTrackbar(0, nullptr);

    namedWindow("bilateral filter", WINDOW_AUTOSIZE);
    createTrackbar("d", "bilateral filter", &bilateralFilterValue, 50, on_bilateralFilterTrackbar);
    on_bilateralFilterTrackbar(0, nullptr);

    waitKey(0);
    return 0;
}
