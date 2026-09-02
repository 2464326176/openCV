// LEARN: L4 DetectionBasedTracker face tracking
// OFFICIAL: samples/cpp/dbt_face_detection.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.3
// TASK: custom IDetector subclass wrapping CascadeClassifier; detect returns empty when no haarcascade xml
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/objdetect/detection_based_tracker.hpp>
#include <opencv_utils.h>

using namespace cv;

// inherit DetectionBasedTracker::IDetector; detect clears objects when no xml
class HaarIDetector : public DetectionBasedTracker::IDetector {
public:
    HaarIDetector() {
        // attempt to load Haar face cascade; cascade is empty if file not found, detect returns empty
        bool ok = cascade.load("../mingw-build/opencv_sources/data/haarcascades/haarcascade_frontalface_alt.xml");
        logInfo("Haar cascade load=%d", ok ? 1 : 0);
    }
    void detect(const Mat& image, std::vector<Rect>& objects) override {
        objects.clear();
        if (cascade.empty()) return;
        Mat gray;
        if (image.channels() == 3) cvtColor(image, gray, COLOR_BGR2GRAY);
        else                       gray = image;
        equalizeHist(gray, gray);
        cascade.detectMultiScale(gray, objects, getScaleFactor(), getMinNeighbours(), 0,
                                  getMinObjectSize(), getMaxObjectSize());
    }
    CascadeClassifier cascade;
};

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) { logInfo("imread failed: lena.jpg"); return -1; }
    dbgMatInfo("img", img);

    Ptr<HaarIDetector> mainDet = makePtr<HaarIDetector>();
    DetectionBasedTracker::Parameters params;
    DetectionBasedTracker dbt(mainDet, mainDet, params);
    dbt.run();

    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    dbt.process(gray);

    std::vector<Rect> objs;
    dbt.getObjects(objs);
    logInfo("DetectionBasedTracker getObjects -> %d", (int)objs.size());

    Mat out = img.clone();
    for (const auto& r : objs) rectangle(out, r, Scalar(0, 255, 0), 2);
    dbt.stop();
    if (mainDet->cascade.empty()) {
        logInfo("no haarcascade xml -> detect returned 0; supply xml for real run");
    }
    dbgShow("L4_16 dbt", out);
    return 0;
}
