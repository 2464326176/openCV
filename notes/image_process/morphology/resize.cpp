#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void resize_demo() {
    Mat src = imread("../data/images/lena.jpg");
    if (src.empty()) {
        cout << "could not load image.." << endl;
        return;
    }
    imshow("input", src);

    Mat dst;
    resize(src, dst, Size(src.cols / 2, src.rows / 2), 0, 0, INTER_LINEAR);
    imshow("output", dst);

    Mat dst2;
    resize(src, dst2, Size(src.cols * 2, src.rows * 2), 0, 0, INTER_LINEAR);
    imshow("output2", dst2);
}

void pyrDown_demo() {
    Mat src = imread("../data/images/lena.jpg");
    if (src.empty()) {
        cout << "could not load image.." << endl;
        return;
    }
    imshow("input", src);
    Mat dst;
    pyrDown(src, dst, Size(src.cols / 2, src.rows / 2));
    imshow("output", dst);
}

void pyrUp_demo() {
    Mat src = imread("../data/images/lena.jpg");
    if (src.empty()) {
        cout << "could not load image.." << endl;
        return;
    }
    imshow("input", src);
    Mat dst;
    pyrUp(src, dst, Size(src.cols * 2, src.rows * 2));
    imshow("output", dst);
}

int main() {
    // resize_demo();

    pyrDown_demo();
    // pyrUp_demo();
    waitKey(0);
    return 0;
}