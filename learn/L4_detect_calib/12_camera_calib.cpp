// LEARN: L4 camera calibration (synthetic chessboard)
// OFFICIAL: tutorial_code/calib3d/camera_calibration/camera_calibration.cpp
// THEORY: docs/ch07_calib3d_stitching.md §10
// TASK: generate chessboard -> findChessboardCorners -> calibrateCamera to get K/dist
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv_utils.h>

using namespace cv;

static Mat makeChessboard(const Size& patternSize, int squarePx) {
    Mat img((patternSize.height + 1) * squarePx,
            (patternSize.width + 1) * squarePx, CV_8UC1, Scalar(255));
    for (int y = 0; y <= patternSize.height; ++y) {
        for (int x = 0; x <= patternSize.width; ++x) {
            if ((x + y) % 2 == 0) continue;
            Rect cell(x * squarePx, y * squarePx, squarePx, squarePx);
            rectangle(img, cell, Scalar(0), FILLED);
        }
    }
    return img;
}

int main() {
    const Size patternSize(9, 6);
    Mat board = makeChessboard(patternSize, 40);

    std::vector<Point2f> corners;
    bool found = findChessboardCorners(board, patternSize, corners,
                                       CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
    if (!found) {
        logInfo("findChessboardCorners failed on synthetic board");
        return -1;
    }
    cornerSubPix(board, corners, Size(11, 11), Size(-1, -1),
                 TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));

    std::vector<Point3f> objGrid;
    for (int y = 0; y < patternSize.height; ++y)
        for (int x = 0; x < patternSize.width; ++x)
            objGrid.emplace_back((float)x, (float)y, 0.f);

    std::vector<std::vector<Point3f>> objectPoints = {objGrid};
    std::vector<std::vector<Point2f>> imagePoints = {corners};
    Mat K = Mat::eye(3, 3, CV_64F);
    Mat dist;
    std::vector<Mat> rvecs, tvecs;
    double rms = calibrateCamera(objectPoints, imagePoints, board.size(), K, dist, rvecs, tvecs);
    logInfo("calibrateCamera RMS=%.4f", rms);
    logInfo("K fx=%.1f fy=%.1f cx=%.1f cy=%.1f",
            K.at<double>(0,0), K.at<double>(1,1), K.at<double>(0,2), K.at<double>(1,2));

    Mat vis;
    cvtColor(board, vis, COLOR_GRAY2BGR);
    drawChessboardCorners(vis, patternSize, corners, found);
    dbgShow("L4_12 camera calib", vis, 0);
    return 0;
}
