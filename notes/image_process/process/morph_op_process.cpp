#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;
using namespace std;

Mat srcImage;

struct MorphDemo {
    const char* winName;
    int op;
    int ksize;
};

static int kernelSize(int value) {
    return max(value, 1);
}

void on_morphTrackbar(int value, void* userdata) {
    auto* demo = static_cast<MorphDemo*>(userdata);
    demo->ksize = value;
    Mat kernel = getStructuringElement(MORPH_RECT,
                                       Size(kernelSize(value), kernelSize(value)));
    Mat dst;
    morphologyEx(srcImage, dst, demo->op, kernel);
    imshow(demo->winName, dst);
}

int main() {
    srcImage = imread(getImagePath("OIP.png"));
    if (srcImage.empty()) {
        logInfo("imread failed");
        return -1;
    }

    MorphDemo demos[] = {
        {"dilate image", MORPH_DILATE, 15},
        {"erode image", MORPH_ERODE, 15},
        {"open image", MORPH_OPEN, 15},
        {"close image", MORPH_CLOSE, 15},
        {"gradient image", MORPH_GRADIENT, 15},
        {"tophat image", MORPH_TOPHAT, 15},
        {"blackhat image", MORPH_BLACKHAT, 15},
    };

    namedWindow("ori image", WINDOW_AUTOSIZE);
    imshow("ori image", srcImage);

    for (auto& demo : demos) {
        namedWindow(demo.winName, WINDOW_AUTOSIZE);
        createTrackbar("ksize", demo.winName, nullptr, 50, on_morphTrackbar, &demo);
        setTrackbarPos("ksize", demo.winName, demo.ksize);
    }

    waitKey(0);
    return 0;
}
