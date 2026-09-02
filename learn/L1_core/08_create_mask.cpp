// LEARN: L1 鍒涘缓鎺╄啘
// OFFICIAL: samples/cpp/create_mask.cpp
// THEORY: docs/ch01_core.md 搂2.15 create_mask
// TASK: Mat::setTo + bitwise_and/or/not 瀹炵幇鍦嗗舰鎺╄啘鎶犲浘涓庤儗鏅悎鎴?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    // 1) 鍒涘缓鍦嗗舰鎺╄啘锛歴etTo 鎶婂叏鍥剧疆 0锛宑ircle 鍦嗗唴濉?255
    Mat mask = Mat::zeros(src.size(), CV_8UC1);
    circle(mask, Point(src.cols / 2, src.rows / 2),
           std::min(src.cols, src.rows) / 3, Scalar(255), -1);

    // 2) bitwise_and锛氬彧淇濈暀鍦嗗唴鐨?src 鍍忕礌
    Mat masked;
    bitwise_and(src, src, masked, mask);

    // 3) 鍙嶆帺鑶?+ setTo锛氬渾澶栧尯鍩熷～鍏呯豢鑹茶儗鏅?    Mat invMask;
    bitwise_not(mask, invMask);
    Mat bg = Mat::zeros(src.size(), src.type());
    bg.setTo(Scalar(0, 255, 0));               // 鐢?setTo 濉厖搴曡壊
    Mat bgOut;
    bitwise_and(bg, bg, bgOut, invMask);      // 鍦嗗涓虹豢鑹诧紝鍦嗗唴 0

    // 4) bitwise_or 鍚堟垚鏈€缁堝浘
    Mat composed;
    bitwise_or(masked, bgOut, composed);

    dbgPrint("mask size", mask.size());
    dbgShowMany({"src", "mask", "masked", "bg", "composed"},
                {src, mask, masked, bgOut, composed}, 0);
    return 0;
}
