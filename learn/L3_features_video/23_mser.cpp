// LEARN: L3 MSER Region Detection
// OFFICIAL: samples/cpp/detect_mser.cpp
// THEORY: docs/ch03_features.md §3.3
// TASK: MSER detect stable extremal regions; delta/minArea/maxArea control region scale
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) src = makeSyntheticTestImage();
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    Ptr<MSER> mser = MSER::create(5, 60, 14400);
    std::vector<std::vector<Point>> regions;
    std::vector<Rect> bboxes;
    mser->detectRegions(gray, regions, bboxes);

    Mat vis = src.clone();
    for (const auto& r : bboxes) rectangle(vis, r, Scalar(0, 255, 0), 1);
    logInfo("MSER regions=%zu bboxes=%zu", regions.size(), bboxes.size());
    logInfo("delta=5: smaller is more sensitive; minArea/maxArea limit region area range");
    dbgShow("L3_23 mser", vis, 0);
    return 0;
}

