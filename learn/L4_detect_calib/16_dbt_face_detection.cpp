// LEARN: L4 DetectionBasedTracker 浜鸿劯璺熻釜
// OFFICIAL: samples/cpp/dbt_face_detection.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.3
// TASK: 鑷畾涔?IDetector 瀛愮被鎵胯浇 CascadeClassifier锛涙棤 haarcascade xml 鏃?detect 杩斿洖绌哄苟璇存槑
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/objdetect/detection_based_tracker.hpp>
#include <opencv_utils.h>

using namespace cv;

// 缁ф壙 DetectionBasedTracker::IDetector锛涙棤 xml 鏃?detect 鐩存帴娓呯┖ objects
class HaarIDetector : public DetectionBasedTracker::IDetector {
public:
    HaarIDetector() {
        // 灏濊瘯鍔犺浇 Haar 浜鸿劯绾ц仈锛涙壘涓嶅埌鏂囦欢鍒?cascade 涓虹┖锛宒etect 鏃惰繑鍥炵┖
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
