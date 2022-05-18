//********************
// Author:  yh 
// Time:    2021/5/17.
// 
//********************
//
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat g_srcImage, g_dstImage, g_grayImage;
int thresh = 30;
int max_thresh = 175;

void onCornerHarris(int, void *);



int main() {
    g_srcImage = imread("../static/data/lena.jpg");

    cvtColor(g_srcImage, g_grayImage, COLOR_BGR2GRAY);
    namedWindow("src", WINDOW_AUTOSIZE);
    createTrackbar("thresh", "src", &thresh, max_thresh, onCornerHarris);
    onCornerHarris(0, 0);

    waitKey(0);
    return 0;
}


void onCornerHarris(int, void *) {
    Mat dstImage, normImage, scaledImage;

    dstImage = Mat::zeros(g_srcImage.size(), CV_32FC1);
    g_dstImage = g_srcImage.clone();

    cornerHarris(g_grayImage, dstImage, 2, 3, 0.04, BORDER_DEFAULT);

    normalize(dstImage, normImage, 0, 255, NORM_MINMAX, CV_32FC1, Mat());

    convertScaleAbs(normImage, scaledImage);

    for(int j = 0; j < normImage.rows; ++j) {
        for(int i = 0; i < normImage.cols; ++i) {
            if ((int)normImage.at<float>(j, i) > thresh + 80) {
                circle(g_dstImage, Point(i, j), 5, Scalar(10, 10, 255), 2, 8 ,0);
                circle(scaledImage, Point(i, j), 5, Scalar(10, 10, 255), 2, 8, 0);
            }
        }

    }

    imshow("src", g_dstImage);
    imshow("dst", scaledImage);
}













