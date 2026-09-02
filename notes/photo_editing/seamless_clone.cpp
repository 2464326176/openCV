//********************
// Author:  yh
// 无缝克隆（Poisson 图像编辑）：
//  原理：把源图目标区域的"梯度场"泊松融合进目标图，使接缝处过渡自然
//  对应官方示例: tutorial_code/photo/seamless_cloning/cloning_demo.cpp
//  使用：seamless_clone <源图> <目标图> <掩膜> [模式]
//        模式: 1=普通克隆 2=混合克隆 3=单色迁移
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

    int flag = argc > 4 ? atoi(argv[4]) : 1;   // 1=普通 2=混合 3=单色
    // 把源图中心对齐到目标图中心作为粘贴位置
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
