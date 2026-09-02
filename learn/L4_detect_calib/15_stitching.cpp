// LEARN: L4 鍥惧儚鎷兼帴 Stitcher
// OFFICIAL: samples/cpp/stitching.cpp, stitching_detailed.cpp
// THEORY: docs/ch07_calib3d_stitching.md 搂8
// TASK: 鐢?VCG1 + VCG2 涓ゅ紶鍥捐皟鐢?Stitcher::create 榛樿鍏ㄦ櫙妯″紡鎷兼垚涓€寮犲苟鏄剧ず
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <opencv_utils.h>

using namespace cv;
using namespace std;

int main() {
    Mat a = imread(getImagePath("VCG1.jpg"));
    Mat b = imread(getImagePath("VCG2.jpg"));
    if (a.empty() || b.empty()) { logInfo("imread failed"); return -1; }
    dbgMatInfo("a", a); dbgMatInfo("b", b);

    vector<Mat> imgs = {a, b};
    Mat pano;
    Ptr<Stitcher> st = Stitcher::create(Stitcher::PANORAMA);
    Stitcher::Status status = st->stitch(imgs, pano);
    if (status != Stitcher::OK) {
        logInfo("stitch failed, status=%d (need overlap images)", (int)status);
        dbgShow("L4_15 a", a);
        dbgShow("L4_15 b", b);
        return 0;
    }
    logInfo("stitch OK, pano size=%dx%d", pano.cols, pano.rows);
    dbgShow("L4_15 pano", pano);
    return 0;
}
