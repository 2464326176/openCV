// LEARN: L3 Shi-Tomasi goodFeaturesToTrack
// OFFICIAL: samples/cpp/tutorial_code/TrackingMotion/goodFeaturesToTrack_Demo.cpp
// THEORY: docs/ch03_features.md 搂瑙掔偣
// TASK: goodFeaturesToTrack 鍙?N 涓己瑙掔偣锛宑ircle 鏍囨敞锛屾粦鍔ㄦ潯鏀瑰弬鏁?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int maxCorners = 30;
static int quality_x1000 = 10;  // 瀹為檯 *0.001
static int minDist = 10;

static void onTrack(int, void*) {
    std::vector<Point2f> corners;
    goodFeaturesToTrack(src, corners, maxCorners,
                        quality_x1000 / 1000.0, minDist);
    Mat show; cvtColor(src, show, COLOR_GRAY2BGR);
    for (size_t i = 0; i < corners.size(); ++i)
        circle(show, corners[i], 4, Scalar(0, 0, 255), -1);
    logInfo("corners=%zu", corners.size());
    imshow("L3_02 good_features", show);
}

int main() {
    src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L3_02 good_features", WINDOW_AUTOSIZE);
    createTrackbar("MaxCorners",    "L3_02 good_features", &maxCorners,    100, onTrack);
    createTrackbar("Quality*0.001",  "L3_02 good_features", &quality_x1000, 50,  onTrack);
    createTrackbar("MinDist",        "L3_02 good_features", &minDist,       50,  onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
