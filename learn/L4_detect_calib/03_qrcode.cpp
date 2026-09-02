// LEARN: L4 浜岀淮鐮佹娴嬩笌瑙ｇ爜
// OFFICIAL: samples/cpp/qrcode.cpp, barcode.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.7
// TASK: QRCodeDetector::detectAndDecode 浼樺厛鍦?mingw testdata 鐨?link_ocv.jpg 涓婅В鐮侊紱缂哄浘鍒欏洖閫€鍒?VCG1.jpg 骞惰鏄?#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // 浼樺厛鐢?OpenCV 鑷甫 QR 娴嬭瘯鍥撅紙mingw-build 瀹夎鐩綍涓嬶級
    Mat img = imread("../mingw-build/testdata/cv/qrcode/link_ocv.jpg");
    if (img.empty()) {
        logInfo("link_ocv.jpg not found, fallback to VCG1.jpg (likely no QR)");
        img = imread(getImagePath("VCG1.jpg"));
    }
    if (img.empty()) { logInfo("imread failed"); return -1; }
    dbgMatInfo("img", img);

    QRCodeDetector qr;
    Mat points;
    bool ok = qr.detect(img, points);
    logInfo("QR detect() -> %s", ok ? "true" : "false");
    if (!ok) {
        logInfo("no QR code found in this image; supply a real QR image to test decode");
        dbgShow("L4_03 qrcode", img);
        return 0;
    }

    std::string data = qr.detectAndDecode(img, points);
    polylines(img, points, true, Scalar(0, 255, 0), 3);
    putText(img, data.empty() ? std::string("(undecodable)") : data,
            Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
    logInfo("decoded payload length = %d", (int)data.size());
    dbgShow("L4_03 qrcode", img);
    return 0;
}
