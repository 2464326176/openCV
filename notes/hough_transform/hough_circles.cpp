//********************
// Author:  yh
// Time:    2022/8/5.
//  霍夫圆检测 HoughCircles
//  原理：霍夫梯度法 —— 先 Canny + Sobel 求梯度方向，再在圆心空间投票
//  注意：输入为灰度图；输出向量元素是 Vec3f (x, y, r)
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat g_src, g_gray;
int g_param1 = 100;   // Canny 高阈值（越低圆越多）
int g_param2 = 30;    // 圆心累加器阈值（越低圆越多越杂）

void onHoughCircles(int, void *) {
    Mat dst = g_src.clone();
    vector<Vec3f> circles;

    HoughCircles(g_gray, circles, HOUGH_GRADIENT, 1,
                 g_gray.rows / 16,   // 圆心最小距离：过滤近邻重复圆
                 g_param1, g_param2,
                 20, 80);            // 半径范围 [20, 80]

    for (size_t i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        circle(dst, center, 3, Scalar(0, 255, 0), -1);        // 圆心
        circle(dst, center, radius, Scalar(255, 0, 0), 2);    // 圆周
    }
    imshow("hough circles", dst);
}

int main(int argc, char **argv) {
    String imageName("../data/images/OIP.png");
    if (argc > 1) imageName = argv[1];

    g_src = imread(imageName);
    if (g_src.empty()) { cout << "could not load image" << endl; return -1; }

    cvtColor(g_src, g_gray, COLOR_BGR2GRAY);
    GaussianBlur(g_gray, g_gray, Size(9, 9), 2, 2);   // 平滑抑制噪声，防止误检

    namedWindow("hough circles", WINDOW_AUTOSIZE);
    createTrackbar("param1", "hough circles", &g_param1, 200, onHoughCircles);
    createTrackbar("param2", "hough circles", &g_param2, 200, onHoughCircles);
    onHoughCircles(0, 0);

    waitKey(0);
    return 0;
}
