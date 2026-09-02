// LEARN: L3 AKAZE 检测与匹配
// OFFICIAL: samples/cpp/tutorial_code/features2d/AKAZE_match.cpp、planar_tracking.cpp
// THEORY: docs/ch03_features.md §描述子
// TASK: AKAZE detectAndCompute, BFMatcher(NORM_HAMMING)+Lowe ratio test
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img1 = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (img1.empty()) { logInfo("imread failed"); return -1; }
    Mat M = getRotationMatrix2D(Point2f(img1.cols / 2.f, img1.rows / 2.f), 20, 0.85);
    Mat img2; warpAffine(img1, img2, M, img1.size());

    Ptr<AKAZE> ak = AKAZE::create();
    std::vector<KeyPoint> kp1, kp2;
    Mat d1, d2;
    ak->detectAndCompute(img1, noArray(), kp1, d1);
    ak->detectAndCompute(img2, noArray(), kp2, d2);

    BFMatcher bf(NORM_HAMMING);
    std::vector<std::vector<DMatch>> knn;
    bf.knnMatch(d1, d2, knn, 2);

    std::vector<DMatch> good;
    for (auto& m : knn) {
        if (m.size() < 2) continue;
        if (m[0].distance < 0.7f * m[1].distance) good.push_back(m[0]);
    }
    Mat out;
    drawMatches(img1, kp1, img2, kp2, good, out,
                Scalar(0, 255, 0), Scalar(0, 0, 255));
    logInfo("akaze kp1=%zu kp2=%zu good=%zu", kp1.size(), kp2.size(), good.size());
    dbgShow("L3_06 akaze match", out, 0);
    return 0;
}

