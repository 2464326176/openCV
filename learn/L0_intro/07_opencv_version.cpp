// LEARN: L0 OpenCV version and build info
// OFFICIAL: samples/cpp/opencv_version.cpp
// THEORY: docs/ch01_core.md §1.0
// TASK: print OpenCV version and build info; verify environment for subsequent exercises
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
