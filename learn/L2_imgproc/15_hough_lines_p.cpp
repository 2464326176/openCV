// LEARN: L2 HoughLinesP probabilistic Hough line
// OFFICIAL: samples/cpp/snippets/imgproc_HoughLinesP.cpp
// THEORY: docs/ch02_imgproc.md §5
// TASK: Canny then HoughLinesP draw segments (endpoints)
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int lowTh = 50;
static int thresh = 50;
static int minLen = 30;
static int maxGap = 10;

static void onTrack(int, void*) {
    Mat edges;
    Canny(gray, edges, lowTh, lowTh * 3, 3);
    std::vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI / 180, thresh, minLen, maxGap);
    Mat dst = src.clone();
    for (size_t i = 0; i < lines.size(); ++i) {
        Vec4i l = lines[i];
        line(dst, Point(l[0], l[1]), Point(l[2], l[3]),
             Scalar(0, 255, 0), 1, LINE_AA);
    }
    logInfo("P-lines=%zu", lines.size());
    Mat up;
    hconcat(src, dst, up);
    hconcat(up, edges, up);
    imshow("L2_15 HoughLinesP", up);
}

int main() {
    src = imread(getImagePath("VCG3.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_15 HoughLinesP", WINDOW_AUTOSIZE);
    createTrackbar("canny low", "L2_15 HoughLinesP", &lowTh, 200, onTrack);
    createTrackbar("hough th",  "L2_15 HoughLinesP", &thresh, 200, onTrack);
    createTrackbar("minLen",    "L2_15 HoughLinesP", &minLen, 200, onTrack);
    createTrackbar("maxGap",    "L2_15 HoughLinesP", &maxGap, 200, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
