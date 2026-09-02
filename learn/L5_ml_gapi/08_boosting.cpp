// LEARN: L5 Boost 鎻愬崌
// OFFICIAL: samples/cpp/letter_recog.cpp (鍊?Boost API)
// THEORY: docs/ch05_ml.md 搂Boost
// TASK: 鍚堟垚涓ょ被鐐癸紝Boost 璁粌骞跺彲瑙嗗寲鍐崇瓥杈圭晫
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    RNG rng(9);
    const int N = 300;
    Mat s(N, 2, CV_32F), l(N, 1, CV_32S);
    for (int i = 0; i < N; ++i) {
        int cls = i % 2;
        float r = (float)rng.gaussian(50);
        s.at<float>(i,0) = 256 + (cls ? 60 : -60) + r;
        s.at<float>(i,1) = 256 + (float)rng.gaussian(60);
        l.at<int>(i) = cls;
    }

    Ptr<Boost> b = Boost::create();
    b->setBoostType(Boost::REAL);
    b->setWeakCount(50);
    b->setWeightTrimRate(0.95);
    b->setMaxDepth(3);
    b->train(s, ROW_SAMPLE, l);

    const int w = 512, h = 512;
    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            Mat m = (Mat_<float>(1,2) << j, i);
            canvas.at<Vec3b>(i,j) = b->predict(m) > 0 ? Vec3b(0,80,0) : Vec3b(80,0,0);
        }
    for (int i = 0; i < N; ++i)
        circle(canvas, Point((int)s.at<float>(i,0),(int)s.at<float>(i,1)), 2,
               l.at<int>(i) ? Scalar(255,255,255) : Scalar(255,0,0), -1);

    dbgShow("L5_08 Boost", canvas, 0);
    return 0;
}
