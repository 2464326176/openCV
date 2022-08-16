/*
 * @Author: yh
 * @Date: 2022/8/5 0:17
 * @Description: 
 * @FilePath: addImage.cpp
 */

#include <opencv2/opencv.hpp>

using namespace cv;

int main() {
    Mat windowsLogo = imread("../static/data/LinuxLogo.jpg");
    Mat linuxLogo = imread("../static/data/WindowsLogo.jpg");
    Mat dst;
    double alpha = 0.5;
    double beta = 1.0 - alpha;
    addWeighted(windowsLogo, alpha, linuxLogo, beta, 0.0, dst);

    imshow("dst", dst);
    waitKey(0);
    return 0;
}


