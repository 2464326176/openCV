// LEARN: L5 G-API declarative graph computation blur+Canny
// OFFICIAL: samples/cpp/tutorial_code/gapi/doc_snippets/api_ref_snippets.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md §G-API
// TASK: Build blur+Canny computation graph with G-API, compare with imperative approach
#include <opencv2/opencv.hpp>
#include <opencv2/gapi.hpp>
#include <opencv2/gapi/core.hpp>
#include <opencv2/gapi/imgproc.hpp>
#include <opencv_utils.h>
using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    GMat in;
    GMat blurred = gapi::blur(in, Size(3,3));
    GMat edges = gapi::Canny(blurred, 50, 150);
    GComputation comp(GIn(in), GOut(edges));

    Mat out;
    comp.apply(gin(gray), gout(out));

    dbgShowMany({"src", "gapi blur+Canny"}, {src, out}, 0);
    return 0;
}
