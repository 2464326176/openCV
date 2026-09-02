// LEARN: L5 HOG feature and SVM training relationship
// OFFICIAL: samples/cpp/train_HOG.cpp, digits_svm.cpp
// THEORY: docs/ch05_ml.md §5.8.1
// TASK: Extract HOG from synthetic positive/negative samples and train linear SVM; larger C means harder margin
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include <opencv_utils.h>

using namespace cv;
using namespace cv::ml;

static Mat makeSample(bool positive) {
    Mat img(64, 64, CV_8UC1, Scalar(30));
    if (positive) rectangle(img, Rect(16, 16, 32, 32), Scalar(220), -1);
    else circle(img, Point(32, 32), 10, Scalar(200), -1);
    return img;
}

int main() {
    HOGDescriptor hog(Size(64, 64), Size(16, 16), Size(8, 8), Size(8, 8), 9);
    std::vector<float> desc;
    hog.compute(makeSample(true), desc);
    const int descSize = (int)desc.size();
    Mat samples(20, descSize, CV_32F);
    Mat labels(20, 1, CV_32S);
    for (int i = 0; i < 10; ++i) {
        hog.compute(makeSample(true), desc);
        Mat(1, descSize, CV_32F, desc.data()).copyTo(samples.row(i));
        labels.at<int>(i) = 1;
        hog.compute(makeSample(false), desc);
        Mat(1, descSize, CV_32F, desc.data()).copyTo(samples.row(i + 10));
        labels.at<int>(i + 10) = -1;
    }

    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);
    svm->setKernel(SVM::LINEAR);
    svm->setC(1.0);
    svm->train(samples, ROW_SAMPLE, labels);
    float err = svm->calcError(TrainData::create(samples, ROW_SAMPLE, labels), false, noArray());
    logInfo("linear SVM train error=%.3f; C=1.0 larger C means less tolerance for error", err);
    logInfo("pipeline: img -> HOGDescriptor -> CV_32F rows -> SVM::train");
    return 0;
}
