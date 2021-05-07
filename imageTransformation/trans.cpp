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
Mat g_srcImgae, g_srcGrayImage, g_dstImgae;

//canny
Mat g_cannyDetectedEdges;
int g_cannyLowThreshold = 1;

//sobel
Mat g_sobelGradient_x, g_sobelGradient_y;
Mat g_sobelAbsGradient_x, g_sobelAbsGradient_y;
int g_sobelKernelSize = 1;
//scharr
Mat g_scharrGradient_x, g_scharrGradient_y;
Mat g_scharrAbsGradient_x, g_scharrAbsGradient_y;

static void onCanny(int, void *);
static void onSobel(int, void *);
void scharr();

int main()
{
    String imgPath = "../static/img/yuhang.jpg";
    String imageName(imgPath.c_str());
    g_srcImgae = imread(imageName);

    g_dstImgae.create(g_srcImgae.size(), g_srcImgae.type());


    cvtColor(g_srcImgae, g_srcGrayImage, COLOR_RGB2GRAY);

    imshow("origin", g_srcGrayImage);
    createTrackbar("value：", "canny", &g_cannyLowThreshold, 120, onCanny);

    createTrackbar("value：", "canny", &g_sobelKernelSize, 3, onSobel);
    onCanny(0, 0);
    onSobel(0, 0);
    scharr();


    while(waitKey(1) != 'q') { };
    return 0;
}


void onCanny(int , void *)
{
    blur(g_srcGrayImage, g_cannyDetectedEdges, Size(3, 3));

    Canny(g_cannyDetectedEdges, g_cannyDetectedEdges, g_cannyLowThreshold, g_cannyLowThreshold * 3, 3);
    g_dstImgae = Scalar::all(0);

    g_srcImgae.copyTo(g_dstImgae, g_cannyDetectedEdges);
    imshow("canny", g_dstImgae);
}

void onSobel(int , void *)
{
    Sobel(g_srcImgae, g_sobelGradient_x, CV_16S, 1, 0, (2 * g_sobelKernelSize +1), 1, 1, BORDER_DEFAULT);
    Sobel(g_srcImgae, g_sobelGradient_y, CV_16S, 1, 0, (2 * g_sobelKernelSize +1), 1, 1, BORDER_DEFAULT);
    convertScaleAbs(g_sobelGradient_x, g_sobelAbsGradient_x);
    convertScaleAbs(g_sobelGradient_y, g_sobelAbsGradient_y);

    addWeighted(g_sobelAbsGradient_x, 0.5, g_sobelAbsGradient_y, 0.5, 0, g_dstImgae);
    imshow("sobel", g_dstImgae);
}

void scharr()
{
    Scharr(g_srcImgae, g_scharrGradient_x, CV_16S, 1, 0, 1, 0, BORDER_DEFAULT);
    Scharr(g_srcImgae, g_scharrGradient_y, CV_16S, 0, 1, 1, 0, BORDER_DEFAULT);
    convertScaleAbs(g_scharrGradient_x, g_scharrAbsGradient_x);
    convertScaleAbs(g_scharrGradient_y, g_scharrAbsGradient_y);

    addWeighted(g_scharrAbsGradient_x, 0.5, g_scharrAbsGradient_y, 0.5, 0, g_dstImgae);
    imshow("scharr", g_dstImgae);
}