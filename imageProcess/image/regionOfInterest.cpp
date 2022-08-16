/*
 * @Author: yh
 * @Date: 2022/8/4 22:48
 * @Description: 
 * @FilePath: regionOfInterest.cpp
 */
#include <opencv2/opencv.hpp>

using namespace cv;

int main() {
    Mat img = imread("../static/data/lena_tmpl.jpg");
    Mat tmpl = imread("../static/data/tmpl.png");
    Mat mask = imread("../static/data/mask.png");
    Mat mask0 = imread("../static/data/mask.png", IMREAD_GRAYSCALE);
    Mat res;

//    int method = 3; // default 3 (CV_TM_CCORR_NORMED)
//    matchTemplate(img, tmpl, res, method, mask);
//
//    double minVal, maxVal;
//    Point minLoc, maxLoc;
//    Rect rect;
//    minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);
//
//    if(method == TM_SQDIFF || method == TM_SQDIFF_NORMED)
//        rect = Rect(minLoc, tmpl.size());
//    else
//        rect = Rect(maxLoc, tmpl.size());
//
//    rectangle(img, rect, Scalar(0, 255, 0), 2);
//
//    imshow("detected template", img);
//    waitKey();
//    waitKey(0);
    Point2i p2i;
    p2i.x = img.rows / 2 - mask.rows / 2;
    p2i.y = img.cols / 2 - mask.cols / 2;
//    Mat imageROI = img(Rect(p2i.x, p2i.y, mask.cols, mask.rows));
    Mat imageROI = img(Rect(p2i.x, p2i.y, mask.rows, mask.cols));
//    Mat imageROI = img(Range(p2i.x, p2i.x + mask.rows), Range(p2i.y, p2i.x + mask.cols));
    mask.copyTo(imageROI, mask0);
    imshow("img", img);
    waitKey();
    return 0;
}