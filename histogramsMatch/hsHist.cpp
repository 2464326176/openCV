//********************
// Author:  yh 
// Time:    2021/5/7.
// 
//********************
//
#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main() {

    Mat src = imread("../static/data/lena.jpg");
    Mat dst;
    cvtColor(src, dst, COLOR_BGR2HSV);

    int hueBinNum = 30;
    int saturationBinNum = 32;
    int histSize[] = {hueBinNum, saturationBinNum};

    float hueRanges[] = {0, 180};
    float saturationRanges[] = {0, 256};
    const float *ranges[] = {hueRanges, saturationRanges};

    int channels[] = {0, 1};
    MatND dstHist;
    calcHist(&dst, 1, channels, Mat(), dstHist, 2, histSize, ranges, true, false);

    double maxValue = 0;
    minMaxLoc(dstHist, 0, &maxValue, 0, 0);
    int scale = 10;

    Mat histImg = Mat::zeros(saturationBinNum * scale, hueBinNum * scale, CV_8UC3);

    for (int hue = 0; hue < hueBinNum; hue++) {
        for (int saturation = 0; saturation < saturationBinNum; saturation++) {
            float binValue = dstHist.at<float>(hue, saturation);
            int intensity = cvRound(binValue * 255 / maxValue);
            rectangle(histImg, Point(hue * scale, saturation * scale),
                    Point((hue + 1) * scale - 1, (saturation + 1) * scale -1),
                    Scalar::all(intensity), FILLED);
        }
    }
    imshow("src", src);
    imshow("dst", histImg);
    waitKey();
    return 0;
}
