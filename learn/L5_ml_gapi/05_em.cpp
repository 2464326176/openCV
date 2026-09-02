// LEARN: L5 EM Gaussian mixture
// OFFICIAL: samples/cpp/em.cpp
// THEORY: docs/ch05_ml.md §EM
// TASK: Generate three Gaussian clusters, EM fits 3 Gaussians, color by posterior clustering
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    const int w = 512, h = 512;
    RNG rng(7);
    const int N = 600;
    Mat samples(N, 2, CV_32F);
    Point2f cen[3] = {{150,150},{380,180},{260,360}};
    for (int i = 0; i < N; ++i) {
        int g = i % 3;
        samples.at<float>(i,0) = cen[g].x + (float)rng.gaussian(30);
        samples.at<float>(i,1) = cen[g].y + (float)rng.gaussian(30);
    }

    Ptr<EM> em = EM::create();
    em->setClustersNumber(3);
    em->setCovarianceMatrixType(EM::COV_MAT_SPHERICAL);
    em->trainEM(samples);

    Mat means = em->getMeans(); // nclusters x dim, CV_64F
    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    Vec3b color[3] = {{0,0,255},{0,255,0},{255,0,0}};
    for (int i = 0; i < N; ++i) {
        double best = 1e18; int g = 0;
        for (int k = 0; k < means.rows; ++k) {
            double dx = samples.at<float>(i,0) - means.at<double>(k,0);
            double dy = samples.at<float>(i,1) - means.at<double>(k,1);
            double d = dx*dx + dy*dy;
            if (d < best) { best = d; g = k; }
        }
        circle(canvas, Point((int)samples.at<float>(i,0),(int)samples.at<float>(i,1)), 2,
               color[g % 3], -1);
    }
    dbgShow("L5_05 EM", canvas, 0);
    return 0;
}
