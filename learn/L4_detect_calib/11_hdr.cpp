// LEARN: L4 HDR high dynamic range imaging
// OFFICIAL: tutorial_code/photo/hdr_imaging/hdr_imaging.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.28
// TASK: demonstrate MergeDebevec + Tonemap with synthetic multi-exposure sequence; wider exposure spread yields larger dynamic range
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat base = imread(getImagePath("lena.jpg"));
    if (base.empty()) base = makeSyntheticTestImage();
    if (base.channels() == 1) cvtColor(base, base, COLOR_GRAY2BGR);
    base.convertTo(base, CV_32F, 1.0 / 255.0);

    std::vector<Mat> images;
    std::vector<float> times = {0.5f, 1.0f, 2.0f, 4.0f};
    for (float t : times) {
        Mat exposed;
        pow(base, 1.0 / t, exposed);
        min(exposed, 1.0, exposed);
        images.push_back(exposed);
    }

    Ptr<MergeDebevec> merge = createMergeDebevec();
    Mat hdr;
    merge->process(images, hdr, times);

    Ptr<Tonemap> tonemap = createTonemap(2.2f);
    tonemap->setGamma(2.2f);
    Mat ldr;
    tonemap->process(hdr, ldr);
    ldr.convertTo(ldr, CV_8U, 255);

    logInfo("synthetic HDR: %zu exposures, tonemap gamma=2.2", images.size());
    dbgShowMany({"input", "tonemapped HDR"}, {images[1], ldr}, 0);
    return 0;
}
