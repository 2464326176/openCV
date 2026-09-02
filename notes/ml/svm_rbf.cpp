//********************
// Author:  yh
// Non-linear SVM (RBF kernel):
//  Data includes a linearly non-separable part (left/right blocks + middle mixed band),
//  handled by the Gaussian RBF kernel
//  Principle: the kernel implicitly maps samples into a high-dimensional space where
//             they become separable
//  Official example: tutorial_code/ml/non_linear_svms/non_linear_svms.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include <iostream>

using namespace cv;
using namespace cv::ml;
using namespace std;

int main() {
    const int N = 100;               // samples per class
    const float LIN = 0.9f;          // fraction of linearly separable part
    const int W = 512, H = 512;

    // 1. Generate noisy training data
    Mat trainData(2 * N, 2, CV_32F);
    Mat labels(2 * N, 1, CV_32S);
    RNG rng(100);
    int nLin = (int)(LIN * N);

    // Class 1 linear part: x ∈ [0, 0.4W)
    rng.fill(trainData.rowRange(0, nLin).colRange(0, 1), RNG::UNIFORM, 0, 0.4f * W);
    rng.fill(trainData.rowRange(0, nLin).colRange(1, 2), RNG::UNIFORM, 0, (float)H);
    // Class 2 linear part: x ∈ [0.6W, W)
    rng.fill(trainData.rowRange(2 * N - nLin, 2 * N).colRange(0, 1), RNG::UNIFORM, 0.6f * W, (float)W);
    rng.fill(trainData.rowRange(2 * N - nLin, 2 * N).colRange(1, 2), RNG::UNIFORM, 0, (float)H);
    // Middle mixed band x ∈ [0.4W, 0.6W): both classes randomly mixed (the non-separable part)
    rng.fill(trainData.rowRange(nLin, 2 * N - nLin).colRange(0, 1), RNG::UNIFORM, 0.4f * W, 0.6f * W);
    rng.fill(trainData.rowRange(nLin, 2 * N - nLin).colRange(1, 2), RNG::UNIFORM, 0, (float)H);

    labels.rowRange(0, N).setTo(1);
    labels.rowRange(N, 2 * N).setTo(2);

    // 2. Configure SVM: RBF Gaussian kernel
    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);
    svm->setC(10.0);                        // penalty coefficient; larger = tighter fit
    svm->setKernel(SVM::RBF);               // Gaussian kernel K(x,y)=exp(-γ||x-y||²)
    svm->setGamma(0.5);                     // kernel width parameter γ
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100000, 1e-6));

    svm->train(trainData, ROW_SAMPLE, labels);

    // 3. Draw the decision region
    Mat I = Mat::zeros(H, W, CV_8UC3);
    Vec3b green(0, 100, 0), blue(100, 0, 0);
    for (int i = 0; i < I.rows; i++)
        for (int j = 0; j < I.cols; j++) {
            Mat sample = (Mat_<float>(1, 2) << j, i);
            I.at<Vec3b>(i, j) = (svm->predict(sample) == 1) ? green : blue;
        }

    // 4. Overlay training samples
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
