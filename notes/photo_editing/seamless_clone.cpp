//********************
// Author:  yh
// Seamless cloning (Poisson image editing):
//  Principle: Poisson-fuse the gradient field of the source region into the target image for a natural seam
//  Official example: tutorial_code/photo/seamless_cloning/cloning_demo.cpp
//  Usage: seamless_clone <source> <target> <mask> [mode]
//        Mode: 1=normal clone 2=mixed clone 3=monochrome transfer
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "用法: seamless_clone <源图> <目标图> <掩膜> [模式 1|2|3 默认1]" << endl;
        return -1;
    }

    Mat source = imread(argv[1], IMREAD_COLOR);
    Mat target = imread(argv[2], IMREAD_COLOR);
    Mat mask   = imread(argv[3], IMREAD_COLOR);
    if (source.empty() || target.empty() || mask.empty()) {
        cout << "图像加载失败" << endl;
        return -1;
    }

    int flag = argc > 4 ? atoi(argv[4]) : 1;   // 1=normal 2=mixed 3=monochrome
    // Align the source center to the target center as the paste position
    Point p(target.cols / 2, target.rows / 2);

    Mat result;
    seamlessClone(source, target, mask, p, result, flag);

    imshow("source", source);
    imshow("target", target);
    imshow("result", result);
    imwrite("../out/seamless_clone.png", result);
    waitKey(0);
    return 0;
}
