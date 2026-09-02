// LEARN: L5 SVM nonlinear RBF kernel
// OFFICIAL: samples/cpp/tutorial_code/ml/non_linear_svms/non_linear_svms.cpp
// THEORY: docs/ch05_ml.md §SVM kernel trick
// TASK: Generate ring+center two-class points, RBF kernel SVM draws nonlinear decision boundary
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    const int w = 512, h = 512;
    RNG rng(42);
    const int N = 200;
    Mat samples(N, 2, CV_32F), labels(N, 1, CV_32S);
    Point2f c(w/2.f, h/2.f);
    for (int i = 0; i < N; ++i) {
        float r = i < N/2 ? rng.uniform(10.f, 60.f) : rng.uniform(120.f, 180.f);
        float a = rng.uniform(0.f, (float)CV_PI*2);
        samples.at<float>(i,0) = c.x + r * cos(a);
        samples.at<float>(i,1) = c.y + r * sin(a);
        labels.at<int>(i) = i < N/2 ? 1 : -1;
    }

    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);
    svm->setKernel(SVM::RBF);
    svm->setGamma(0.001);
    svm->setC(10);
    svm->train(samples, ROW_SAMPLE, labels);

    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            Mat m = (Mat_<float>(1,2) << j, i);
            canvas.at<Vec3b>(i,j) = svm->predict(m) > 0 ? Vec3b(0,80,0) : Vec3b(80,0,0);
        }
    for (int i = 0; i < N; ++i)
        circle(canvas, Point((int)samples.at<float>(i,0),(int)samples.at<float>(i,1)), 3,
               labels.at<int>(i) > 0 ? Scalar(255,255,255) : Scalar(255,0,0), -1);

    dbgShow("L5_03 SVM RBF", canvas, 0);
    return 0;
}
