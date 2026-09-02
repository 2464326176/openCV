// LEARN: L4 ArUco marker detection
// OFFICIAL: tutorial_code/objectDetection/detect_markers.cpp, detect_board.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.15
// TASK: complete pipeline: generate DICT_4X4_50 marker image, then ArucoDetector::detectMarkers + drawDetectedMarkers
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv_utils.h>

using namespace cv;
using namespace cv::aruco;

int main() {
    // 1) synthesize an image containing 1 ArUco marker
    Dictionary dict = getPredefinedDictionary(DICT_4X4_50);
    const int side = 300, pad = 60;
    Mat marker, canvas(Size(side + 2 * pad, side + 2 * pad), CV_8UC1, Scalar(255));
    generateImageMarker(dict, 7, side, marker);
    Mat roi = canvas(Rect(pad, pad, side, side));
    marker.copyTo(roi);
    cvtColor(canvas, canvas, COLOR_GRAY2BGR);
    dbgMatInfo("canvas", canvas);

    // 2) 检测
    ArucoDetector detector(dict);
    std::vector<std::vector<Point2f>> corners, rejected;
    std::vector<int> ids;
    detector.detectMarkers(canvas, corners, ids, rejected);
    logInfo("detected %d markers, rejected %d", (int)ids.size(), (int)rejected.size());

    // 3) draw detection results (green box + id label)
    if (!ids.empty()) {
        drawDetectedMarkers(canvas, corners, ids, Scalar(0, 255, 0));
        for (size_t i = 0; i < ids.size(); ++i) {
            Point c(0, 0);
            for (const auto& p : corners[i]) c += Point((int)p.x, (int)p.y);
            c /= 4;
            putText(canvas, std::to_string(ids[i]), c - Point(15, 0),
                    FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        }
    }
    dbgShow("L4_05 aruco detect", canvas);
    return 0;
}
