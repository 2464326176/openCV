//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "cFunction.h"
using namespace cv;
using namespace std;

String g_yhpath = "E:\\lyh\\code\\openCV\\static\\img\\yuhang.jpg";


void createAlphaMat(Mat &mat) {
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            Vec4b &rgba = mat.at<Vec4b>(i, j);
            rgba[0] = UCHAR_MAX;
            rgba[1] = saturate_cast<uchar> (float (mat.cols - j)) / ((float) mat.cols * UCHAR_MAX);
            rgba[2] = saturate_cast<uchar> (float (mat.cols - j)) / ((float) mat.rows * UCHAR_MAX);
            rgba[3] = saturate_cast<uchar> (0.5 * (rgba[1] + rgba[2]));
        }
    }
}

int main1()
{

    /*alpha */
    Mat mat(480, 640, CV_8UC4);

    createAlphaMat(mat);


    vector<int>compression_params;
    compression_params.push_back(IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

    try {
        imwrite("alpha.png", mat, compression_params);
        namedWindow("png", WINDOW_AUTOSIZE);
        imshow("png", mat);
        waitKey(0);
    } catch(runtime_error &ex) {
        cout << "error" << endl;
        return -1;
    }


    return 0;
}




#define WINDOW_NAME "opencv test"
#define IMG_PATH_1 "E:\\lyh\\code\\openCV\\static\\img\\yuhang.jpg"
#define IMG_PATH_2 "E:\\lyh\\code\\openCV\\static\\img\\gril.jpg"

int g_nMaxAlphaValue = 100;
int g_nAlphaValueSlider;
double g_dAlphaValue;
double g_dBetaValue;

Mat g_srcImgae1;
Mat g_srcImgae2;
Mat g_dstImage;

void onTrackbar(int, void *) {
    g_dAlphaValue = double(g_nAlphaValueSlider / g_nMaxAlphaValue);
    g_dBetaValue = (1.0 - g_dAlphaValue);

    addWeighted(g_srcImgae1, g_dAlphaValue, g_srcImgae2, g_dBetaValue, 0.0, g_dstImage);

    imshow(WINDOW_NAME, g_dstImage);


}


int main2()
{

    g_srcImgae1 = imread(IMG_PATH_1, IMREAD_GRAYSCALE);
    g_srcImgae2 = imread(IMG_PATH_2, IMREAD_GRAYSCALE);

    g_nAlphaValueSlider = 70;

    namedWindow(WINDOW_NAME, 0);

    char TracKbarName[50];

    createTrackbar(TracKbarName, WINDOW_NAME, &g_nAlphaValueSlider, g_nMaxAlphaValue, onTrackbar);

    onTrackbar(g_nAlphaValueSlider, 0);

    waitKey(0);

    return 0;
}
