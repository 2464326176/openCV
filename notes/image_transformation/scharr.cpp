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
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y, dst;

    String imgPath = "../static/img/yuhang.jpg";
    String imageName(imgPath.c_str());

    Mat srcImage = imread(imageName, IMREAD_GRAYSCALE);
    //x,y 方向的梯度
    Scharr(srcImage, grad_x, CV_16S, 1, 0, 1, 0, BORDER_DEFAULT);
    Scharr(srcImage, grad_y, CV_16S, 0, 1, 1, 0, BORDER_DEFAULT);
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    //合并梯度
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);
    imshow("origin", srcImage);
    imshow("x", abs_grad_x);
    imshow("y", abs_grad_y);
    imshow("x,y", dst);


    waitKey(0);
    return 0;
}