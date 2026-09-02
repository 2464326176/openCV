// LEARN: L2 HoughCircles 闇嶅か鍦?// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/HoughCircle_Demo.cpp銆乭oughcircles.cpp
// THEORY: docs/ch02_imgproc.md 搂5
// TASK: 涓€兼护娉㈠悗 HoughCircles 鐢诲渾
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int dp = 1;
static int minDist = 50;
static int thresh = 80;
static int minR = 10;
static int maxR = 80;

static void onTrack(int, void*) {
    Mat smooth;
    medianBlur(gray, smooth, 5);
    std::vector<Vec3f> circles;
    HoughCircles(smooth, circles, HOUGH_GRADIENT, dp,
                 minDist, thresh, 30, minR, maxR);
    Mat dst = src.clone();
    for (size_t i = 0; i < circles.size(); ++i) {
        Point c(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int r = cvRound(circles[i][2]);
        circle(dst, c, r, Scalar(0, 255, 0), 2, LINE_AA);
        circle(dst, c, 2, Scalar(0, 0, 255), 3, LINE_AA);
    }
    logInfo("circles=%zu", circles.size());
    imshow("L2_16 HoughCircles", dst);
}

int main() {
    src = imread(getImagePath("OIP.png"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_16 HoughCircles", WINDOW_AUTOSIZE);
    createTrackbar("dp",      "L2_16 HoughCircles", &dp, 5, onTrack);
    createTrackbar("minDist", "L2_16 HoughCircles", &minDist, 200, onTrack);
    createTrackbar("thresh",  "L2_16 HoughCircles", &thresh, 300, onTrack);
    createTrackbar("minR",    "L2_16 HoughCircles", &minR, 200, onTrack);
    createTrackbar("maxR",    "L2_16 HoughCircles", &maxR, 300, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
