//********************
// Author:  yh 
// Time:    2021/5/14.
// 
//********************
//
#include <opencv2/opencv.hpp>


#include <iostream>

using namespace cv;
using namespace std;


int main() {

    Mat srcImage = imread("../static/gril/0.jpg", 0);
    imshow("origin", srcImage);

    Mat cornerStrength, harrisConner;
    cornerHarris(srcImage, cornerStrength, 2, 3, 0.01);

    threshold(cornerStrength, harrisConner, 0.00001, 255, THRESH_BINARY);
    imshow("cornerHarris", harrisConner);

    waitKey(0);
    return 0;
}