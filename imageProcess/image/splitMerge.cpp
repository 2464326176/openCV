/*
 * @Author: yh
 * @Date: 2022/8/5 0:37
 * @Description: 
 * @FilePath: splitMerge.cpp
 */
#include <opencv2/opencv.hpp>

using namespace cv;

int main() {
    //Mat src = imread("../static/data/HappyFish.jpg");
    Mat src = imread("../static/data/fruits.jpg");
    std::vector<Mat> dest;
    split(src, dest);

    imshow("blur", dest[0]);
    imshow("green", dest[1]);
    imshow("red", dest[2]);

    waitKey(0);
    return 0;
}
