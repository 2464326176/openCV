//********************
// Author:  yh
// Time:    2022/8/5.
//  Feature matching + homography matrix + image stitching
//  Workflow: ORB features -> matching -> RANSAC homography H -> warpPerspective stitching
//  (this build has no xfeatures2d, so ORB is used; swap to xfeatures2d::SIFT::create() if available)
//  Official example: stitching.cpp / SURF_FLANN_matching_homography_Demo.cpp
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    String img1Name("../data/images/VCG1.jpg");
    String img2Name("../data/images/VCG2.jpg");
    if (argc > 2) { img1Name = argv[1]; img2Name = argv[2]; }

    Mat img1 = imread(img1Name);
    Mat img2 = imread(img2Name);
    if (img1.empty() || img2.empty()) { cout << "could not load images" << endl; return -1; }

    // 1. Feature detection + description (ORB: binary descriptor, use NORM_HAMMING)
    Ptr<Feature2D> detector = ORB::create();
    vector<KeyPoint> kp1, kp2;
    Mat desc1, desc2;
    detector->detectAndCompute(img1, Mat(), kp1, desc1);
    detector->detectAndCompute(img2, Mat(), kp2, desc2);

    // 2. Match + Lowe's ratio test
    BFMatcher matcher(NORM_HAMMING);
    vector<vector<DMatch>> knnMatches;
    matcher.knnMatch(desc1, desc2, knnMatches, 2);

    vector<DMatch> goodMatches;
    for (size_t i = 0; i < knnMatches.size(); i++) {
        if (knnMatches[i][0].distance < 0.75 * knnMatches[i][1].distance)
            goodMatches.push_back(knnMatches[i][0]);
    }

    // 3. RANSAC homography H from matched points (mapping img2 -> img1)
    vector<Point2f> pts1, pts2;
    for (size_t i = 0; i < goodMatches.size(); i++) {
        pts1.push_back(kp1[goodMatches[i].queryIdx].pt);
        pts2.push_back(kp2[goodMatches[i].trainIdx].pt);
    }
    Mat H = findHomography(pts2, pts1, RANSAC, 3.0);

    // 4. warpPerspective maps img2 into img1's coordinate system
    Mat result;
    warpPerspective(img2, result, H, Size(img1.cols + img2.cols, img1.rows));
    Mat roi(result, Rect(0, 0, img1.cols, img1.rows));
    img1.copyTo(roi);   // overlay img1 on the left half

    // visualize matches
    Mat matchImg;
    drawMatches(img1, kp1, img2, kp2, goodMatches, matchImg);

    imshow("matches", matchImg);
    imshow("stitched", result);
    waitKey(0);
    return 0;
}
