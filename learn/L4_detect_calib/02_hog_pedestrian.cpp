// LEARN: L4 HOG 琛屼汉妫€娴?// OFFICIAL: samples/cpp/peopledetect.cpp, train_HOG.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.6
// TASK: HOGDescriptor + getDefaultPeopleDetector + setSVMDetector + detectMultiScale 妫€娴嬭浜虹敾妗?#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("VCG1.jpg"));
    if (img.empty()) { logInfo("imread failed: VCG1.jpg"); return -1; }
    dbgMatInfo("img", img);

    // 琛屼汉鍥鹃€氬父杈冨ぇ锛孒OG 榛樿绐楀彛 64x128锛涘厛缂╁埌鍚堢悊澶у皬鍐嶆娴?    Mat small;
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
