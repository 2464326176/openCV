// LEARN: L4 Charuco board detection
// OFFICIAL: tutorial_code/objectDetection/calibrate_camera_charuco.cpp, detect_board_charuco.cpp
// THEORY: docs/ch06_objdetect_photo.md §6.4
// TASK: synthesize Charuco board and detectBoard; demonstrate detection and corner refinement without real calibration sequence
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_board.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <opencv_utils.h>

using namespace cv;
using namespace cv::aruco;

int main() {
    Dictionary dict = getPredefinedDictionary(DICT_4X4_50);
    Size boardSize(5, 7);
    float squareLength = 0.04f, markerLength = 0.02f;
    CharucoBoard board(boardSize, squareLength, markerLength, dict);

    Mat boardImg;
    board.generateImage(Size(600, 800), boardImg, 10, 1);

    CharucoDetector charucoDetector(board);
    Mat charucoCorners, charucoIds;
    charucoDetector.detectBoard(boardImg, charucoCorners, charucoIds);

    Mat vis;
    cvtColor(boardImg, vis, COLOR_GRAY2BGR);
    if (!charucoIds.empty()) {
        drawDetectedCornersCharuco(vis, charucoCorners, charucoIds, Scalar(0, 255, 0));
    }
    logInfo("charuco corners=%d", charucoIds.rows);
    logInfo("real calibration needs multi-pose Charuco sequence + calibrateCameraCharuco");
    dbgShow("L4_21 charuco detect", vis, 0);
    return 0;
}
