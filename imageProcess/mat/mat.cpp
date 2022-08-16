/*
 * @Author: yh
 * @Date: 2022/8/2 0:46
 * @Description: 
 * @FilePath: mat.cpp
 */
#include "mat.h"
#include <iostream>
//#include "units.h"
using namespace std;

int mat_text() {
    // 1 CV_[位数][带符号与否]C[通道数]
    Mat M(2, 2, CV_8UC3, Scalar(0, 0, 255));
    cout << "M= " << M << endl;

    // 2 创建超过两维的矩阵， 指定维数，然后传递一个指向数组的指针，这个数组包含着每个维度的尺寸
//    int matSize[3]{2, 2, 2};
//    Mat L(3, matSize, CV_8UC3, Scalar::all(0));
//    cout << "L= " << L << endl;

    //3
    Mat img = imread("../static/data/lena.jpg");
    Mat mtx(img);
    cout << "mtx= " << mtx << endl;
    // 4
    M.create(4, 4, CV_8UC(2));
    cout << "M= " << M << endl;
    //5
    Mat E = Mat::eye(4, 4, CV_64F);
    Mat O = Mat::ones(4, 4, CV_64F);
    Mat Z = Mat::zeros(4, 4, CV_64F);
    cout << "E= " << E << endl;
    cout << "O= " << O << endl;
    cout << "Z= " << Z << endl;
    // 6
    Mat C = (Mat_<double>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    cout << "C= " << C << endl;
    //7
    Mat RowClone = C.row(1).clone();
    cout << "RowClone= " << RowClone << endl;
    return 0;
}