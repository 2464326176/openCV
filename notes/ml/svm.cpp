//********************
// Author:  yh
// Linear SVM binary classification:
//  4 training samples (1 positive, 3 negative); visualize the whole plane with the
//  decision boundary after training
//  Principle: SVM finds the maximum-margin separating hyperplane; the boundary is
//             determined only by a few support vectors
//  Official example: tutorial_code/ml/introduction_to_svm/introduction_to_svm.cpp
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include <iostream>

using namespace cv;
using namespace cv::ml;
using namespace std;

int main(int, char **) {
    // 1. Training data: 4 2D points, labels 1 positive / 3 negative
    int labels[4] = {1, -1, -1, -1};
    float trainingData[4][2] = {{501, 10}, {255, 10}, {501, 255}, {10, 501}};

    Mat trainData(4, 2, CV_32F, trainingData);
    Mat labelsMat(4, 1, CV_32SC1, labels);   // SVM labels use CV_32S

    // 2. Create and configure SVM
    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);          // C-SVC: classifier with penalty term
    svm->setKernel(SVM::LINEAR);       // linear kernel: K(x,y)=x·y
    svm->setC(1.0);                    // regularization parameter: larger = harder classification (tolerates fewer misclassifications)
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

    // 3. Train: ROW_SAMPLE means each row is one sample
    svm->train(trainData, ROW_SAMPLE, labelsMat);

    // 4. Predict per pixel and draw the decision region of the whole plane
    int width = 512, height = 512;
    Mat image = Mat::zeros(height, width, CV_8UC3);
    Vec3b green(0, 255, 0), blue(255, 0, 0);
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            Mat sample = (Mat_<float>(1, 2) << j, i);
            float response = svm->predict(sample);   // returns the class
            image.at<Vec3b>(i, j) = (response == 1) ? green : blue;
        }
    }

    // 5. Overlay training samples (black = positive, white = negative)
    circle(image, Point(501, 10), 5, Scalar(0, 0, 0), -1);
    circle(image, Point(255, 10), 5, Scalar(255, 255, 255), -1);
    circle(image, Point(501, 255), 5, Scalar(255, 255, 255), -1);
    circle(image, Point(10, 501), 5, Scalar(255, 255, 255), -1);

    // 6. Highlight support vectors (gray ring): only they decide the boundary
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
