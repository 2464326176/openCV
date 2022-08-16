/*
 * @Author: yh
 * @Date: 2022/8/4 21:53
 * @Description: 
 * @FilePath: image.cpp
 */
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
//#include "units.h"
using namespace cv;

void colorReduceByPtr(Mat &srcImage, Mat &destImage, int div){
    destImage = srcImage.clone();
    int rows = destImage.rows;
    int cols = destImage.cols * destImage.channels();

    for(int i = 0; i < rows; ++i) {
        uchar* data = destImage.ptr<uchar>(i);
        for(int j = 0; j < cols; ++j) {
            data[j] = data[j] / div *div + div/2;
        }
    }
}

void colorReduceByIterator(Mat &srcImage, Mat &destImage, int div){
    destImage = srcImage.clone();
    Mat_<Vec3b>::iterator itBegin = destImage.begin<Vec3b>();
    Mat_<Vec3b>::iterator itEnd = destImage.end<Vec3b>();

    while(itBegin != itEnd) {
        (*itBegin)[0] = (*itBegin)[0] / div *div + div/2;
        (*itBegin)[1] = (*itBegin)[1] / div *div + div/2;
        (*itBegin)[2] = (*itBegin)[2] / div *div + div/2;
        ++itBegin;
    }
}

void colorReduceByAt(Mat &srcImage, Mat &destImage, int div){
    destImage = srcImage.clone();
    int rows = destImage.rows;
    int cols = destImage.cols;

    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < cols; ++j) {
            destImage.at<Vec3b>(i, j)[0] = destImage.at<Vec3b>(i, j)[0] / div *div + div/2;
            destImage.at<Vec3b>(i, j)[1] = destImage.at<Vec3b>(i, j)[1] / div *div + div/2;
            destImage.at<Vec3b>(i, j)[2] = destImage.at<Vec3b>(i, j)[2] / div *div + div/2;
        }
    }
}

int main() {
    Mat srcImage = imread("../static/data/lena.jpg");
    namedWindow("srcImage", WINDOW_AUTOSIZE);
    imshow("srcImage", srcImage);

    Mat destImage1, destImage2, destImage3;
    colorReduceByPtr(srcImage, destImage1, 10);
    colorReduceByIterator(srcImage, destImage2, 40);
    colorReduceByAt(srcImage, destImage3, 80);
    namedWindow("destImage1", WINDOW_AUTOSIZE);
    namedWindow("destImage2", WINDOW_AUTOSIZE);
    namedWindow("destImage3", WINDOW_AUTOSIZE);
    imshow("destImage1", destImage1);
    imshow("destImage2", destImage2);
    imshow("destImage3", destImage3);
    waitKey(0);
    return 0;
}
