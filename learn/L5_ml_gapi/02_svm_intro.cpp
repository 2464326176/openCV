// LEARN: L5 SVM 绾挎€у垎绫?+ 鍐崇瓥杈圭晫鍙鍖?// OFFICIAL: samples/cpp/tutorial_code/ml/introduction_to_svm/introduction_to_svm.cpp
// THEORY: docs/ch05_ml.md 搂SVM
// TASK: 鍚堟垚涓ょ被浜岀淮鐐癸紝璁粌绾挎€?SVM锛屽彲瑙嗗寲鏀寔鍚戦噺涓庡喅绛栬竟鐣?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    const int w = 512, h = 512;
    RNG rng(12345);
    Mat samples(20, 2, CV_32F), labels(20, 1, CV_32S);
    for (int i = 0; i < 10; ++i) {
        samples.at<float>(i,0) = rng.uniform(50, 200);
        samples.at<float>(i,1) = rng.uniform(50, 200);
        labels.at<int>(i) = 1;
    }
    for (int i = 10; i < 20; ++i) {
        samples.at<float>(i,0) = rng.uniform(300, 460);
        samples.at<float>(i,1) = rng.uniform(300, 460);
        labels.at<int>(i) = -1;
    }

    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);
    svm->setKernel(SVM::LINEAR);
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));
    svm->train(samples, ROW_SAMPLE, labels);

    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    Vec3b green(0,80,0), blue(80,0,0);
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            Mat m = (Mat_<float>(1,2) << j, i);
            canvas.at<Vec3b>(i,j) = svm->predict(m) > 0 ? green : blue;
        }
    for (int i = 0; i < 20; ++i)
        circle(canvas, Point((int)samples.at<float>(i,0),(int)samples.at<float>(i,1)), 3,
               labels.at<int>(i) > 0 ? Scalar(255,255,255) : Scalar(0,0,255), -1);

    Mat sv = svm->getSupportVectors();
    for (int i = 0; i < sv.rows; ++i)
        circle(canvas, Point((int)sv.at<float>(i,0),(int)sv.at<float>(i,1)), 6, Scalar(0,255,255), 2);

    dbgShow("L5_02 SVM linear", canvas, 0);
    return 0;
}
