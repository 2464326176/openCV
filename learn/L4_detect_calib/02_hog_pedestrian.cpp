// LEARN: L4 HOG pedestrian detection
// OFFICIAL: samples/cpp/peopledetect.cpp, train_HOG.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.6
// TASK: HOGDescriptor + getDefaultPeopleDetector + setSVMDetector + detectMultiScale for pedestrian detection
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("VCG1.jpg"));
    if (img.empty()) { logInfo("imread failed: VCG1.jpg"); return -1; }
    dbgMatInfo("img", img);

    // pedestrian images are usually large, HOG default window 64x128; resize to reasonable size first
    Mat small;
    double scale = 640.0 / std::max(img.cols, img.rows);
    resize(img, small, Size(), scale, scale, INTER_AREA);
    dbgMatInfo("small", small);

    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    logInfo("HOG winSize=%dx%d", hog.winSize.width, hog.winSize.height);

    std::vector<Rect> found;
    hog.detectMultiScale(small, found, 0.0, Size(8, 8), Size(16, 16), 1.05, 2.0);
    logInfo("detected %d candidates", (int)found.size());

    Mat out = small.clone();
    for (const auto& r : found) rectangle(out, r.tl(), r.br(), Scalar(0, 255, 0), 2);
    dbgShow("L4_02 hog pedestrian", out);
    return 0;
}
