#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
int main()
{
    Mat M;
    Point center = Point(255, 255); //圆形
    int r = 100; //半径
    circle(M, center, r, Scalar(0, 0, 0)); //原图像 圆心 半径 颜色 粗组 线型

    namedWindow("display window", WINDOW_AUTOSIZE);
    imshow("display window", M);
    waitKey(0);

    return 0;
}