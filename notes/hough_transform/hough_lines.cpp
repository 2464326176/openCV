//********************
// Author:  yh
// Time:    2022/8/5.
//  Hough line detection: standard HoughLines (polar) vs probabilistic HoughLinesP (segments)
//  Workflow: grayscale -> Canny edges -> vote in polar parameter space -> back-project to image lines
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat g_src, g_gray, g_canny, g_dst;

// Standard Hough: returns (rho, theta) parameter pairs; lines must be drawn manually
void onHoughLines(int, void *) {
    Mat dst = Mat::zeros(g_src.size(), CV_8UC3);
    g_canny.copyTo(dst);                     // use edge image as background
    cvtColor(dst, dst, COLOR_GRAY2BGR);

    vector<Vec2f> lines;
    HoughLines(g_canny, lines, 1, CV_PI / 180, 100, 0, 0);

    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0], theta = lines[i][1];
        // Compute two endpoints on the line from (rho, theta)
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
        Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
        line(dst, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);
    }
    imshow("hough lines", dst);
}

// Probabilistic Hough: returns segment endpoints (x1,y1,x2,y2); draw directly
void onHoughLinesP(int, void *) {
    Mat dst = Mat::zeros(g_src.size(), CV_8UC3);
    g_canny.copyTo(dst);
    cvtColor(dst, dst, COLOR_GRAY2BGR);

    vector<Vec4i> lines;
    HoughLinesP(g_canny, lines, 1, CV_PI / 180, 100, 50, 10);

    for (size_t i = 0; i < lines.size(); i++) {
        Vec4i l = lines[i];
        line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 0), 2, LINE_AA);
    }
    imshow("hough lines P", dst);
}

int main(int argc, char **argv) {
    String imageName("../data/images/OIP.png");
    if (argc > 1) imageName = argv[1];

    g_src = imread(imageName);
    if (g_src.empty()) { cout << "could not load image" << endl; return -1; }

    cvtColor(g_src, g_gray, COLOR_BGR2GRAY);
    Canny(g_gray, g_canny, 50, 150, 3);     // Hough input must be a binary edge map

    onHoughLines(0, 0);
    onHoughLinesP(0, 0);

    imshow("canny", g_canny);
    waitKey(0);
    return 0;
}
