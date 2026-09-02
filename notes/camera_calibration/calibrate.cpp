//********************
// Author:  yh
// 相机标定（棋盘格）：检测角点 → 亚像素细化 → calibrateCamera 求内参 → 畸变矫正
//  流程：读入多张不同位姿的棋盘格图 → findChessboardCorners 检测
//        → cornerSubPix 亚像素细化 → calibrateCamera 求内参/畸变系数
//        → getOptimalNewCameraMatrix + undistort 去畸变
//  对应官方示例: tutorial_code/calib3d/camera_calibration/camera_calibration.cpp
//  使用：把若干棋盘格图片（或视频）路径作为参数传入
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// 棋盘格内角点个数：9x6（最外圈的格子不算角点）
const Size boardSize(9, 6);
const float squareSize = 1.0f;   // 每个格子的物理边长（单位任意，保持一致即可）

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

    // 1. 收集所有棋盘格角点的三维坐标（世界坐标）与二维投影（像素坐标）
    vector<vector<Point3f>> objectPoints;   // 世界坐标（每张图相同）
    vector<vector<Point2f>> imagePoints;    // 像素坐标（每张图不同）
    vector<Point3f> boardCorners;
    calcBoardCornerPositions(boardCorners);

    Size imageSize;

    // 支持两种输入：多张图片 或 一段视频
    if (argc >= 3 || imread(argv[1], IMREAD_GRAYSCALE).empty()) {
        // 视频输入：逐帧检测，成功就加入样本
        VideoCapture cap(argv[1]);
        if (!cap.isOpened()) { cout << "无法打开视频 " << argv[1] << endl; return -1; }
        Mat frame, gray;
        while (cap.read(frame)) {
            cvtColor(frame, gray, COLOR_BGR2GRAY);
            imageSize = gray.size();
            vector<Point2f> pointBuf;
            bool found = findChessboardCorners(gray, boardSize, pointBuf);
            if (found) {
                // 亚像素细化：提高角点精度，标定精度强烈依赖于此
                cornerSubPix(gray, pointBuf, Size(11, 11), Size(-1, -1),
                             TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
                objectPoints.push_back(boardCorners);
                imagePoints.push_back(pointBuf);
                drawChessboardCorners(frame, boardSize, pointBuf, found);  // 可视化
                imshow("chessboard", frame);
                if (waitKey(30) == 27) break;
            }
        }
    } else {
        // 图片输入
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

    // 2. calibrateCamera：求解内参矩阵与畸变系数（最小化重投影误差）
    Mat cameraMatrix, distCoeffs;
    vector<Mat> rvecs, tvecs;   // 每张图的外参（旋转/平移），标定过程中同时求出
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize,
                                 cameraMatrix, distCoeffs, rvecs, tvecs);
    cout << "重投影误差 RMS = " << rms << "（越小越好，< 0.5 为佳）" << endl;
    cout << "内参矩阵:\n" << cameraMatrix << endl;
    cout << "畸变系数: " << distCoeffs.t() << endl;

    // 3. 去畸变
    // getOptimalNewCameraMatrix 根据去畸变后的视场调整内参，alpha 控制保留像素比例
    Mat newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1);
    Mat map1, map2;
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), newCameraMatrix,
                            imageSize, CV_32FC1, map1, map2);

    // 用一张输入图演示矫正效果
    Mat raw, undist;
    if (argc >= 3) {
        VideoCapture cap(argv[1]);
        cap.read(raw);
    } else {
        raw = imread(argv[1]);
    }
    if (!raw.empty()) {
        // 方式 A：remap 查表（高效，适合视频逐帧处理）
        remap(raw, undist, map1, map2, INTER_LINEAR);
        // 方式 B：undistort 一步到位（内部等价于上两行）
        // undistort(raw, undist, cameraMatrix, distCoeffs);
        imshow("raw", raw);
        imshow("undistorted", undist);
        imwrite("../out/calib_undistorted.png", undist);
        waitKey(0);
    }
    return 0;
}
