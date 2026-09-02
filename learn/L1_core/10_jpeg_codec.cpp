// LEARN: L1 JPEG 缂栬В鐮?// OFFICIAL: samples/cpp/imgcodecs_jpeg.cpp銆乮mgcodecs_imwrite.cpp
// THEORY: docs/ch01_core.md 搂3.1 imgcodecs
// TASK: imwrite 涓嶅悓 quality锛沬mencode 鍒板唴瀛?buffer锛沬mdecode 浠?buffer 閲嶅缓
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    dbgMatInfo("src", src);

    // 1) imwrite锛氫笉鍚?quality 鍐欐枃浠?    imwrite("l1_10_q100.jpg", src,
            {IMWRITE_JPEG_QUALITY, 100});
    imwrite("l1_10_q50.jpg",  src,
            {IMWRITE_JPEG_QUALITY, 50});
    imwrite("l1_10_q10.jpg",  src,
            {IMWRITE_JPEG_QUALITY, 10});

    // 2) imencode锛氱紪鐮佸埌鍐呭瓨 buffer
    std::vector<uchar> buf;
    std::vector<int> params = {IMWRITE_JPEG_QUALITY, 50};
    imencode(".jpg", src, buf, params);
    logInfo("encoded buffer size = %zu bytes", buf.size());

    // 3) imdecode锛氫粠 buffer 閲嶅缓 Mat
    Mat decoded = imdecode(buf, IMREAD_COLOR);
    if (decoded.empty()) { logInfo("imdecode failed"); return -1; }
    dbgMatInfo("decoded", decoded);

    // 4) 涓庡師鍥惧姣斿樊寮?    Mat q50 = imread("l1_10_q50.jpg");
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
