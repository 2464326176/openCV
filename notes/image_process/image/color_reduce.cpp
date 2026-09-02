/*
 * @Author: yh
 * @Date: 2022/8/4 21:53
 * @Description:
 * @FilePath: image.cpp
 */
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

#include <iostream>

using namespace cv;
using namespace std;

// Color space reduction
//
// Goal: compress each 8-bit channel (0..255) into a smaller number of levels.
// Mapping rule (with rounding to the bin center):
//     Inew = Iold / div * div + div / 2
//
// Workflow using a lookup table (LUT):
//   1. Build a 256-entry lookup table that maps every old value to its reduced value.
//   2. Apply it with cv::LUT() to transform the whole image in one vectorized pass.
//
// How to choose `div`?
//   Each channel is quantized into roughly 256 / div levels.
//
//   div      levels/channel      visual effect
//   ------------------------------------------------
//   4-8      ~64-32              nearly lossless, hard to notice
//   10-16    ~25-16              good for demos, slight posterization
//   32-64    ~8-4                strong effect; 64 is common in tutorials
//   128+     ~2 levels           collapses into flat color blocks
//   ------------------------------------------------
//   A value around 10-16 is a balanced choice for demos.
//   colorReduceByBit requires div to be a power of two (4/8/16/...).

static void fillReduceTable(uchar table[256], int div) {
    CV_Assert(div > 0);
    for (int v = 0; v < 256; ++v) {
        int value = (v / div) * div + div / 2;
        table[v] = saturate_cast<uchar>(value);
    }
}

/**
 * @brief colorReduceByLUT
 *        Stack LUT (no heap Mat for the table) + cv::LUT().
 *        OpenCV's LUT is SIMD/IPP-backed; this is usually the fastest general method.
 */
void colorReduceByLUT(const Mat &srcImage, Mat &destImage, int div) {
    CV_Assert(srcImage.depth() == CV_8U);
    uchar table[256];
    fillReduceTable(table, div);
    Mat lut(1, 256, CV_8U, table);  // wraps stack buffer, no extra alloc
    LUT(srcImage, lut, destImage);
}

/**
 * @brief colorReduceByPtr
 *        Inline division in the inner loop. Baseline pointer-scan version.
 */
void colorReduceByPtr(const Mat &srcImage, Mat &destImage, int div) {
    CV_Assert(srcImage.depth() == CV_8U && div > 0);
    destImage.create(srcImage.size(), srcImage.type());

    const uchar* src = srcImage.data;
    uchar* dst = destImage.data;
    size_t n = (size_t)srcImage.rows * srcImage.cols * srcImage.channels();
    if (!(srcImage.isContinuous() && destImage.isContinuous())) {
        n = (size_t)srcImage.cols * srcImage.channels();
        for (int i = 0; i < srcImage.rows; ++i) {
            src = srcImage.ptr<uchar>(i);
            dst = destImage.ptr<uchar>(i);
            for (size_t j = 0; j < n; ++j)
                dst[j] = (uchar)(src[j] / div * div + div / 2);
        }
        return;
    }
    for (size_t j = 0; j < n; ++j)
        dst[j] = (uchar)(src[j] / div * div + div / 2);
}

/**
 * @brief colorReduceByIterator
 *        Mat iterator access. No clone: write dest from src.
 */
void colorReduceByIterator(const Mat &srcImage, Mat &destImage, int div) {
    CV_Assert(srcImage.depth() == CV_8U && srcImage.channels() == 3 && div > 0);
    destImage.create(srcImage.size(), srcImage.type());

    auto itS = srcImage.begin<Vec3b>();
    auto itE = srcImage.end<Vec3b>();
    auto itD = destImage.begin<Vec3b>();
    while (itS != itE) {
        const Vec3b& s = *itS++;
        Vec3b& d = *itD++;
        d[0] = (uchar)(s[0] / div * div + div / 2);
        d[1] = (uchar)(s[1] / div * div + div / 2);
        d[2] = (uchar)(s[2] / div * div + div / 2);
    }
}

/**
 * @brief colorReduceByPtrTable
 *        Local 256-byte table + pointer scan. Fast for any positive div.
 */
void colorReduceByPtrTable(const Mat &srcImage, Mat &destImage, int div) {
    CV_Assert(srcImage.depth() == CV_8U);
    destImage.create(srcImage.size(), srcImage.type());

    uchar table[256];
    fillReduceTable(table, div);

    size_t n = (size_t)srcImage.rows * srcImage.cols * srcImage.channels();
    if (srcImage.isContinuous() && destImage.isContinuous()) {
        const uchar* src = srcImage.data;
        uchar* dst = destImage.data;
        const uchar* end = src + n;
        while (src < end)
            *dst++ = table[*src++];
        return;
    }
    const int cols = srcImage.cols * srcImage.channels();
    for (int i = 0; i < srcImage.rows; ++i) {
        const uchar* src = srcImage.ptr<uchar>(i);
        uchar* dst = destImage.ptr<uchar>(i);
        const uchar* end = src + cols;
        while (src < end)
            *dst++ = table[*src++];
    }
}

/**
 * @brief colorReduceByBit
 *        Bitmask instead of division. div MUST be a power of two and > 1.
 */
void colorReduceByBit(const Mat &srcImage, Mat &destImage, int div) {
    CV_Assert(srcImage.depth() == CV_8U);
    CV_Assert(div > 1 && (div & (div - 1)) == 0);

    destImage.create(srcImage.size(), srcImage.type());

    // uchar mask: for div=8, ~0x07 -> 0xF8. Keeps math in 8-bit, easier to vectorize.
    const uchar invMask = (uchar)~(div - 1);
    const uchar add = (uchar)(div / 2);

    size_t n = (size_t)srcImage.rows * srcImage.cols * srcImage.channels();
    if (srcImage.isContinuous() && destImage.isContinuous()) {
        const uchar* src = srcImage.data;
        uchar* dst = destImage.data;
        const uchar* end = src + n;
        while (src < end)
            *dst++ = (uchar)((*src++ & invMask) + add);
        return;
    }
    const int cols = srcImage.cols * srcImage.channels();
    for (int i = 0; i < srcImage.rows; ++i) {
        const uchar* src = srcImage.ptr<uchar>(i);
        uchar* dst = destImage.ptr<uchar>(i);
        const uchar* end = src + cols;
        while (src < end)
            *dst++ = (uchar)((*src++ & invMask) + add);
    }
}

/**
 * @brief colorReduceByAt
 *        Mat::at() access. No clone: write dest from src.
 */
void colorReduceByAt(const Mat &srcImage, Mat &destImage, int div) {
    CV_Assert(srcImage.depth() == CV_8U && srcImage.channels() == 3 && div > 0);
    destImage.create(srcImage.size(), srcImage.type());
    const int rows = srcImage.rows;
    const int cols = srcImage.cols;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const Vec3b& s = srcImage.at<Vec3b>(i, j);
            Vec3b& d = destImage.at<Vec3b>(i, j);
            d[0] = (uchar)(s[0] / div * div + div / 2);
            d[1] = (uchar)(s[1] / div * div + div / 2);
            d[2] = (uchar)(s[2] / div * div + div / 2);
        }
    }
}

// Warm-up once, then average `repeat` runs. Uses OpenCV tick (sub-ms).
template <typename F>
void bench(const char* name, int repeat, F func) {
    func();
    const int64 t0 = getTickCount();
    for (int k = 0; k < repeat; ++k)
        func();
    const double avgMs = (getTickCount() - t0) * 1000.0 / getTickFrequency() / repeat;
    logInfo("%-10s avg %.3f ms (x%d)", name, avgMs, repeat);
}

void timeTest() {
    Mat srcImage = imread(getImagePath("OIP.png"));
    if (srcImage.empty()) {
        logInfo("imread failed");
        return;
    }
    dbgMatInfo("src", srcImage);

    Mat destImage;
    destImage.create(srcImage.size(), srcImage.type());  // avoid first-call malloc in LUT

    const int div = 8;  // power of two -> also exercises colorReduceByBit
    const int repeat = 50;

    bench("lut",      repeat, [&]() { colorReduceByLUT(srcImage, destImage, div); });
    bench("ptr",      repeat, [&]() { colorReduceByPtr(srcImage, destImage, div); });
    bench("ptrTable", repeat, [&]() { colorReduceByPtrTable(srcImage, destImage, div); });
    bench("bit",      repeat, [&]() { colorReduceByBit(srcImage, destImage, div); });
    bench("iterator", repeat, [&]() { colorReduceByIterator(srcImage, destImage, div); });
    bench("at",       repeat, [&]() { colorReduceByAt(srcImage, destImage, div); });
}

int main() {
    timeTest();
    return 0;
}
