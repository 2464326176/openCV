// LEARN: L5 G-API application pipeline
// OFFICIAL: samples/cpp/tutorial_code/gapi/face_beautification/face_beautification.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md §G-API
// TASK: Declarative graph: grayscale -> bilateral filter -> Canny, compare with imperative approach
#include <opencv2/opencv.hpp>
#include <opencv2/gapi.hpp>
#include <opencv2/gapi/core.hpp>
#include <opencv2/gapi/imgproc.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) src = makeSyntheticTestImage();

    GMat in;
    GMat gray = gapi::BGR2Gray(in);
    GMat smooth = gapi::bilateralFilter(gray, -1, 25, 25);
    GMat edges = gapi::Canny(smooth, 50, 150);
    GComputation comp(GIn(in), GOut(edges));

    Mat gapiOut;
    comp.apply(gin(src), gout(gapiOut));

    Mat grayMat, smoothMat, cmdOut;
    cvtColor(src, grayMat, COLOR_BGR2GRAY);
    bilateralFilter(grayMat, smoothMat, -1, 25, 25);
    Canny(smoothMat, cmdOut, 50, 150);

    logInfo("G-API graph: BGR2Gray -> bilateralFilter -> Canny");
    dbgShowMany({"gapi", "cmd"}, {gapiOut, cmdOut}, 0);
    return 0;
}
