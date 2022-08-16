//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//
#include "units.h"

void echoPrint(const char *str){
    printf("%s\n", str);
    return ;
}

void displayMat(std::string winName, Mat m) {
    namedWindow(winName, WINDOW_AUTOSIZE );
    imshow(winName, m);
}