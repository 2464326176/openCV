// LEARN: L3 Point-to-Contour Distance pointPolygonTest
// OFFICIAL: samples/cpp/tutorial_code/ShapeDescriptors/pointPolygonTest_demo.cpp
// THEORY: docs/ch03_features.md §contours
// TASK: get largest contour, pointPolygonTest for each pixel, draw inside/edge/outside three colors
#include <opencv2/opencv.hpp>
// #include <opencv_utils.h>  // comment out if this header does not exist

using namespace cv;

int main() {
    Mat src = imread("lena.jpg", IMREAD_GRAYSCALE);  // removed getImagePath()
    if (src.empty()) { 
        printf("imread failed\n"); 
        return -1; 
    }
    resize(src, src, Size(256, 256));
    Mat edge; 
    Canny(src, edge, 100, 200);
    std::vector<std::vector<Point>> conts;
    findContours(edge, conts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (conts.empty()) { 
        printf("no contour\n"); 
        return -1; 
    }

    size_t k = 0; 
    double maxArea = 0;
    for (size_t i = 0; i < conts.size(); ++i) {
        double ai = contourArea(conts[i]);
        if (ai > maxArea) { 
            maxArea = ai; 
            k = i; 
        }
    }

    Mat show = Mat::zeros(src.size(), CV_8UC3);
    for (int y = 0; y < show.rows; ++y) {
        for (int x = 0; x < show.cols; ++x) {
            double d = pointPolygonTest(conts[k], Point2f((float)x, (float)y), true);
            if (d > 0)      
                show.at<Vec3b>(y, x) = Vec3b(0, 100, 0);    // inside - green
            else if (d < 0) 
                show.at<Vec3b>(y, x) = Vec3b(0, 0, 50);     // outside - dark blue
            else            
                show.at<Vec3b>(y, x) = Vec3b(255, 255, 255); // edge - white
        }
    }
    drawContours(show, conts, (int)k, Scalar(0, 255, 255), 2);
    printf("pointPolygonTest rendered\n");
    imshow("L3_14 polygon test", show);
    waitKey(0);
    return 0;
}