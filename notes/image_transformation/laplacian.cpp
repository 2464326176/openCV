//********************
// Author:  yh 
// Time:    2021/5/3.
// 
//********************
//
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


int main()
{
    Mat src, src_gray, dst, abs_dst;

    String imgPath = "../static/img/yuhang.jpg";
    String imageName(imgPath.c_str());

    src = imread(imageName);

    GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);

    cvtColor(src, src_gray, COLOR_RGB2GRAY);

    Laplacian(src_gray, dst, CV_16S, 3, 1, 0, BORDER_DEFAULT);

    convertScaleAbs(dst, abs_dst);

    imshow("gray", src_gray);
    imshow("abs_dst", abs_dst);
    waitKey(0);
    return 0;
}