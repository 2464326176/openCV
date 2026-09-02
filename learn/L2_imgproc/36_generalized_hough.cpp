// LEARN: L2 Generalized Hough transform
// OFFICIAL: tutorial_code/ImgTrans/generalizedHoughTransform.cpp
// THEORY: docs/ch02_imgproc.md §5
// TASK: detect template on edge image with GeneralizedHoughBallard; param dp/minDist affects peak detection
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"), IMREAD_GRAYSCALE);
    if (src.empty()) src = makeSyntheticTestImage();
    if (src.channels() > 1) cvtColor(src, src, COLOR_BGR2GRAY);

    Mat templ = src(Rect(src.cols / 4, src.rows / 4, src.cols / 4, src.rows / 4)).clone();
    Mat edges, templEdges;
    Canny(src, edges, 50, 150);
    Canny(templ, templEdges, 50, 150);

    Ptr<GeneralizedHoughBallard> gh = createGeneralizedHoughBallard();
    gh->setTemplate(templEdges);
    gh->setCannyLowThresh(50);
    gh->setCannyHighThresh(150);
    gh->setDp(1);
    gh->setMinDist(10);

    std::vector<Vec4i> positions;
    gh->detect(edges, positions);
    logInfo("GeneralizedHoughBallard found %zu candidates", positions.size());

    Mat vis;
    cvtColor(src, vis, COLOR_GRAY2BGR);
    for (const auto& p : positions) {
        circle(vis, Point(p[0], p[1]), 8, Scalar(0, 255, 0), 2);
    }
    rectangle(vis, Rect(src.cols / 4, src.rows / 4, src.cols / 4, src.rows / 4),
              Scalar(255, 0, 0), 2);
    dbgShow("L2_36 generalized hough", vis, 0);
    return 0;
}
