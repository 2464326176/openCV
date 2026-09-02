// LEARN: L5 PCA 闄嶇淮涓庝富鎴愬垎鍙鍖?// OFFICIAL: samples/cpp/pca.cpp, tutorial_code/ml/introduction_to_pca/introduction_to_pca.cpp
// THEORY: docs/ch05_ml.md 搂PCA
// TASK: 鍚堟垚鏃嬭浆楂樻柉鐐逛簯锛孭CA 姹備富鎴愬垎锛屽湪鏁ｇ偣鍥句笂鐢荤壒寰佸悜閲?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;

int main() {
    RNG rng(1);
    const int N = 500;
    Mat pts(N, 2, CV_32F);
    Point2f c(256, 256);
    for (int i = 0; i < N; ++i) {
        float x = (float)rng.gaussian(40);
        float y = (float)rng.gaussian(15);
        pts.at<float>(i,0) = c.x + 0.9f*x - 0.4f*y; // 鏃嬭浆
        pts.at<float>(i,1) = c.y + 0.4f*x + 0.9f*y;
    }

    PCA pca(pts, Mat(), PCA::DATA_AS_ROW, 2);
    Mat mean = pca.mean, ev = pca.eigenvectors;
    logInfo("mean = %.1f %.1f", mean.at<float>(0), mean.at<float>(1));
    logInfo("lambda0=%.1f lambda1=%.1f",
            pca.eigenvalues.at<float>(0), pca.eigenvalues.at<float>(1));

    Mat canvas(512, 512, CV_8UC3, Scalar(20,20,20));
    for (int i = 0; i < N; ++i)
        circle(canvas, Point((int)pts.at<float>(i,0),(int)pts.at<float>(i,1)), 1, Scalar(200,200,200), -1);

    Point mc((int)mean.at<float>(0), (int)mean.at<float>(1));
    for (int k = 0; k < 2; ++k) {
        float len = 4.0f * sqrtf(fabsf(pca.eigenvalues.at<float>(k)));
        Vec2f e(ev.at<float>(k,0), ev.at<float>(k,1));
        Point p2(mc.x + (int)(len*e[0]), mc.y + (int)(len*e[1]));
        arrowedLine(canvas, mc, p2, k==0 ? Scalar(0,0,255) : Scalar(0,255,0), 2);
    }
    dbgShow("L5_04 PCA", canvas, 0);
    return 0;
}
