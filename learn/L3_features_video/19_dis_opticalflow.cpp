// LEARN: L3 DIS Dense Optical Flow
// OFFICIAL: samples/cpp/dis_opticalflow.cpp (reference official sample)
// THEORY: docs/ch04_video.md §optical flow
// TASK: DISOpticalFlow compute VCG1→VCG2 optical flow, HSV encoded display
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat flowToBgr(const Mat& flow) {
    Mat xy[2]; split(flow, xy);
    Mat mag, ang, magN;
    cartToPolar(xy[0], xy[1], mag, ang, true);
    normalize(mag, magN, 0.0, 1.0, NORM_MINMAX, CV_32F);
    ang *= (180.0 / (2.0 * CV_PI));
    Mat hsvCh[3] = { ang, Mat::ones(ang.size(), CV_32F), magN };
    Mat hsvF, hsv8, bgr;
    merge(hsvCh, 3, hsvF);
    hsvF.convertTo(hsv8, CV_8U, 255.0);
    cvtColor(hsv8, bgr, COLOR_HSV2BGR);
    return bgr;
}

int main() {
    Mat f1 = imread(getImagePath("VCG1.jpg"));
    Mat f2 = imread(getImagePath("VCG2.jpg"));
    if (f1.empty() || f2.empty()) { logInfo("imread failed"); return -1; }
    resize(f1, f1, Size(480, 320)); resize(f2, f2, Size(480, 320));
    Mat g1, g2; cvtColor(f1, g1, COLOR_BGR2GRAY); cvtColor(f2, g2, COLOR_BGR2GRAY);

    Ptr<DISOpticalFlow> dis = DISOpticalFlow::create(DISOpticalFlow::PRESET_FAST);
    Mat flow;
    dis->calc(g1, g2, flow);

    Mat bgr = flowToBgr(flow);
    logInfo("dis flow ok, type=%d", flow.type());
    dbgShowMany({"L3_19 frame1", "L3_19 dis flow"}, {f1, bgr}, 0);
    return 0;
}

