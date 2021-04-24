//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//

#include <opencv2/opencv.hpp>
#include <String>
#include <cstdlib>
#include <ctime>


using namespace cv;
using namespace std;
#define WINDOW_NAME "opencv operateMouse"


Rect g_rectangle;
bool g_bDrawingBox = false;
RNG g_rng(12345);


void DrawRectangle(cv::Mat &img, cv::Rect box) {
    rectangle(img, box.tl(), box.br(), Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255)));
}




void onMouseHandle(int event, int x, int y, int flags, void* param) {
    Mat& image = *(Mat *) param;
    std::cout << "x:" << x<< "y" << y << std::endl;
    switch(event) {
        case EVENT_MOUSEMOVE:
        {
            if(g_bDrawingBox) {
                g_rectangle.width = x - g_rectangle.x;
                g_rectangle.height = y - g_rectangle.y;
            }

        }
            break;
        case EVENT_LBUTTONDOWN:
        {
            g_bDrawingBox = true;
            g_rectangle = Rect (x, y, 0, 0);
        }
            break;
        case EVENT_LBUTTONUP:
        {
            g_bDrawingBox = false;
            if(g_rectangle.width < 0) {
                g_rectangle.x += g_rectangle.width;
                g_rectangle.width *= -1;
            }
            if(g_rectangle.height < 0) {
                g_rectangle.y += g_rectangle.height;
                g_rectangle.height *= -1;
            }

            DrawRectangle(image, g_rectangle);
        }
            break;
    }
}



int operateMouse() {
    g_rectangle = Rect(-1, -1, 0, 0);

    Mat srcImage(600, 800, CV_8UC3), tempImgae;
    srcImage.copyTo(tempImgae);

    g_rectangle = Rect(-1, -1, 0, 0);
    srcImage = Scalar::all(0);

    namedWindow(WINDOW_NAME);
    setMouseCallback(WINDOW_NAME, onMouseHandle, (void *)&srcImage);
    time_t t;
    srand((unsigned) time(&t));
    while(1) {
        srcImage.copyTo(tempImgae);
        if(g_bDrawingBox) {
            DrawRectangle(tempImgae, g_rectangle);
        }



        DrawRectangle(tempImgae, g_rectangle);
        imshow(WINDOW_NAME, tempImgae);
        if(waitKey(10) == 27) {
            break;
        }
    }
    return 0;
}




