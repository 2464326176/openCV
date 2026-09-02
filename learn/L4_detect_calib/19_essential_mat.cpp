// LEARN: L4 essential matrix + 3D reconstruction
// OFFICIAL: samples/cpp/essential_mat_reconstr.cpp
// THEORY: docs/ch07_calib3d_stitching.md §6
// TASK: estimate E from 8 synthetic normalized point pairs + known intrinsic K, recoverPose to get R/t and print
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // calibrated camera intrinsics
    Mat K = (Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
    logInfo("intrinsic K set: fx=fy=800, cx=320, cy=240");

    // 8 pairs of synthetic normalized image points (pixel coordinates), global translation + small perturbation on second image
    std::vector<Point2f> pts1 = {
        {100, 100}, {150, 90}, {200, 110}, {250, 130},
        {110, 200}, {160, 210}, {210, 220}, {260, 230}};
    std::vector<Point2f> pts2 = pts1;
    for (auto& p : pts2) { p.x += 18.f; p.y += 4.f; }

    Mat mask;
    Mat E = findEssentialMat(pts1, pts2, K, RANSAC, 0.999, 1.0, mask);
    if (E.empty()) { logInfo("findEssentialMat returned empty"); return -1; }
    logInfo("findEssentialMat RANSAC inliers=%d", countNonZero(mask));

    Mat R, t;
    int inliers = recoverPose(E, pts1, pts2, K, R, t, mask);
    logInfo("recoverPose inliers=%d", inliers);
    logInfo("R[0]=%.3f %.3f %.3f  t=(%.3f, %.3f, %.3f)",
            R.at<double>(0, 0), R.at<double>(0, 1), R.at<double>(0, 2),
            t.at<double>(0), t.at<double>(1), t.at<double>(2));
    return 0;
}
