// LEARN: L1 Mat creation and types

// OFFICIAL: samples/cpp/tutorial_code/core/mat_the_basic_image_container/mat_the_basic_image_container.cpp、mat_operations.cpp
// THEORY: docs/ch01_core.md §2.1-2.4
// TASK: Mat(type,size) multi-type matrix; clone/copyTo/ROI/isContinuous; modify ROI to verify shared memory
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // Explicitly construct matrices of different types and sizes
    Mat f = Mat(3, 3, CV_32FC1, Scalar(1.5f));
    Mat u = Mat(2, 2, CV_8UC3, Scalar(10, 20, 30));
    Mat z = Mat::zeros(4, 4, CV_8UC1);
    Mat e = Mat::ones(3, 1, CV_64FC1);

    dbgMatInfo("f32", f);
    dbgMatInfo("u8c3", u);
    dbgMatInfo("zeros", z);
    dbgMatInfo("ones64", e);
    dbgPrint("f continuous", (int)f.isContinuous());

    // Reference counting and shallow/deep copy: = is shallow copy, clone() is deep copy
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat shallow = src;            // shares the same memory
    Mat deep    = src.clone();    // deep copy
    Mat copyto;
    src.copyTo(copyto);           // deep copy
    // ROI: Rect creates a view, shares original image memory
    Rect roi(80, 80, 100, 100);
    Mat roiView = src(roi);
    roiView.setTo(Scalar(0, 0, 255)); // modifying ROI modifies the original image
    logInfo("set ROI red -> shallow = src, ROI shares original image memory");
    dbgPixel("src@ROI", src, 130, 130);     // (0,0,255)
    dbgPixel("deep@ROI", deep, 130, 130);   // original value
    dbgPixel("copyto@ROI", copyto, 130, 130); // original value
    dbgPixel("shallow@ROI", shallow, 130, 130); // (0,0,255)

    dbgShowMany({"src(ROI red)", "deep", "copyto"},
                {src, deep, copyto}, 0);
    return 0;
}
