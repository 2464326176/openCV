// LEARN: L2 Morphology line extraction
// OFFICIAL: samples/cpp/tutorial_code/ImgProc/morph_lines_detection/Morphology_3.cpp、morphology2.cpp
// THEORY: docs/ch02_imgproc.md §2
// TASK: horizontal/vertical kernel line extraction
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("OIP.png"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat gray;
    if (src.channels() == 3) cvtColor(src, gray, COLOR_BGR2GRAY);
    else gray = src;
    Mat bin;
    adaptiveThreshold(~gray, bin, 255,
                      ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

    Mat horiz = bin.clone();
    Mat vert  = bin.clone();
    int sz = 40;
    Mat hElem = getStructuringElement(MORPH_RECT, Size(sz, 1));
    Mat vElem = getStructuringElement(MORPH_RECT, Size(1, sz));
    erode(horiz, horiz, hElem);  dilate(horiz, horiz, hElem);
    erode(vert,  vert,  vElem); dilate(vert,  vert,  vElem);

    Mat up;
    hconcat(bin, horiz, up);
    Mat all;
    hconcat(up, vert, all);
    dbgShow("L2_07 lines bin|H|V", all, 0);
    return 0;
}
