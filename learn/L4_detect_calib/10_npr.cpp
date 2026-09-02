// LEARN: L4 闈炵湡瀹炴劅娓叉煋 NPR
// OFFICIAL: tutorial_code/photo/non_photorealistic_rendering/npr_demo.cpp, npr_demo.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.24
// TASK: edgePreservingFilter / detailEnhance / stylization 涓夌 NPR 婊ら暅鍦?VCG1 涓婂姣?#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("VCG1.jpg"));
    if (img.empty()) { logInfo("imread failed: VCG1.jpg"); return -1; }
    dbgMatInfo("img", img);

    // 缂╁埌鍚堢悊澶у皬渚夸簬鏄剧ず
    Mat small;
    resize(img, small, Size(), 480.0 / std::max(img.cols, img.rows), 480.0 / std::max(img.cols, img.rows), INTER_AREA);

    Mat edge, detail, styl;
    edgePreservingFilter(small, edge, 1, 60.0);    // flags=RECURS_FILTER
    detailEnhance(small, detail, 10.0f, 0.15f);
    stylization(small, styl, 60, 0.7);
    logInfo("edgePreservingFilter + detailEnhance + stylization done");

    dbgShowMany({"L4_10 input", "L4_10 edge", "L4_10 detail", "L4_10 styl"},
                {small, edge, detail, styl}, 0);
    return 0;
}
