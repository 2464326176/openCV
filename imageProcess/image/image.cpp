/*
 * @Author: yh
 * @Date: 2022/8/5 0:57
 * @Description: 
 * @FilePath: image.cpp
 */
#include <opencv2/opencv.hpp>

using namespace cv;

int main() {
    Mat src = imread("../static/data/lena.jpg");
    Mat winLogo = imread("../static/data/WindowsLogo.jpg");
    Mat linuxLogo = imread("../static/data/LinuxLogo.jpg");
    Mat mask = imread("../static/data/WindowsLogo.jpg", IMREAD_GRAYSCALE);
    Mat linearBlend;
    double alpha = 0.5;
    double beta = 1.0 - alpha;
    std::vector<Mat> dest;
    split(winLogo, dest);
//    imshow("blur", dest[0]);
//    imshow("green", dest[1]);
//    imshow("red", dest[2]);
    Point2i p2i;
    p2i.x = src.rows / 2 - winLogo.rows / 2;
    p2i.y = src.cols / 2 - winLogo.cols / 2;
    Mat imageROI = src(Rect(p2i.x, p2i.y, winLogo.cols, winLogo.rows));

    //addWeighted(imageROI, alpha, dest[0], beta, 0, imageROI);
    winLogo.copyTo(imageROI, dest[1]);
    imshow("blur lena", src);
    waitKey(0);
    return 0;
}
