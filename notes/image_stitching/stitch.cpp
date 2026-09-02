//********************
// Author:  yh
// Time:    2022/8/5.
//  特征匹配 + 单应性矩阵 + 图像拼接
//  流程：ORB 提取特征 → 匹配 → RANSAC 求单应 H → warpPerspective 拼接
//  （本机 OpenCV 未编 xfeatures2d，用 ORB；若可用 SIFT 可换 xfeatures2d::SIFT::create()）
//  对应官方示例: stitching.cpp / SURF_FLANN_matching_homography_Demo.cpp
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

    // 1. 特征检测 + 描述（ORB：二进制描述子，配 NORM_HAMMING）
    Ptr<Feature2D> detector = ORB::create();
    vector<KeyPoint> kp1, kp2;
    Mat desc1, desc2;
    detector->detectAndCompute(img1, Mat(), kp1, desc1);
    detector->detectAndCompute(img2, Mat(), kp2, desc2);

    // 2. 匹配 + Lowe's ratio test
    BFMatcher matcher(NORM_HAMMING);
    vector<vector<DMatch>> knnMatches;
    matcher.knnMatch(desc1, desc2, knnMatches, 2);

    vector<DMatch> goodMatches;
    for (size_t i = 0; i < knnMatches.size(); i++) {
        if (knnMatches[i][0].distance < 0.75 * knnMatches[i][1].distance)
            goodMatches.push_back(knnMatches[i][0]);
    }

    // 3. 由匹配点对用 RANSAC 求单应性矩阵 H（img2 → img1 的映射）
    vector<Point2f> pts1, pts2;
    for (size_t i = 0; i < goodMatches.size(); i++) {
        pts1.push_back(kp1[goodMatches[i].queryIdx].pt);
        pts2.push_back(kp2[goodMatches[i].trainIdx].pt);
    }
    Mat H = findHomography(pts2, pts1, RANSAC, 3.0);

    // 4. warpPerspective 把 img2 变换到 img1 的坐标系
    Mat result;
    warpPerspective(img2, result, H, Size(img1.cols + img2.cols, img1.rows));
    Mat roi(result, Rect(0, 0, img1.cols, img1.rows));
    img1.copyTo(roi);   // 左半部分覆盖 img1

    // 可视化匹配
    Mat matchImg;
    drawMatches(img1, kp1, img2, kp2, goodMatches, matchImg);

    imshow("matches", matchImg);
    imshow("stitched", result);
    waitKey(0);
    return 0;
}
