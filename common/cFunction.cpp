//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//

#include "cFunction.h"

void echoPrint(const char *str){
    printf("%s\n", str);
    return ;
};


void colorReduce(Mat& inputImage, Mat& outputImage, int div)
{
    //参数准备
    outputImage = inputImage.clone(); //拷贝实参到临时变量
    int rowNumber = outputImage.rows;//行数
    int colNumber = outputImage.cols*outputImage.channels();//列数 x 通道数=每一行元素的个数

    for (int i = 0; i < rowNumber; i++)
    {
        uchar* data = outputImage.ptr<uchar>(i);//获取第i行的首地址
        for (int j = 0; j < colNumber; j++)
        {
            // ---------【开始处理每个像素】-------------
            data[j] = data[j] / div * div ;
        }

    }

    return ;
}
