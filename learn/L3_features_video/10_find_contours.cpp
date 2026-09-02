// LEARN: L3 Contour Detection findContours
// OFFICIAL: samples/cpp/tutorial_code/ShapeDescriptors/findContours_demo.cpp、contours2.cpp
// THEORY: docs/ch03_features.md §轮廓
// TASK: Canny edge detection; findContours(RETR_EXTERNAL) get contours; drawContours random coloring
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int thresh = 100;

static void onTrack(int, void*) {
    Mat edge; Canny(src, edge, thresh, thresh * 2);
    std::vector<std::vector<Point>> conts;
    std::vector<Vec4i> hier;
    findContours(edge, conts, hier, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    Mat show = Mat::zeros(src.size(), CV_8UC3);
    RNG rng(12345);
    for (size_t i = 0; i < conts.size(); ++i) {
        Scalar c(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
        drawContours(show, conts, (int)i, c, 2);
    }
    logInfo("thresh=%d contours=%zu", thresh, conts.size());
    imshow("L3_10 contours", show);
}

int main() {
    src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L3_10 contours", WINDOW_AUTOSIZE);
    createTrackbar("Canny", "L3_10 contours", &thresh, 255, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}

