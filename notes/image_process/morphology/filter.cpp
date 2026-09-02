//********************
// Author:  yh
// Time:    2022/8/4.
//  五种滤波对比：方框 / 均值 / 高斯 / 中值 / 双边
//  - 线性滤波：boxFilter, blur, GaussianBlur
//  - 非线性滤波：medianBlur (去椒盐), bilateralFilter (保边)
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main() {
    Mat src = imread("../data/images/OIP.png");
    if (src.empty()) {
        cout << "could not load image.." << endl;
        return -1;
    }
    imshow("original image", src);

    // 1. 方框滤波：核内系数相等，normalize=true 时等价均值滤波
    Mat boxDst;
    boxFilter(src, boxDst, -1, Size(5, 5));
    imshow("box filter", boxDst);

    // 2. 均值滤波：归一化方框，简单快速但模糊边缘
    Mat meanDst;
    blur(src, meanDst, Size(5, 5), Point(-1, -1));
    imshow("mean filter", meanDst);

    // 3. 高斯滤波：中心权重最大，保边性优于均值，Canny 标准预处理
    Mat gaussDst;
    GaussianBlur(src, gaussDst, Size(5, 5), 0, 0);
    imshow("gaussian filter", gaussDst);

    // 4. 中值滤波：取邻域中位数，椒盐噪声克星
    Mat medianDst;
    medianBlur(src, medianDst, 5);
    imshow("median filter", medianDst);

    // 5. 双边滤波：空间 + 值域双重加权，去噪同时保住边缘
    Mat bilateralDst;
    bilateralFilter(src, bilateralDst, 5, 10.0, 2.5);
    imshow("bilateral filter", bilateralDst);

    waitKey(0);
    return 0;
}
