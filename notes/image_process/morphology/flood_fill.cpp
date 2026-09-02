#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main() {
    Mat image = imread("../data/images/2019101203383336641.jpg");
    imshow("original image", image);

    Rect ccomp;
    floodFill(image, Point(0, 0), Scalar(155, 255, 55), &ccomp,
        Scalar(20, 20, 20), Scalar(20, 20, 20));

    imshow("flood fill image", image);
    waitKey(0);
    return 0;
}