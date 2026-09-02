// LEARN: L2 鍑讳腑鍑讳笉涓?// OFFICIAL: samples/cpp/tutorial_code/ImgProc/HitMiss/HitMiss.cpp
// THEORY: docs/ch02_imgproc.md 搂2
// TASK: morphologyEx(MORPH_HITMISS) 鐢ㄨ嚜寤烘牳
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // 鑷缓浜屽€煎浘锛?x3 鏂瑰潡 (涓績涓哄墠鏅?
    Mat src = (Mat_<uchar>(8,8) <<
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 255, 255, 255, 0, 0, 0,
        0, 0, 255, 255, 255, 0, 0, 0,
        0, 0, 255, 255, 255, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0);

    // 鍑讳腑閮ㄥ垎 = 1锛屽嚮涓嶄腑閮ㄥ垎 = -1锛屽拷鐣?= 0
    Mat kernel = (Mat_<char>(3,3) <<
         0,  1,  0,
         1, -1,  1,
         0,  1,  0);

    Mat dst;
    morphologyEx(src, dst, MORPH_HITMISS, kernel);
    dbgMatInfo("hitmiss-src", src);
    dbgMatInfo("hitmiss-dst", dst);

    Mat srcUp, dstUp;
    resize(src, srcUp, Size(), 30, 30, INTER_NEAREST);
    resize(dst, dstUp, Size(), 30, 30, INTER_NEAREST);
    Mat grid;
    hconcat(srcUp, dstUp, grid);
    dbgShow("L2_06 HitMiss", grid, 0);
    return 0;
}
