// LEARN: L5 GpuMat GPU 鍩虹
// OFFICIAL: samples/cpp/tutorial_code/gpu/gpu-basics-similarity/gpu-basics-similarity.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md 搂GPU
// TASK: GpuMat 涓婁紶/涓嬭浇/杩愮畻锛涙棤 CUDA 鍒欑紪璇戞湡璺宠繃骞惰鏄?#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
#ifdef HAVE_CUDA
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#endif
using namespace cv;

int main() {
#ifdef HAVE_CUDA
    int n = cuda::getCudaEnabledDeviceCount();
    logInfo("CUDA device count = %d", n);
    if (n <= 0) {
        logInfo("CUDA built but no device available, skip");
        return 0;
    }
    cuda::setDevice(0);
    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }
    cuda::GpuMat gsrc, ggray, gblur;
    gsrc.upload(src);
    cuda::cvtColor(gsrc, ggray, COLOR_BGR2GRAY);
    Ptr<cuda::Filter> bf = cuda::createBoxFilter(ggray.type(), ggray.type(), Size(5,5));
    bf->apply(ggray, gblur);
    Mat out;
    gblur.download(out);
    dbgShowMany({"src", "gpu boxblur gray"}, {src, out}, 0);
#else
    logInfo("OpenCV built without CUDA (HAVE_CUDA not defined).");
    logInfo("GpuMat demo requires opencv_cuda* modules (cudaarithm/cudaimgproc).");
    logInfo("See official gpu-basics-similarity.cpp; rebuild OpenCV WITH_CUDA=ON to enable.");
#endif
    return 0;
}
