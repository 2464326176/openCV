// LEARN: L2 绾挎€у彉鎹笌浼僵鑹?// OFFICIAL: samples/cpp/tutorial_code/ImgProc/BasicLinearTransforms.cpp銆乻nippets/imgproc_applyColorMap.cpp
// THEORY: docs/ch02_imgproc.md 搂1 + 浼僵鑹?// TASK: convertScaleAbs 璋冧寒搴﹀姣斿害锛沘pplyColorMap 澶?LUT
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat src;
static int alpha = 100;  // *0.01
static int beta  = 0;
static int cmap  = 2;    // COLORMAP_JET

static void onTrack(int, void*) {
    Mat lin;
    convertScaleAbs(src, lin, alpha / 100.0, beta);
    Mat color;
    applyColorMap(lin, color, cmap);
    Mat up;
    hconcat(src, color, up);
    imshow("L2_03 linear+colormap", up);
}

int main() {
    src = imread(getImagePath("lena.jpg"), IMREAD_COLOR);
    if (src.empty()) { logInfo("imread failed"); return -1; }
    namedWindow("L2_03 linear+colormap", WINDOW_AUTOSIZE);
    createTrackbar("alpha x0.01", "L2_03 linear+colormap", &alpha, 300, onTrack);
    createTrackbar("beta",        "L2_03 linear+colormap", &beta,  100, onTrack);
    createTrackbar("colormap",    "L2_03 linear+colormap", &cmap,  21,  onTrack);
    onTrack(0, 0);
    waitKey(0);
    return 0;
}
