// LEARN: L4 facial features (eyes/nose/mouth) cascade chain
// OFFICIAL: samples/cpp/facial_features.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.5
// TASK: use FaceDetectorYN(yunet) to find face+landmarks; skip nested cascade if no eyes/nose/mouth xml
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv_utils.h>

using namespace cv;

static const char* kYuNet = "../models/face_detection_yunet_2023mar.onnx";

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) { logInfo("imread failed: lena.jpg"); return -1; }
    dbgMatInfo("img", img);

    Ptr<FaceDetectorYN> det = FaceDetectorYN::create(kYuNet, "", Size(320, 320), 0.6f, 0.3f, 5000);
    if (det.empty()) { logInfo("YuNet model not loaded"); return -1; }
    det->setInputSize(img.size());

    Mat faces;
    det->detect(img, faces);
    logInfo("FaceDetectorYN -> %d faces", faces.rows);

    // eyes/nose/mouth cascade xml paths (named per facial_features.cpp convention)
    const char* parts[] = {
        "../mingw-build/opencv_sources/data/haarcascades/haarcascade_eye.xml",
        "../mingw-build/opencv_sources/data/haarcascades/haarcascade_mcs_nose.xml",
        "../mingw-build/opencv_sources/data/haarcascades/haarcascade_mcs_mouth.xml"};
    std::vector<CascadeClassifier> partsCls(3);
    bool allEmpty = true;
    for (int i = 0; i < 3; ++i) if (partsCls[i].load(parts[i])) allEmpty = false;

    for (int i = 0; i < faces.rows; ++i) {
        float* f = faces.ptr<float>(i);
        Rect face(cvRound(f[0]), cvRound(f[1]), cvRound(f[2]), cvRound(f[3]));
        rectangle(img, face, Scalar(0, 255, 0), 2);
        for (int k = 0; k < 5; ++k) circle(img,
            Point(cvRound(f[4 + 2 * k]), cvRound(f[5 + 2 * k])), 2, Scalar(0, 0, 255), -1);
        if (allEmpty) continue;
        // invoke nested cascade inside face ROI (skip part if haarcascade_eye.xml missing)
        Mat roi = img(face);
        for (int p = 0; p < 3; ++p) {
            if (partsCls[p].empty()) continue;
            std::vector<Rect> r2;
            partsCls[p].detectMultiScale(roi, r2, 1.2, 5, 0, Size(20, 20));
            for (const auto& rr : r2) rectangle(img, face.tl() + rr.tl(), face.tl() + rr.br(),
                                                Scalar(255, 0, 0), 1);
        }
    }
    if (allEmpty) logInfo("no eyes/nose/mouth xml -> skipped nested cascade, kept YuNet keypoints");
    dbgShow("L4_17 facial_features", img);
    return 0;
}
