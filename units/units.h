//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//

#ifndef OPENCV_CFUNCTION_H
#define OPENCV_CFUNCTION_H
#include <opencv2/opencv.hpp>
#include <string>
using namespace cv;


#define IMG_PATH_DATA "../static"

void echoPrint(const char *str);
void displayMat(std::string winName, Mat m);


#endif //OPENCV_CFUNCTION_H
