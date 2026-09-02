// LEARN: L5 G-API 澹版槑寮忓浘璁＄畻 blur+Canny
// OFFICIAL: samples/cpp/tutorial_code/gapi/doc_snippets/api_ref_snippets.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md 搂G-API
// TASK: 鐢?G-API 鏋勫缓 blur+Canny 璁＄畻鍥撅紝瀵规瘮鍛戒护寮忓啓娉?#include <opencv2/opencv.hpp>
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
