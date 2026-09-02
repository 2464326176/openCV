// LEARN: L3 MSER 鍖哄煙妫€娴?
// OFFICIAL: samples/cpp/detect_mser.cpp
// THEORY: docs/ch03_features.md 搂3.3
// TASK: MSER 妫€娴嬬ǔ瀹氭瀬鍊煎尯鍩燂紱delta/minArea/maxArea 鎺у埗鍖哄煙灏哄害
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
    logInfo("delta=5: 瓒婂皬瓒婃晱鎰燂紱minArea/maxArea 闄愬埗鍖哄煙闈㈢Н鑼冨洿");
    dbgShow("L3_23 mser", vis, 0);
    return 0;
}
