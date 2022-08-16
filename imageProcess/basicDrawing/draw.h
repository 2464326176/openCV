/*
 * @Author: yh
 * @Date: 2022/8/3 23:46
 * @Description: 
 * @FilePath: draw.h
 */

#ifndef IMAGE_IMAGEPROCESS_BASICDRAWING_DRAW_H
#define IMAGE_IMAGEPROCESS_BASICDRAWING_DRAW_H
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;

void drawEllipse(Mat img, double angle);
void drawFilledCircle(Mat img, Point center);
void drawPolygon(Mat img);
void drawLine(Mat img,  Point start, Point end );

#endif //IMAGE_IMAGEPROCESS_BASICDRAWING_DRAW_H
