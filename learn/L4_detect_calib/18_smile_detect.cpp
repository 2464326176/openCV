// LEARN: L4 寰瑧妫€娴?// OFFICIAL: samples/cpp/smiledetect.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.4
// TASK: 鐢?FaceDetectorYN(yunet) 鎵捐劯锛涙棤 smile xml 鏃舵墦鍗拌鏄庤烦杩囧井绗戝己搴﹁瘎浼?#include <opencv2/opencv.hpp>
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

    CascadeClassifier smile;
    bool hasSmile = smile.load("../mingw-build/opencv_sources/data/haarcascades/haarcascade_smile.xml");
    logInfo("smile cascade load=%d", hasSmile ? 1 : 0);

    for (int i = 0; i < faces.rows; ++i) {
        float* f = faces.ptr<float>(i);
        Rect face(cvRound(f[0]), cvRound(f[1]), cvRound(f[2]), cvRound(f[3]));
        rectangle(img, face, Scalar(0, 255, 0), 2);
        // 鍢撮儴 ROI锛氬彇 face 涓嬪崐閮ㄥ垎
        Rect mouthRoi = Rect(face.x, face.y + face.height * 2 / 3,
                             face.width, face.height / 3) & Rect(0, 0, img.cols, img.rows);
        rectangle(img, mouthRoi, Scalar(255, 0, 0), 1);
        if (!hasSmile) continue;
        std::vector<Rect> smiles;
        Mat mroi = img(mouthRoi);
        smile.detectMultiScale(mroi, smiles, 1.22, 0, 0, Size(20, 20));
        int I = smiles.empty() ? 0 : 255;     // 绠€鍖栫増鈥滃己搴︽潯鈥?        rectangle(img, Rect(face.x, face.y - 12, I, 10), Scalar(0, 0, I), -1);
        logInfo("face %d: %d smile candidates", i, (int)smiles.size());
    }
    if (!hasSmile) logInfo("no haarcascade_smile.xml -> skipped smile classifier");
    dbgShow("L4_18 smile", img);
    return 0;
}
