// LEARN: L3 Homography Decomposition and Pose
// OFFICIAL: tutorial_code/features2d/Homography/decompose_homography.cpp
// THEORY: docs/ch03_features.md §3.2.5
// TASK: estimate H, decomposeHomographyMat get rotation/translation candidates; need camera intrinsics K
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img1 = imread(getImagePath("lena.jpg"));
    if (img1.empty()) img1 = makeSyntheticTestImage();
    Mat img2 = img1.clone();
    Mat M = (Mat_<double>(2, 3) << 1, 0, 40, 0, 1, 25);
    warpAffine(img1, img2, M, img1.size());

    Ptr<ORB> orb = ORB::create(500);
    std::vector<KeyPoint> kp1, kp2;
    Mat d1, d2;
    orb->detectAndCompute(img1, noArray(), kp1, d1);
    orb->detectAndCompute(img2, noArray(), kp2, d2);
    BFMatcher matcher(NORM_HAMMING);
    std::vector<DMatch> matches;
    matcher.match(d1, d2, matches);
    std::vector<Point2f> pts1, pts2;
    for (const auto& m : matches) {
        pts1.push_back(kp1[m.queryIdx].pt);
        pts2.push_back(kp2[m.trainIdx].pt);
    }

    Mat H = findHomography(pts1, pts2, RANSAC, 3.0);
    Mat K = (Mat_<double>(3, 3) << img1.cols, 0, img1.cols / 2.0,
                                   0, img1.cols, img1.rows / 2.0,
                                   0, 0, 1);
    std::vector<Mat> Rs, Ts, Ns;
    int solutions = decomposeHomographyMat(H, K, Rs, Ts, Ns);
    logInfo("homography solutions=%d (need >=4 inlier matches)", solutions);
    if (solutions > 0) {
        logInfo("first translation: [%.2f, %.2f, %.2f]",
                Ts[0].at<double>(0), Ts[0].at<double>(1), Ts[0].at<double>(2));
    }
    Mat vis;
    drawMatches(img1, kp1, img2, kp2, matches, vis);
    dbgShow("L3_25 homography decompose", vis, 0);
    return 0;
}

