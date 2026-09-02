// LEARN: L4 浜鸿劯浜斿畼锛堢溂/榧?鍢达級绾ц仈閾?// OFFICIAL: samples/cpp/facial_features.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.5
// TASK: 鐢?FaceDetectorYN(yunet) 鎵捐劯+鍏抽敭鐐癸紱鏃犵溂鐫?榧?鍢?xml 鏃舵墦鍗拌鏄庤烦杩囧祵濂楃骇鑱?#include <opencv2/opencv.hpp>
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

    // 鐪?榧?鍢寸骇鑱?xml 璺緞锛堟寜 facial_features.cpp 鐨勮璁″懡鍚嶏級
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
        // 鍦?face ROI 鍐呰皟鐢ㄥ祵濂楃骇鑱旓紙濡?haarcascade_eye.xml 缂哄け鍒欒烦杩囧搴旈儴浠讹級
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
