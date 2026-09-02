// LEARN: L2 Hit-or-miss
// OFFICIAL: samples/cpp/tutorial_code/ImgProc/HitMiss/HitMiss.cpp
// THEORY: docs/ch02_imgproc.md §2
// TASK: morphologyEx(MORPH_HITMISS) with custom kernel
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // Custom binary image: 3x3 block (center is foreground)
    Mat src = (Mat_<uchar>(8,8) <<
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 255, 255, 255, 0, 0, 0,
        0, 0, 255, 255, 255, 0, 0, 0,
        0, 0, 255, 255, 255, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0);

    // hit = 1, miss = -1, ignore = 0
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
