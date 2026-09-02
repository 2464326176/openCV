// LEARN: L1 JPEG codec

// OFFICIAL: samples/cpp/imgcodecs_jpeg.cpp、imgcodecs_imwrite.cpp
// THEORY: docs/ch01_core.md §3.1 imgcodecs
// TASK: imwrite with different quality; imencode to memory buffer; imdecode from buffer
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    dbgMatInfo("src", src);

    // 1) imwrite: different quality to file
    imwrite("l1_10_q100.jpg", src,
            {IMWRITE_JPEG_QUALITY, 100});
    imwrite("l1_10_q50.jpg",  src,
            {IMWRITE_JPEG_QUALITY, 50});
    imwrite("l1_10_q10.jpg",  src,
            {IMWRITE_JPEG_QUALITY, 10});

    // 2) imencode: encode to memory buffer
    std::vector<uchar> buf;
    std::vector<int> params = {IMWRITE_JPEG_QUALITY, 50};
    imencode(".jpg", src, buf, params);
    logInfo("encoded buffer size = %zu bytes", buf.size());

    // 3) imdecode: reconstruct Mat from buffer
    Mat decoded = imdecode(buf, IMREAD_COLOR);
    if (decoded.empty()) { logInfo("imdecode failed"); return -1; }
    dbgMatInfo("decoded", decoded);

    // 4) Compare difference with original
    Mat q50 = imread("l1_10_q50.jpg");
    Mat diff;
    absdiff(src, decoded, diff);
    dbgStats("diff(src vs decoded)", diff);

    dbgShowMany({"src", "q10", "q50", "decoded"},
                {src,
                 imread("l1_10_q10.jpg"),
                 q50,
                 decoded}, 0);
    return 0;
}
