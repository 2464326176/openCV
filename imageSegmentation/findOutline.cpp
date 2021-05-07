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
    String imgPath = "../static/img/yuhang.jpg";
    String imageName(imgPath.c_str());

    Mat srcImage = imread(imageName, 0);

    Mat dstImage = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC3);
    srcImage = srcImage > 119;

    imshow("1", srcImage);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(srcImage, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

    for (int index = 0; index >= 0; index = hierarchy[index][0]) {
        Scalar color(rand()&255, rand()&255, rand()&255);
        drawContours(dstImage, contours, index, color, FILLED, 8, hierarchy);
    }

    imshow("2", dstImage);

    waitKey(0);
    return 0;
}



