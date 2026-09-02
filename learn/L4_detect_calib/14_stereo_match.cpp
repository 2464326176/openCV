// LEARN: L4 binocular stereo matching StereoSGBM
// OFFICIAL: samples/cpp/stereo_match.cpp, stereo_calib.cpp
// THEORY: docs/ch07_calib3d_stitching.md §4
// TASK: resize VCG1+VCG2 to same-size grayscale, compute disparity with StereoSGBM and display with colormap
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat l = imread(getImagePath("VCG1.jpg"));
    Mat r = imread(getImagePath("VCG2.jpg"));
    if (l.empty() || r.empty()) { logInfo("imread failed"); return -1; }
    dbgMatInfo("L", l); dbgMatInfo("R", r);

    // 1) stereo images usually need same size; force resize to min(W,H) x min(W,H)
    int w = std::min(l.cols, r.cols);
    int h = std::min(l.rows, r.rows);
    resize(l, l, Size(w, h));
    resize(r, r, Size(w, h));
    Mat lg, rg;
    cvtColor(l, lg, COLOR_BGR2GRAY);
    cvtColor(r, rg, COLOR_BGR2GRAY);

    // 2) StereoSGBM
    Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, 16 * 5, 5);
    sgbm->setP1(8 * 1 * 5 * 5);
    sgbm->setP2(32 * 1 * 5 * 5);
    sgbm->setDisp12MaxDiff(1);
    sgbm->setPreFilterCap(31);
    sgbm->setUniquenessRatio(10);
    sgbm->setSpeckleWindowSize(100);
    sgbm->setSpeckleRange(32);
    sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);

    Mat disp, disp8;
    sgbm->compute(lg, rg, disp);
    if (disp.empty()) { logInfo("disparity empty"); return -1; }
    disp.convertTo(disp8, CV_8U, 255.0 / (16 * 5 * 2));
    applyColorMap(disp8, disp8, COLORMAP_JET);
    logInfo("StereoSGBM disp done, numDisp=%d", sgbm->getNumDisparities());

    dbgShowMany({"L4_14 left", "L4_14 right", "L4_14 disparity"}, {l, r, disp8}, 0);
    return 0;
}
