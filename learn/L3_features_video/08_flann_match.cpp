// LEARN: L3 FlannBasedMatcher 鍖归厤
// OFFICIAL: samples/cpp/tutorial_code/features2d/feature_flann_matcher/SURF_FLNN_matching_Demo.cpp
// THEORY: docs/ch03_features.md 搂鍖归厤
// TASK: ORB 浜岃繘鍒舵弿杩板瓙閰?LshIndexParams 鐨?FlannBasedMatcher锛宬nnMatch + Lowe
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img1 = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (img1.empty()) { logInfo("imread failed"); return -1; }
    Mat M = getRotationMatrix2D(Point2f(img1.cols / 2.f, img1.rows / 2.f), 18, 0.9);
    Mat img2; warpAffine(img1, img2, M, img1.size());

    Ptr<ORB> orb = ORB::create(1000);
    std::vector<KeyPoint> kp1, kp2;
    Mat d1, d2;
    orb->detectAndCompute(img1, noArray(), kp1, d1);
    orb->detectAndCompute(img2, noArray(), kp2, d2);

    // ORB 鎻忚堪瀛愪负浜岃繘鍒讹紝FLANN 蹇呴』鐢?LshIndexParams
    Ptr<flann::IndexParams> params = makePtr<flann::LshIndexParams>(6, 12, 1);
    FlannBasedMatcher flann(params);
    std::vector<std::vector<DMatch>> knn;
    flann.knnMatch(d1, d2, knn, 2);

    std::vector<DMatch> good;
    for (auto& m : knn) {
        if (m.size() < 2) continue;
        if (m[0].distance < 0.7f * m[1].distance) good.push_back(m[0]);
    }
    Mat out;
    drawMatches(img1, kp1, img2, kp2, good, out,
                Scalar(0, 255, 0), Scalar(0, 0, 255));
    logInfo("flann-lsh good=%zu", good.size());
    dbgShow("L3_08 flann match", out, 0);
    return 0;
}
