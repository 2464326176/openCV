// LEARN: L1 FileStorage read/write
// OFFICIAL: samples/cpp/tutorial_code/core/file_input_output/file_input_output.cpp、imagelist_reader.cpp
// THEORY: docs/ch01_core.md §2.6 FileStorage
// TASK: write int/string/Mat to yml; read back to verify; write image path list
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static const char* YML_FILE = "l1_06_test.yml";

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    Mat roi = src(Rect(0, 0, 64, 64)).clone();

    // 1) Write: scalar + matrix + list
    FileStorage fw(YML_FILE, FileStorage::WRITE);
    if (!fw.isOpened()) { logInfo("open write failed"); return -1; }
    fw << "frameIdx" << 42;
    fw << "name" << "lena";
    fw << "roi" << roi;
    fw << "images" << "[";
    fw << "lena.jpg" << "VCG1.jpg" << "VCG2.jpg" << "]";
    fw.release();
    logInfo("write done -> %s", YML_FILE);

    // 2) Read back verification
    FileStorage fr(YML_FILE, FileStorage::READ);
    if (!fr.isOpened()) { logInfo("open read failed"); return -1; }
    int frameIdx = -1;
    std::string name;
    Mat roi2;
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

    // 3) Verify ROI consistency
    Mat diff;
    absdiff(roi, roi2, diff);
    logInfo("roi round-trip diff = %d", countNonZero(diff));

    dbgShowMany({"src", "roi_write", "roi_read"},
                {src, roi, roi2}, 0);
    return 0;
}
