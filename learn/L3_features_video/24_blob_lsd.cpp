// LEARN: L3 Blob 妫€娴嬩笌 LSD 绾挎
// OFFICIAL: samples/cpp/detect_blob.cpp, samples/cpp/lsd_lines.cpp
// THEORY: docs/ch03_features.md 搂3.4
// TASK: SimpleBlobDetector 鎵炬枒鐐癸紱LineSegmentDetector 鎻愬彇绾挎
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) src = makeSyntheticTestImage();
    if (src.channels() > 1) cvtColor(src, src, COLOR_BGR2GRAY);

    Ptr<SimpleBlobDetector> blobDet = SimpleBlobDetector::create();
    std::vector<KeyPoint> kps;
    blobDet->detect(src, kps);
    Mat blobVis;
    cvtColor(src, blobVis, COLOR_GRAY2BGR);
    drawKeypoints(blobVis, kps, blobVis, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    Ptr<LineSegmentDetector> lsd = createLineSegmentDetector();
    std::vector<Vec4f> lines;
    lsd->detect(src, lines);
    Mat lineVis;
    cvtColor(src, lineVis, COLOR_GRAY2BGR);
    lsd->drawSegments(lineVis, lines);

    logInfo("blobs=%zu lines=%zu", kps.size(), lines.size());
    dbgShowMany({"blob", "lsd"}, {blobVis, lineVis}, 0);
    return 0;
}
