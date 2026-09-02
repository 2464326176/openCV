//********************
// Author:  yh
// 线性 SVM 二分类：
//  4 个训练样本（1 正 3 负），训练后用决策边界可视化整个平面
//  原理：SVM 寻找"间隔最大"的分隔超平面，分类面仅由少数"支持向量"决定
//  对应官方示例: tutorial_code/ml/introduction_to_svm/introduction_to_svm.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include <iostream>

using namespace cv;
using namespace cv::ml;
using namespace std;

int main(int, char **) {
    // 1. 训练数据：4 个 2D 点，标签 1 正 3 负
    int labels[4] = {1, -1, -1, -1};
    float trainingData[4][2] = {{501, 10}, {255, 10}, {501, 255}, {10, 501}};

    Mat trainData(4, 2, CV_32F, trainingData);
    Mat labelsMat(4, 1, CV_32SC1, labels);   // SVM 标签用 CV_32S

    // 2. 创建并配置 SVM
    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);          // C-SVC：带惩罚项的分类器
    svm->setKernel(SVM::LINEAR);       // 线性核：K(x,y)=x·y
    svm->setC(1.0);                    // 正则化参数：越大越硬分类（容忍更少误分）
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

    // 3. 训练：ROW_SAMPLE 表示每行是一个样本
    svm->train(trainData, ROW_SAMPLE, labelsMat);

    // 4. 逐像素预测，画出整个平面的决策区域
    int width = 512, height = 512;
    Mat image = Mat::zeros(height, width, CV_8UC3);
    Vec3b green(0, 255, 0), blue(255, 0, 0);
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            Mat sample = (Mat_<float>(1, 2) << j, i);
            float response = svm->predict(sample);   // 返回类别
            image.at<Vec3b>(i, j) = (response == 1) ? green : blue;
        }
    }

    // 5. 叠加训练样本（黑色=正样本，白色=负样本）
    circle(image, Point(501, 10), 5, Scalar(0, 0, 0), -1);
    circle(image, Point(255, 10), 5, Scalar(255, 255, 255), -1);
    circle(image, Point(501, 255), 5, Scalar(255, 255, 255), -1);
    circle(image, Point(10, 501), 5, Scalar(255, 255, 255), -1);

    // 6. 高亮支持向量（灰色圆环）：只有它们决定分类面
    Mat sv = svm->getUncompressedSupportVectors();
    for (int i = 0; i < sv.rows; i++) {
        const float *v = sv.ptr<float>(i);
        circle(image, Point((int)v[0], (int)v[1]), 6, Scalar(128, 128, 128), 2);
    }

    imshow("SVM Linear", image);
    imwrite("../out/svm_linear.png", image);
    waitKey(0);
    return 0;
}
