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

Mat g_srcImage, g_grayImgae;

int g_nThresh = 80;
int g_nThresh_max = 255;
RNG g_rng(123456);
Mat g_cannyMat_output;
vector<vector<Point> > g_vContours;
vector<Vec4i> g_vHierarchy;

void onThreshChange(int, void *);


int main() {
    String imgPath = "../static/img/yuhang.jpg";
    String imageName(imgPath.c_str());

    g_srcImage = imread(imageName);

    cvtColor(g_srcImage, g_grayImgae, COLOR_RGB2GRAY);
    blur(g_grayImgae, g_grayImgae, Size(3, 3));
    imshow("blur", g_grayImgae);

    createTrackbar("canny 阈值", "blur", &g_nThresh, g_nThresh_max, onThreshChange);
    onThreshChange(0, 0);

    waitKey(0);
    return 0;
}

void onThreshChange(int, void *)
{
    Canny(g_grayImgae, g_cannyMat_output, g_nThresh, g_nThresh *2, 3);
    findContours(g_cannyMat_output, g_vContours, g_vHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    Mat drawing =Mat::zeros(g_cannyMat_output.size(), CV_8UC3);
    for (size_t i =  0; i < g_vContours.size(); ++i)
    {
        Scalar color = Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255));
        drawContours(drawing, g_vContours, i, color, 2, 8, g_vHierarchy, 0, Point());
    }

    imshow("segmentation", drawing);
}



