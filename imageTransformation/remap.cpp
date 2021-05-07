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
    Mat srcImage, dstImage;
    Mat map_x, map_y;
    String imgPath = "../static/img/yuhang.jpg";
    String imageName(imgPath.c_str());
    srcImage = imread(imageName);

    dstImage.create(srcImage.size(), srcImage.type());
    map_x.create(srcImage.size(), CV_32FC1);
    map_y.create(srcImage.size(), CV_32FC1);

    for (int i = 0; i < srcImage.rows; ++i)
    {
        for (int j = 0; j < srcImage.cols; ++j)
        {
            map_x.at<float>(i, j) = static_cast<float>(j);
            map_y.at<float>(i, j) = static_cast<float>(srcImage.rows - i);
        }
    }

    remap(srcImage, dstImage, map_x, map_y, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));

    imshow("origin", srcImage);
    imshow("remap", dstImage);


    waitKey(0);
    return 0;
}
