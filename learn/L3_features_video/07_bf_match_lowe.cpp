// LEARN: L3 BFMatcher + Lowe 姣斿€兼楠?// OFFICIAL: samples/cpp/tutorial_code/features2d/feature_description/SURF_matching_Demo.cpp
// THEORY: docs/ch03_features.md 搂鍖归厤
// TASK: ORB 鎻忚堪瀛愶紝knnMatch k=2锛屾瘮鍊?trackbar 鎺у埗绛涢€夛紝鐢?good matches
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat img1, img2, d1, d2, out;
static std::vector<KeyPoint> kp1, kp2;
static std::vector<std::vector<DMatch>> knn;
static int ratio_x100 = 75;

static void onTrack(int, void*) {
    std::vector<DMatch> good;
    float r = ratio_x100 / 100.0f;
    for (auto& m : knn) {
        if (m.size() < 2) continue;
        if (m[0].distance < r * m[1].distance) good.push_back(m[0]);
    }
    drawMatches(img1, kp1, img2, kp2, good, out,
                Scalar(0, 255, 0), Scalar(0, 0, 255));
    logInfo("ratio=%.2f good=%zu", r, good.size());
    imshow("L3_07 lowe", out);
}

int main() {
    img1 = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (img1.empty()) { logInfo("imread failed"); return -1; }
    Mat M = getRotationMatrix2D(Point2f(img1.cols / 2.f, img1.rows / 2.f), 25, 0.8);
    warpAffine(img1, img2, M, img1.size());

    Ptr<ORB> orb = ORB::create(1000);
    orb->detectAndCompute(img1, noArray(), kp1, d1);
    orb->detectAndCompute(img2, noArray(), kp2, d2);

    BFMatcher bf(NORM_HAMMING);
    bf.knnMatch(d1, d2, knn, 2);

    namedWindow("L3_07 lowe", WINDOW_AUTOSIZE);
    createTrackbar("Ratio*0.01", "L3_07 lowe", &ratio_x100, 100, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
