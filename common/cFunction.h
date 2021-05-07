//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//

#ifndef OPENCV_CFUNCTION_H
#define OPENCV_CFUNCTION_H
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


void echoPrint(const char *str);
void colorReduce(Mat& inputImage, Mat& outputImage, int div);


#endif //OPENCV_CFUNCTION_H
