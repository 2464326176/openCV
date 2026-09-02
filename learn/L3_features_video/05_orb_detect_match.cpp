// LEARN: L3 ORB Detection and Matching
// OFFICIAL: samples/cpp/tutorial_code/features2d/AKAZE_match.cpp、matchmethod_orb_akaze_brisk.cpp
// THEORY: docs/ch03_features.md §descriptors
// TASK: ORB detectAndCompute two images, BFMatcher(NORM_HAMMING)+Lowe ratio test matching
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img1 = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (img1.empty()) { logInfo("imread failed"); return -1; }
    // use affine perturbation as second image (guarantees real correspondence)
    Mat M = getRotationMatrix2D(Point2f(img1.cols / 2.f, img1.rows / 2.f), 15, 0.9);
    Mat img2; warpAffine(img1, img2, M, img1.size());

    Ptr<ORB> orb = ORB::create(1000);
    std::vector<KeyPoint> kp1, kp2;
    Mat d1, d2;
    orb->detectAndCompute(img1, noArray(), kp1, d1);
    orb->detectAndCompute(img2, noArray(), kp2, d2);

    BFMatcher bf(NORM_HAMMING);
    std::vector<std::vector<DMatch>> knn;
    bf.knnMatch(d1, d2, knn, 2);

    std::vector<DMatch> good;
    for (auto& m : knn) {
        if (m.size() < 2) continue;
        if (m[0].distance < 0.75f * m[1].distance) good.push_back(m[0]);
    }
    Mat out;
    drawMatches(img1, kp1, img2, kp2, good, out,
                Scalar(0, 255, 0), Scalar(0, 0, 255));
    logInfo("orb kp1=%zu kp2=%zu good=%zu", kp1.size(), kp2.size(), good.size());
    dbgShow("L3_05 orb match", out, 0);
    return 0;
}

