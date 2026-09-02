// LEARN: L1 ECC 鍥惧儚瀵归綈
// OFFICIAL: samples/cpp/image_alignment.cpp
// THEORY: docs/ch01_core.md 搂2.16 ECC
// TASK: 鍚屼竴寮犲浘+浜哄伐浠垮皠鍙樻崲鍋氬榻愰獙璇侊紙findTransformECC + warpAffine锛?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }

    // 1) 浜哄伐浠垮皠鍙樻崲鏋勯€犳祴璇曞浘锛堝钩绉?12,6 + 寰皬鏃嬭浆/鍓垏锛?    Point2f srcTri[3] = {
        Point2f(50.f, 50.f),
        Point2f(200.f, 50.f),
        Point2f(50.f, 200.f)
    };
    Point2f dstTri[3] = {
        Point2f(62.f, 56.f),
        Point2f(212.f, 56.f),
        Point2f(56.f, 206.f)
    };
    Mat A = getAffineTransform(srcTri, dstTri);
    Mat warped;
    warpAffine(src, warped, A, src.size(), INTER_LINEAR);
    dbgMatInfo("A", A);

    // 2) findTransformECC锛氫及璁℃妸 warped 瀵归綈鍒?src 鐨勪豢灏勭煩闃?    Mat est = Mat::eye(2, 3, CV_32F);
    TermCriteria tc(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-5);
    double cc = findTransformECC(src, warped, est, MOTION_AFFINE, tc);
    logInfo("ECC final correlation = %.6f", cc);
    dbgMatInfo("est", est);

    // 3) 鐢ㄤ及璁＄殑鐭╅樀鍥?warp锛岀湅涓庡師鍥惧樊寮?    Mat recovered;
    warpAffine(warped, recovered, est, src.size(), INTER_LINEAR);
    Mat diff;
    absdiff(src, recovered, diff);
    threshold(diff, diff, 10, 255, THRESH_BINARY);
    double mismatch = countNonZero(diff) / (double)diff.total();
    logInfo("recovered mismatch ratio = %.4f", mismatch);

    dbgShowMany({"src", "warped", "recovered", "diff"},
                {src, warped, recovered, diff}, 0);
    return 0;
}
