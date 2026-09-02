// LEARN: L3 Homography Matrix RANSAC Alignment
// OFFICIAL: samples/cpp/tutorial_code/features2d/feature_homography/SURF_FLANN_matching_homography_Demo.cpp、perspective_correction.cpp
// THEORY: docs/ch03_features.md §homography
// TASK: ORB matching, findHomography(RANSAC) compute homography; warpPerspective align img2 to img1
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img1 = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (img1.empty()) { logInfo("imread failed"); return -1; }

    // perspective perturbation as img2
    std::vector<Point2f> srcPts = {
        {0, 0}, {(float)img1.cols, 0},
        {(float)img1.cols, (float)img1.rows}, {0, (float)img1.rows}};
    std::vector<Point2f> dstPts = {
        {10, 10}, {(float)img1.cols - 20, 15},
        {(float)img1.cols - 30, (float)img1.rows - 10}, {20, (float)img1.rows - 20}};
    Mat H0 = getPerspectiveTransform(srcPts, dstPts);
    Mat img2; warpPerspective(img1, img2, H0, img1.size());

    Ptr<ORB> orb = ORB::create(1500);
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
    if (good.size() < 4) { logInfo("not enough matches: %zu", good.size()); return -1; }

    std::vector<Point2f> obj, scene;
    for (auto& m : good) {
        obj.push_back(kp1[m.queryIdx].pt);
        scene.push_back(kp2[m.trainIdx].pt);
    }
    Mat mask;
    Mat H = findHomography(scene, obj, RANSAC, 5.0, mask);
    Mat aligned; warpPerspective(img2, aligned, H, img1.size());

    logInfo("good=%zu inliers=%d", good.size(), (int)sum(mask)[0]);
    dbgShowMany({"L3_09 img1", "L3_09 img2", "L3_09 aligned"},
                {img1, img2, aligned}, 0);
    return 0;
}

