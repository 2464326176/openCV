// LEARN: L0 Hello imread
// OFFICIAL: samples/cpp/tutorial_code/introduction/display_image/display_image.cpp
// THEORY: docs/ch01_core.md §1.1
// TASK: imread image; check empty; imshow; waitKey; press 's' to save as PNG
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) {
        logInfo("imread failed: lena.jpg");
        return -1;
    }
    dbgMatInfo("img", img);

    namedWindow("L0_01 hello", WINDOW_AUTOSIZE);
    imshow("L0_01 hello", img);
    int k = waitKey(0);
    if (k == 's' || k == 'S') {
        imwrite("l0_01_saved.png", img);
        logInfo("saved -> l0_01_saved.png");
    }
    return 0;
}