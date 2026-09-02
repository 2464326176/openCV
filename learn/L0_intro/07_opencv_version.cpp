// LEARN: L0 OpenCV 鐗堟湰涓庢瀯寤轰俊鎭?
// OFFICIAL: samples/cpp/opencv_version.cpp
// THEORY: docs/ch01_core.md 搂1.0
// TASK: 鎵撳嵃 OpenCV 鐗堟湰銆佹瀯寤轰俊鎭紱璇存槑褰撳墠鐜鏄惁婊¤冻鍚庣画缁冧範
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    logInfo("OpenCV version : %s", CV_VERSION);
    logInfo("Major.Minor.Patch: %d.%d.%d", CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_VERSION_STATUS);
    logInfo("Build info:\n%s", getBuildInformation().c_str());
    logInfo("Project root   : %s", getProjectRoot().c_str());
    logInfo("Data root      : %s", getDataRoot().c_str());
    logInfo("Models root    : %s", getModelsRoot().c_str());

    Mat probe = imread(getImagePath("lena.jpg"));
    if (probe.empty()) {
        probe = makeSyntheticTestImage(320, 240);
        logInfo("sample image   : synthetic fallback (data/images/lena.jpg missing)");
    } else {
        logInfo("sample image   : %s", getImagePath("lena.jpg").c_str());
    }
    dbgMatInfo("probe", probe);
    dbgShow("L0_07 opencv_version", probe, 1);
    return 0;
}
