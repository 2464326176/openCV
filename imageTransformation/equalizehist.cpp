//********************
// Author:  yh 
// Time:    2021/5/4.
// 
//********************
//
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


int main() {
    Mat srcImage, dstImage;
    String imgPath = "../static/data/baboon.jpg";
    String imageName(imgPath.c_str());
    srcImage = imread(imageName);

    cvtColor(srcImage, srcImage, COLOR_BGR2GRAY);

    equalizeHist(srcImage, dstImage);

    imshow("src", srcImage);
    imshow("dst", dstImage);
    waitKey(0);
    return  0;
}