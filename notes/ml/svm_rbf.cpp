//********************
// Author:  yh
// 非线性 SVM（RBF 核）：
//  数据含线性不可分部分（左右两块 + 中间混合带），用高斯核 RBF 处理
//  原理：核函数把样本隐式映射到高维空间，使其在高维可分
//  对应官方示例: tutorial_code/ml/non_linear_svms/non_linear_svms.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include <iostream>

using namespace cv;
using namespace cv::ml;
using namespace std;

int main() {
    const int N = 100;               // 每类样本数
    const float LIN = 0.9f;          // 线性可分部分占比
    const int W = 512, H = 512;

    // 1. 生成带噪声的训练数据
    Mat trainData(2 * N, 2, CV_32F);
    Mat labels(2 * N, 1, CV_32S);
    RNG rng(100);
    int nLin = (int)(LIN * N);

    // 类1 线性部分：x ∈ [0, 0.4W)
    rng.fill(trainData.rowRange(0, nLin).colRange(0, 1), RNG::UNIFORM, 0, 0.4f * W);
    rng.fill(trainData.rowRange(0, nLin).colRange(1, 2), RNG::UNIFORM, 0, (float)H);
    // 类2 线性部分：x ∈ [0.6W, W)
    rng.fill(trainData.rowRange(2 * N - nLin, 2 * N).colRange(0, 1), RNG::UNIFORM, 0.6f * W, (float)W);
    rng.fill(trainData.rowRange(2 * N - nLin, 2 * N).colRange(1, 2), RNG::UNIFORM, 0, (float)H);
    // 中间混合带 x ∈ [0.4W, 0.6W)：两类随机混合（线性不可分部分）
    rng.fill(trainData.rowRange(nLin, 2 * N - nLin).colRange(0, 1), RNG::UNIFORM, 0.4f * W, 0.6f * W);
    rng.fill(trainData.rowRange(nLin, 2 * N - nLin).colRange(1, 2), RNG::UNIFORM, 0, (float)H);

    labels.rowRange(0, N).setTo(1);
    labels.rowRange(N, 2 * N).setTo(2);

    // 2. 配置 SVM：RBF 高斯核
    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);
    svm->setC(10.0);                        // 惩罚系数，越大拟合越紧
    svm->setKernel(SVM::RBF);               // 高斯核 K(x,y)=exp(-γ||x-y||²)
    svm->setGamma(0.5);                     // 核宽度参数 γ
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100000, 1e-6));

    svm->train(trainData, ROW_SAMPLE, labels);

    // 3. 画出决策区域
    Mat I = Mat::zeros(H, W, CV_8UC3);
    Vec3b green(0, 100, 0), blue(100, 0, 0);
    for (int i = 0; i < I.rows; i++)
        for (int j = 0; j < I.cols; j++) {
            Mat sample = (Mat_<float>(1, 2) << j, i);
            I.at<Vec3b>(i, j) = (svm->predict(sample) == 1) ? green : blue;
        }

    // 4. 叠加训练样本
    for (int k = 0; k < 2 * N; k++) {
        const float *p = trainData.ptr<float>(k);
        Scalar c = (labels.at<int>(k) == 1) ? Scalar(0, 255, 0) : Scalar(255, 0, 0);
        circle(I, Point((int)p[0], (int)p[1]), 3, c, -1);
    }
    imshow("SVM RBF (non-linear)", I);
    imwrite("../out/svm_rbf.png", I);
    waitKey(0);
    return 0;
}
