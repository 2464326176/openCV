// LEARN: L1 linear blending

// OFFICIAL: samples/cpp/tutorial_code/core/AddingImages/AddingImages.cpp、BasicLinearTransforms.cpp
// THEORY: docs/ch01_core.md §2.17 linear blending

// TASK: blend two images with addWeighted; brightness/contrast with convertScaleAbs; trackbar for alpha
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat a, bResized;
static int g_alpha = 50; // actual value multiplied by 0.01

static void onTrack(int, void*) {
    double w = g_alpha / 100.0;
    // Path 1: addWeighted linear blend of two images
    Mat blend;
    addWeighted(a, w, bResized, 1.0 - w, 0, blend);

    // Path 2: convertScaleAbs single-image brightness/contrast transform
    //   dst = alpha * src + beta
    Mat bright;
    convertScaleAbs(a, bright, 1.5, 30);

    dbgPrint("alpha", w);
    dbgShowMany({"a", "b", "blend", "bright"},
                {a, bResized, blend, bright}, 0);
}

int main() {
    a = imread(getImagePath("lena.jpg"));
    Mat b = imread(getImagePath("VCG1.jpg"));
    if (a.empty() || b.empty()) { logInfo("imread failed"); return -1; }
    resize(b, bResized, a.size()); // addWeighted requires same size/type
    namedWindow("a", WINDOW_AUTOSIZE);
    createTrackbar("alpha x0.01", "a", &g_alpha, 100, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
