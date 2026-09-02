// LEARN: L3 鐐瑰埌杞粨璺濈 pointPolygonTest
// OFFICIAL: samples/cpp/tutorial_code/ShapeDescriptors/pointPolygonTest_demo.cpp
// THEORY: docs/ch03_features.md 搂杞粨
// TASK: 鍙栨渶澶ц疆寤擄紝瀵瑰浘鍍忔瘡涓儚绱?pointPolygonTest锛岀敾 inside/edge/outside 涓夎壊
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    resize(src, src, Size(256, 256));
    Mat edge; Canny(src, edge, 100, 200);
    std::vector<std::vector<Point>> conts;
    findContours(edge, conts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (conts.empty()) { logInfo("no contour"); return -1; }

    size_t k = 0; double a = 0;
    for (size_t i = 0; i < conts.size(); ++i) {
        double ai = contourArea(conts[i]);
        if (ai > a) { a = ai; k = i; }
    }

    Mat show = Mat::zeros(src.size(), CV_8UC3);
    for (int y = 0; y < show.rows; ++y) {
        for (int x = 0; x < show.cols; ++x) {
            double d = pointPolygonTest(conts[k], Point2f((float)x, (float)y), true);
            if (d > 0)      show.at<Vec3b>(y, x) = Vec3b(0, 100, 0);   // 鍐?            else if (d < 0) show.at<Vec3b>(y, x) = Vec3b(0, 0, 50);   // 澶?            else            show.at<Vec3b>(y, x) = Vec3b(255, 255, 255); // 杈?        }
    }
    drawContours(show, conts, (int)k, Scalar(0, 255, 255), 2);
    logInfo("pointPolygonTest rendered");
    dbgShow("L3_14 polygon test", show, 0);
    return 0;
}
