// LEARN: L5 DTrees decision tree
// OFFICIAL: samples/cpp/tree_engine.cpp
// THEORY: docs/ch05_ml.md §DTrees
// TASK: Generate two-class 2D points, train DTrees and visualize decision boundary
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    RNG rng(3);
    const int N = 200;
    Mat s(N, 2, CV_32F), l(N, 1, CV_32S);
    for (int i = 0; i < N; ++i) {
        int cls = i < N/2 ? 0 : 1;
        s.at<float>(i,0) = cls ? rng.uniform(200,400) : rng.uniform(50,250);
        s.at<float>(i,1) = rng.uniform(50,400);
        l.at<int>(i) = cls;
    }

    Ptr<DTrees> dt = DTrees::create();
    dt->setMaxDepth(8);
    dt->setMinSampleCount(2);
    dt->setRegressionAccuracy(0);
    dt->setMaxCategories(10);
    dt->train(s, ROW_SAMPLE, l);

    const int w = 450, h = 450;
    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            Mat m = (Mat_<float>(1,2) << j, i);
            canvas.at<Vec3b>(i,j) = dt->predict(m) > 0 ? Vec3b(0,80,0) : Vec3b(80,0,0);
        }
    for (int i = 0; i < N; ++i)
        circle(canvas, Point((int)s.at<float>(i,0),(int)s.at<float>(i,1)), 2,
               l.at<int>(i) ? Scalar(255,255,255) : Scalar(255,0,0), -1);

    dbgShow("L5_07 DTrees", canvas, 0);
    return 0;
}
