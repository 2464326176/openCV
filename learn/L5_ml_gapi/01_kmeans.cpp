// LEARN: L5 kmeans 棰滆壊鑱氱被
// OFFICIAL: samples/cpp/kmeans.cpp
// THEORY: docs/ch05_ml.md 搂鑱氱被
// TASK: 瀵瑰浘鍍忓儚绱犲仛 kmeans(K=8) 棰滆壊閲忓寲锛屾樉绀洪噺鍖栫粨鏋?
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;

int main() {
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat data;
    src.convertTo(data, CV_32F);
    data = data.reshape(1, src.rows * src.cols); // 姣忚涓€涓儚绱?

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
