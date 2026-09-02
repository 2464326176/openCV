// LEARN: L0 閫氶亾鎷嗗垎鍚堝苟
// OFFICIAL: samples/cpp/snippets/core_split.cpp, core_merge.cpp
// THEORY: docs/ch01_core.md 搂Mat 閫氶亾
// TASK: split 鎷?BGR锛涘垎鍒樉绀哄崟閫氶亾锛沵erge 鍚堝洖
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    std::vector<Mat> ch;
    split(src, ch); // [B, G, R]

    Mat zeros = Mat::zeros(src.size(), CV_8UC1);
    Mat onlyB, onlyG, onlyR;
    std::vector<Mat> b = {ch[0], zeros, zeros}; merge(b, onlyB);
    std::vector<Mat> g = {zeros, ch[1], zeros}; merge(g, onlyG);
    std::vector<Mat> r = {zeros, zeros, ch[2]}; merge(r, onlyR);

    Mat merged;
    merge(ch, merged);

    dbgMatInfo("B", ch[0]);
    dbgShowMany({"src", "B", "G", "R", "merged"},
                {src, onlyB, onlyG, onlyR, merged}, 0);
    return 0;
}
