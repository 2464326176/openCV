// LEARN: L2 HoughLines standard Hough line
// OFFICIAL: samples/cpp/tutorial_code/ImgTrans/HoughLines_Demo.cpp、houghlines.cpp
// THEORY: docs/ch02_imgproc.md §5
// TASK: Canny edge then HoughLines draw polar lines
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src, gray;
static int lowTh = 50;
static int thresh = 50;

static void onTrack(int, void*) {
    Mat edges;
    Canny(gray, edges, lowTh, lowTh * 3, 3);
    std::vector<Vec2f> lines;
    HoughLines(edges, lines, 1, CV_PI / 180, thresh, 0, 0);
    Mat dst = src.clone();
    for (size_t i = 0; i < lines.size(); ++i) {
        float rho = lines[i][0], theta = lines[i][1];
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * a));
        Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * a));
        line(dst, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
    }
    logInfo("lines=%zu", lines.size());
    Mat up;
    hconcat(src, dst, up);
    hconcat(up, edges, up);
    imshow("L2_14 HoughLines", up);
}

int main() {
    src = imread(getImagePath("VCG3.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cvtColor(src, gray, COLOR_BGR2GRAY);
    namedWindow("L2_14 HoughLines", WINDOW_AUTOSIZE);
    createTrackbar("canny low", "L2_14 HoughLines", &lowTh, 200, onTrack);
    createTrackbar("hough th",  "L2_14 HoughLines", &thresh, 200, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
