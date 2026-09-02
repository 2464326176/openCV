// LEARN: L1 鎺╄啘鍗风Н
// OFFICIAL: samples/cpp/tutorial_code/core/mat_mask_operations/mat_mask_operations.cpp
// THEORY: docs/ch01_core.md 搂2.6
// TASK: 鎵嬪啓閿愬寲鍗风Н vs filter2D锛涙粦鍔ㄦ潯鏀?strength 鏄剧ず鏁堟灉瀵规瘮
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int strength = 5; // 鏍镐腑蹇冨€硷紝閭诲煙 -1

static void onTrack(int, void*) {
    if (strength < 1) strength = 1;
    Mat kernel = (Mat_<float>(3, 3) <<
                   0, -1,  0,
                  -1, strength, -1,
                   0, -1,  0);
    // 1) filter2D锛歄penCV 閫氱敤鍗风Н
    Mat sharp;
    filter2D(src, sharp, src.depth(), kernel);

    // 2) 鎵嬪啓 3x3 鍗风Н锛堜笉澶勭悊杈圭晫锛?    Mat manual = Mat::zeros(src.size(), src.type());
    for (int y = 1; y < src.rows - 1; ++y) {
        for (int x = 1; x < src.cols - 1; ++x) {
            float s[3] = {0, 0, 0};
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    Vec3b p = src.at<Vec3b>(y + dy, x + dx);
                    float k = kernel.at<float>(dy + 1, dx + 1);
                    s[0] += p[0] * k;
                    s[1] += p[1] * k;
                    s[2] += p[2] * k;
                }
            manual.at<Vec3b>(y, x) = Vec3b(saturate_cast<uchar>(s[0]),
                                          saturate_cast<uchar>(s[1]),
                                          saturate_cast<uchar>(s[2]));
        }
    }
    dbgPrint("kernel center", strength);
    dbgShowMany({"src", "filter2D", "manual"},
                {src, sharp, manual}, 0);
}

int main() {
    src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("src", WINDOW_AUTOSIZE);
    createTrackbar("strength", "src", &strength, 9, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
