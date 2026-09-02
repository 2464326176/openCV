// LEARN: L0 Drawing primitives
// OFFICIAL: samples/cpp/drawing.cpp, tutorial_code/ImgProc/basic_drawing/Drawing_1.cpp
// THEORY: docs/ch01_core.md §geometric primitives + ch08_gui_gapi_gpu.md
// TASK: draw all primitives: line/rectangle/circle/ellipse/putText/fillPoly/arrowedLine
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat canvas(400, 600, CV_8UC3, Scalar(30, 30, 30));

    line(canvas, Point(20, 20), Point(200, 80), Scalar(0, 0, 255), 2);
    rectangle(canvas, Point(220, 20), Point(360, 100), Scalar(255, 0, 0), 2);
    circle(canvas, Point(450, 60), 40, Scalar(0, 255, 0), -1);
    ellipse(canvas, Point(80, 200), Size(60, 30), 30, 0, 360, Scalar(255, 255, 0), 2);

    Point pts[1][5] = {{{300,160},{360,160},{380,220},{330,250},{290,220}}};
    const Point* p[1] = {pts[0]};
    int npt[] = {5};
    fillPoly(canvas, p, npt, 1, Scalar(0, 255, 255));

    putText(canvas, "Hello OpenCV L0", Point(20, 320),
            FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 2);
    arrowedLine(canvas, Point(20, 360), Point(300, 360),
                Scalar(255, 100, 100), 2);

    dbgShow("L0_04 drawing", canvas, 0);
    return 0;
}