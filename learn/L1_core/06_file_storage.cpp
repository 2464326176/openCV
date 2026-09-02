// LEARN: L1 FileStorage 璇诲啓
// OFFICIAL: samples/cpp/tutorial_code/core/file_input_output/file_input_output.cpp銆乮magelist_reader.cpp
// THEORY: docs/ch01_core.md 搂2.6 FileStorage
// TASK: 鍐欏叆 int/瀛楃涓?Mat 鍒?yml锛涜鍥為獙璇侊紱鍐欏浘鍍忚矾寰勫垪琛?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static const char* YML_FILE = "l1_06_test.yml";

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    Mat roi = src(Rect(0, 0, 64, 64)).clone();

    // 1) 鍐欏叆锛氭爣閲?+ 鐭╅樀 + 鍒楄〃
    FileStorage fw(YML_FILE, FileStorage::WRITE);
    if (!fw.isOpened()) { logInfo("open write failed"); return -1; }
    fw << "frameIdx" << 42;
    fw << "name" << "lena";
    fw << "roi" << roi;
    fw << "images" << "[";
    fw << "lena.jpg" << "VCG1.jpg" << "VCG2.jpg" << "]";
    fw.release();
    logInfo("write done -> %s", YML_FILE);

    // 2) 璇诲洖楠岃瘉
    FileStorage fr(YML_FILE, FileStorage::READ);
    if (!fr.isOpened()) { logInfo("open read failed"); return -1; }
    int frameIdx = -1; std::string name; Mat roi2;
    fr["frameIdx"] >> frameIdx;
    fr["name"]    >> name;
    fr["roi"]     >> roi2;

    FileNode n = fr["images"];
    std::vector<String> list;
    FileNodeIterator it = n.begin(), end = n.end();
    for (; it != end; ++it) list.push_back((String)*it);
    fr.release();

    logInfo("read back: frameIdx=%d name=%s list_size=%zu",
            frameIdx, name.c_str(), list.size());
    dbgMatInfo("roi2", roi2);

    // 3) 鏍￠獙 ROI 鏄惁涓€鑷?    Mat diff;
    absdiff(roi, roi2, diff);
    logInfo("roi round-trip diff = %d", countNonZero(diff));

    dbgShowMany({"src", "roi_write", "roi_read"},
                {src, roi, roi2}, 0);
    return 0;
}
