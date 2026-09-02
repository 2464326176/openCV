// LEARN: L4 epipolar geometry fundamental matrix
// OFFICIAL: samples/cpp/epipolar_lines.cpp
// THEORY: docs/ch07_calib3d_stitching.md §5
// TASK: estimate F from 8 synthetic point pairs (FM_8POINT + RANSAC), computeCorrespondEpilines to draw epilines on img2
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // two images of the same size (VCG1 duplicated, used only as epiline background)
    Mat img1 = imread(getImagePath("VCG1.jpg"));
    if (img1.empty()) { logInfo("imread failed: VCG1.jpg"); return -1; }
    Mat img2 = img1.clone();
    dbgMatInfo("img1", img1);

    // 8 pairs of synthetic corresponding points (slight offset from img1 to img2 to simulate matching)
    std::vector<Point2f> pts1 = {
        {120, 100}, {180, 90}, {250, 120}, {300, 110},
        {150, 200}, {220, 210}, {280, 230}, {350, 200}};
    std::vector<Point2f> pts2 = pts1;
    for (auto& p : pts2) { p.x += 25.f; p.y += 8.f; }

    Mat F = findFundamentalMat(pts1, pts2, FM_8POINT);
    logInfo("findFundamentalMat FM_8POINT done, F empty=%d", F.empty());
    if (F.empty()) { logInfo("F is empty"); return -1; }

    // draw epilines corresponding to pts1 on img2
    std::vector<Vec3f> lines;
    computeCorrespondEpilines(pts1, 1, F, lines);
    for (size_t i = 0; i < lines.size(); ++i) {
        float a = lines[i][0], b = lines[i][1], c = lines[i][2];
        double denom = (a * a + b * b);
        if (denom < 1e-6) continue;
        // compute intersection of line with image top/bottom edges and draw
        Point2f p0(0, -c / b);
        Point2f p1((float)img2.cols, -(c + a * img2.cols) / b);
        line(img2, p0, p1, Scalar(0, 255, 0), 1);
        circle(img1, pts1[i], 4, Scalar(0, 0, 255), -1);
        circle(img2, pts2[i], 4, Scalar(0, 0, 255), -1);
    }
    dbgShowMany({"L4_13 img1 pts", "L4_13 img2 epilines"}, {img1, img2}, 0);
    return 0;
}
