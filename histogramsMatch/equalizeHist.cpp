//********************
// Author:  yh 
// Time:    2021/5/7.
// 
//********************
//
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


int main() {

    Mat srcIMage = imread("../static/data/lena.jpg");

    cvtColor(srcIMage, srcIMage, COLOR_BGR2GRAY);

    Mat dstImage;
    equalizeHist(srcIMage, dstImage);

    imshow("src", srcIMage);
    imshow("dst", dstImage);


    waitKey();

    return 0;
}
