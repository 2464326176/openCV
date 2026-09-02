//********************
// Author:  yh
// Camera calibration (chessboard): detect corners -> sub-pixel refinement ->
//   calibrateCamera for intrinsics -> undistortion
//  Workflow: read multiple chessboard images from different poses ->
//        findChessboardCorners detection -> cornerSubPix sub-pixel refinement ->
//        calibrateCamera for intrinsics/distortion coeffs ->
//        getOptimalNewCameraMatrix + undistort to remove distortion
//  Official example: tutorial_code/calib3d/camera_calibration/camera_calibration.cpp
//  Usage: pass several chessboard image (or video) paths as arguments
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Number of inner corner points of the chessboard: 9x6 (outer cells are excluded)
const Size boardSize(9, 6);
const float squareSize = 1.0f;   // Physical side length of each cell (unit arbitrary, keep consistent)

static void calcBoardCornerPositions(vector<Point3f>& corners) {
    corners.clear();
    for (int i = 0; i < boardSize.height; i++)
        for (int j = 0; j < boardSize.width; j++)
            corners.push_back(Point3f(j * squareSize, i * squareSize, 0));
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "用法: calibrate <图片1> <图片2> ... 或 <视频文件>" << endl;
        return -1;
    }

    // 1. Collect 3D world coords and 2D pixel projections of all chessboard corners
    vector<vector<Point3f>> objectPoints;   // World coordinates (same for every image)
    vector<vector<Point2f>> imagePoints;    // Pixel coordinates (differ per image)
    vector<Point3f> boardCorners;
    calcBoardCornerPositions(boardCorners);

    Size imageSize;

    // Supports two inputs: multiple images OR a video
    if (argc >= 3 || imread(argv[1], IMREAD_GRAYSCALE).empty()) {
        // Video input: detect per-frame; add to samples on success
        VideoCapture cap(argv[1]);
        if (!cap.isOpened()) { cout << "无法打开视频 " << argv[1] << endl; return -1; }
        Mat frame, gray;
        while (cap.read(frame)) {
            cvtColor(frame, gray, COLOR_BGR2GRAY);
            imageSize = gray.size();
            vector<Point2f> pointBuf;
            bool found = findChessboardCorners(gray, boardSize, pointBuf);
            if (found) {
                // Sub-pixel refinement: improves corner accuracy, which calibration heavily depends on
                cornerSubPix(gray, pointBuf, Size(11, 11), Size(-1, -1),
                             TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
                objectPoints.push_back(boardCorners);
                imagePoints.push_back(pointBuf);
                drawChessboardCorners(frame, boardSize, pointBuf, found);  // visualize
                imshow("chessboard", frame);
                if (waitKey(30) == 27) break;
            }
        }
    } else {
        // Image input
        for (int i = 1; i < argc; i++) {
            Mat img = imread(argv[i], IMREAD_GRAYSCALE);
            if (img.empty()) continue;
            imageSize = img.size();
            vector<Point2f> pointBuf;
            bool found = findChessboardCorners(img, boardSize, pointBuf);
            if (found) {
                cornerSubPix(img, pointBuf, Size(11, 11), Size(-1, -1),
                             TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
                objectPoints.push_back(boardCorners);
                imagePoints.push_back(pointBuf);
            }
        }
    }

    if (objectPoints.size() < 3) {
        cout << "有效标定样本太少（至少 3 张不同位姿的棋盘格）" << endl;
        return -1;
    }
    cout << "收集到 " << objectPoints.size() << " 张有效棋盘格样本" << endl;

    // 2. calibrateCamera: solve intrinsics & distortion coefficients (minimize reprojection error)
    Mat cameraMatrix, distCoeffs;
    vector<Mat> rvecs, tvecs;   // Per-image extrinsics (rotation/translation), solved together during calibration
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize,
                                 cameraMatrix, distCoeffs, rvecs, tvecs);
    cout << "重投影误差 RMS = " << rms << "（越小越好，< 0.5 为佳）" << endl;
    cout << "内参矩阵:\n" << cameraMatrix << endl;
    cout << "畸变系数: " << distCoeffs.t() << endl;

    // 3. Undistort
    // getOptimalNewCameraMatrix adjusts intrinsics for the undistorted FOV; alpha controls retained pixel ratio
    Mat newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1);
    Mat map1, map2;
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), newCameraMatrix,
                            imageSize, CV_32FC1, map1, map2);

    // Demonstrate undistortion with one input image
    Mat raw, undist;
    if (argc >= 3) {
        VideoCapture cap(argv[1]);
        cap.read(raw);
    } else {
        raw = imread(argv[1]);
    }
    if (!raw.empty()) {
        // Method A: remap lookup (efficient, good for per-frame video processing)
        remap(raw, undist, map1, map2, INTER_LINEAR);
        // Method B: undistort in one step (internally equivalent to the above two lines)
        // undistort(raw, undist, cameraMatrix, distCoeffs);
        imshow("raw", raw);
        imshow("undistorted", undist);
        imwrite("../out/calib_undistorted.png", undist);
        waitKey(0);
    }
    return 0;
}
