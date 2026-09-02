// LEARN: L1 绾挎€ф贩鍚?// OFFICIAL: samples/cpp/tutorial_code/core/AddingImages/AddingImages.cpp銆丅asicLinearTransforms.cpp
// THEORY: docs/ch01_core.md 搂2.17 绾挎€ф贩鍚?// TASK: addWeighted 娣峰悎涓ゅ紶鍥撅紱convertScaleAbs 瀹炵幇浜害瀵规瘮搴﹀彉鎹紱婊戝姩鏉?alpha
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat a, bResized;
static int g_alpha = 50; // 瀹為檯涔?0.01

static void onTrack(int, void*) {
    double w = g_alpha / 100.0;
    // 璺緞 1: addWeighted 绾挎€ф贩鍚堜袱寮犲浘
    Mat blend;
    addWeighted(a, w, bResized, 1.0 - w, 0, blend);

    // 璺緞 2: convertScaleAbs 鍗曞浘浜害/瀵规瘮搴﹀彉鎹?    //   dst = alpha * src + beta
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
    resize(b, bResized, a.size()); // addWeighted 瑕佹眰涓ゅ浘灏哄/绫诲瀷涓€鑷?
    namedWindow("a", WINDOW_AUTOSIZE);
    createTrackbar("alpha x0.01", "a", &g_alpha, 100, onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
