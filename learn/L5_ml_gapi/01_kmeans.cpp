// LEARN: L5 kmeans color clustering
// OFFICIAL: samples/cpp/kmeans.cpp
// THEORY: docs/ch05_ml.md §Clustering
// TASK: Perform kmeans (K=8) color quantization on image pixels, display the result
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat data;
    src.convertTo(data, CV_32F);
    data = data.reshape(1, src.rows * src.cols); // one pixel per row

    Mat labels, centers;
    kmeans(data, 8, labels,
           TermCriteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 10, 1.0),
           3, KMEANS_PP_CENTERS, centers);

    Mat quant(src.rows * src.cols, 1, CV_32FC3);
    for (int i = 0; i < src.rows * src.cols; ++i)
        quant.at<Vec3f>(i) = centers.at<Vec3f>(labels.at<int>(i));
    quant = quant.reshape(3, src.rows);
    quant.convertTo(quant, CV_8U);

    dbgShowMany({"src", "kmeans K=8"}, {src, quant}, 0);
    return 0;
}
