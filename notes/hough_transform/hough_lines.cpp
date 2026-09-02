//********************
// Author:  yh
// Time:    2022/8/5.
//  霍夫直线检测：标准 HoughLines（极坐标）vs 概率 HoughLinesP（线段）
//  流程：灰度 → Canny 边缘 → 极坐标参数空间投票 → 反变换回图像直线
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat g_src, g_gray, g_canny, g_dst;

// 标准霍夫：返回的是 (rho, theta) 参数对，需要自己画直线
void onHoughLines(int, void *) {
    Mat dst = Mat::zeros(g_src.size(), CV_8UC3);
    g_canny.copyTo(dst);                     // 以边缘图为底
    cvtColor(dst, dst, COLOR_GRAY2BGR);

    vector<Vec2f> lines;
    HoughLines(g_canny, lines, 1, CV_PI / 180, 100, 0, 0);

    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0], theta = lines[i][1];
        // 由 (rho, theta) 求直线上两点
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
        Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
        line(dst, pt1, pt2, Scalar(0, 0, 255), 2, LINE_AA);
    }
    imshow("hough lines", dst);
}

// 概率霍夫：返回的是线段端点 (x1,y1,x2,y2)，直接画
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
    Canny(g_gray, g_canny, 50, 150, 3);     // 霍夫输入必须是二值边缘图

    onHoughLines(0, 0);
    onHoughLinesP(0, 0);

    imshow("canny", g_canny);
    waitKey(0);
    return 0;
}
