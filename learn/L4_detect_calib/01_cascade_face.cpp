// LEARN: L4 cascade face detection
// OFFICIAL: samples/cpp/face_detect.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.1
// TASK: prefer cv::FaceDetectorYN + yunet onnx for face detection; fall back to CascadeClassifier when no onnx/xml
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv_utils.h>

using namespace cv;

static const char* kYuNet = nullptr; // resolved at runtime via getModelPath

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) {
        logInfo("lena.jpg not found, using synthetic image");
        img = makeSyntheticTestImage();
    }
    dbgMatInfo("img", img);

    std::string yunetPath = getModelPath("face_detection_yunet_2023mar.onnx");
    Ptr<FaceDetectorYN> detector = FaceDetectorYN::create(yunetPath, "", Size(320, 320),
                                                           0.6f, 0.3f, 5000);
    Mat faces;
    if (!detector.empty()) {
        detector->setInputSize(img.size());
        detector->detect(img, faces);
        logInfo("FaceDetectorYN detected %d faces", faces.rows);
        for (int i = 0; i < faces.rows; ++i) {
            float* f = faces.ptr<float>(i);
            int x = cvRound(f[0]), y = cvRound(f[1]);
            int w = cvRound(f[2]), h = cvRound(f[3]);
            rectangle(img, Rect(x, y, w, h), Scalar(0, 255, 0), 2);
            // 5 landmarks: left eye/right eye/nose tip/left mouth corner/right mouth corner
            for (int k = 0; k < 5; ++k) circle(img,
                Point(cvRound(f[4 + 2 * k]), cvRound(f[5 + 2 * k])), 2, Scalar(0, 0, 255), -1);
        }
    } else {
        // 2) fallback CascadeClassifier (print info if no haarcascade xml)
        logInfo("YuNet onnx not loaded (%s), fall back to CascadeClassifier", yunetPath.c_str());
        CascadeClassifier cascade;
        std::string cascadePath = getModelPath("haarcascade_frontalface_alt.xml");
        bool ok = cascade.load(cascadePath);
        if (!ok || cascade.empty()) {
            logInfo("no haarcascade xml found, skip cascade detect");
        } else {
            std::vector<Rect> rects;
            Mat gray; cvtColor(img, gray, COLOR_BGR2GRAY);
            cascade.detectMultiScale(gray, rects, 1.1, 3, 0, Size(30, 30));
            logInfo("CascadeClassifier detected %d faces", (int)rects.size());
            for (const auto& r : rects) rectangle(img, r, Scalar(255, 0, 0), 2);
        }
    }
    dbgShow("L4_01 cascade face", img);
    return 0;
}
