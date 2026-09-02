// LEARN: L1 SIMD 閫氱敤鍐呰仈
// OFFICIAL: samples/cpp/univ_intrin.cpp銆乻imd_basic.cpp
// THEORY: docs/ch01_core.md 搂2.9 SIMD 鍚戦噺鍖?// TASK: v_ universal intrinsics 鍋氶槇鍊煎寲锛堟渶灏忕ず渚嬶紝閫夊仛锛?#include <opencv2/opencv.hpp>
#include <opencv2/core/hal/intrin.hpp>
#include <opencv_utils.h>

using namespace cv;

// 鐢?universal intrinsics 鎶?src 闃堝€煎寲锛? thr -> maxv锛?=thr -> 0
static void thresholdSimd(const Mat& src, Mat& dst, uchar thr, uchar maxv) {
    CV_Assert(src.type() == CV_8UC1 && src.isContinuous());
    dst.create(src.size(), src.type());
    const uchar* sp = src.ptr<uchar>();
    uchar* dp = dst.ptr<uchar>();
    int total = (int)src.total();

    v_uint8 vthr  = v_setall_u8(thr);
    v_uint8 vmax  = v_setall_u8(maxv);
    v_uint8 vzero = v_setzero_u8();

    int i = 0;
    const int lanes = (int)v_uint8::nlanes;
    for (; i + lanes <= total; i += lanes) {
        v_uint8 v = v_load(sp + i);
        v_uint8 m = v_gt(v, vthr);     // mask: 0xFF where v>thr
        v_store(dp + i, v_select(m, vmax, vzero));
    }
    for (; i < total; ++i)
        dp[i] = sp[i] > thr ? maxv : 0;
}

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat simdDst;
    thresholdSimd(src, simdDst, 128, 255);

    // 鐢?OpenCV 鍐呯疆 threshold 鍋氬鐓?    Mat ref;
    threshold(src, ref, 128, 255, THRESH_BINARY);

    Mat diff;
    absdiff(simdDst, ref, diff);
    logInfo("simd vs builtin diff = %d", countNonZero(diff));

    dbgShowMany({"src", "simd", "builtin"},
                {src, simdDst, ref}, 0);
    return 0;
}
