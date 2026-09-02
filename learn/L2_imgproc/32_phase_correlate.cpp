// LEARN: L2 鐩镐綅鐩稿叧骞崇Щ浼拌
// OFFICIAL: samples/cpp/phase_corr.cpp
// THEORY: docs/ch02_imgproc.md 搂6
// TASK: 瀵瑰钩绉诲悗鐨勫浘鍍忕敤 phaseCorrelate 浼拌 (dx,dy)锛涘弬鏁?response 瓒婂ぇ鍖归厤瓒婂彲闈?
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) src = makeSyntheticTestImage(); 
    if (src.channels() > 1) cvtColor(src, src, COLOR_BGR2GRAY);

    const Point2d shift(37.0, -21.0);
    Mat M = (Mat_<double>(2, 3) << 1, 0, shift.x, 0, 1, shift.y);
    Mat shifted;
    warpAffine(src, shifted, M, src.size(), INTER_LINEAR, BORDER_REPLICATE);

    Mat srcF, shiftedF;
    src.convertTo(srcF, CV_32F);
    shifted.convertTo(shiftedF, CV_32F);

    double response = 0.0;
    Point2d est = phaseCorrelate(shiftedF, srcF, noArray(), &response);
    logInfo("ground truth shift: (%.1f, %.1f)", shift.x, shift.y);
    logInfo("estimated shift   : (%.2f, %.2f), response=%.4f", est.x, est.y, response);
    logInfo("response: 瓒婃帴杩?1 琛ㄧず棰戝煙宄板€艰秺灏栭攼锛屽尮閰嶈秺鍙潬");

    Mat vis;
    cvtColor(src, vis, COLOR_GRAY2BGR);
    rectangle(vis, Rect(20, 20, 120, 80), Scalar(0, 255, 0), 2);
    Mat vis2;
    cvtColor(shifted, vis2, COLOR_GRAY2BGR);
    rectangle(vis2, Rect(20 + (int)shift.x, 20 + (int)shift.y, 120, 80), Scalar(0, 255, 0), 2);
    dbgShowMany({"src", "shifted"}, {vis, vis2}, 0);
    return 0;
}
