// LEARN: L5 LogisticRegression 閫昏緫鍥炲綊
// OFFICIAL: samples/cpp/logistic_regression.cpp
// THEORY: docs/ch05_ml.md 搂Logistic
// TASK: 鍚堟垚涓ょ被鐐癸紝閫昏緫鍥炲綊璁粌骞跺彲瑙嗗寲鍐崇瓥杈圭晫
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    RNG rng(5);
    const int N = 300;
    Mat s(N, 2, CV_32F), l(N, 1, CV_32S);
    for (int i = 0; i < N; ++i) {
        int cls = i % 2;
        s.at<float>(i,0) = cls ? rng.uniform(250,450) : rng.uniform(50,250);
        s.at<float>(i,1) = rng.uniform(50,450);
        l.at<int>(i) = cls;
    }

    Ptr<LogisticRegression> lr = LogisticRegression::create();
    lr->setLearningRate(0.001);
    lr->setIterations(1000);
    lr->setRegularization(LogisticRegression::REG_L2);
    lr->setTrainMethod(LogisticRegression::BATCH);
    lr->setMiniBatchSize(1);
    lr->train(s, ROW_SAMPLE, l);

    const int w = 512, h = 512;
    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            Mat m = (Mat_<float>(1,2) << j, i);
            float r = lr->predict(m);
            canvas.at<Vec3b>(i,j) = r > 0.5f ? Vec3b(0,80,0) : Vec3b(80,0,0);
        }
    for (int i = 0; i < N; ++i)
        circle(canvas, Point((int)s.at<float>(i,0),(int)s.at<float>(i,1)), 2,
               l.at<int>(i) ? Scalar(255,255,255) : Scalar(255,0,0), -1);

    dbgShow("L5_09 Logistic", canvas, 0);
    return 0;
}
