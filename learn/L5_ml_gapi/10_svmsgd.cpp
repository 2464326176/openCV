// LEARN: L5 SVMSGD
// OFFICIAL: samples/cpp/train_svmsgd.cpp
// THEORY: docs/ch05_ml.md §SVMSGD
// TASK: Generate two-class points, train SVMSGD and visualize decision boundary
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    RNG rng(11);
    const int N = 300;
    Mat s(N, 2, CV_32F), l(N, 1, CV_32S);
    for (int i = 0; i < N; ++i) {
        int cls = i % 2;
        s.at<float>(i,0) = cls ? rng.uniform(260,460) : rng.uniform(50,250);
        s.at<float>(i,1) = rng.uniform(50,460);
        l.at<int>(i) = cls ? 1 : -1;
    }

    Ptr<SVMSGD> sgd = SVMSGD::create();
    sgd->setSvmsgdType(SVMSGD::ASGD);
    Ptr<TrainData> td = TrainData::create(s, ROW_SAMPLE, l);
    sgd->train(td);

    const int w = 512, h = 512;
    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            Mat m = (Mat_<float>(1,2) << j, i);
            canvas.at<Vec3b>(i,j) = sgd->predict(m) > 0 ? Vec3b(0,80,0) : Vec3b(80,0,0);
        }
    for (int i = 0; i < N; ++i)
        circle(canvas, Point((int)s.at<float>(i,0),(int)s.at<float>(i,1)), 2,
               l.at<int>(i) > 0 ? Scalar(255,255,255) : Scalar(255,0,0), -1);

    dbgShow("L5_10 SVMSGD", canvas, 0);
    return 0;
}
