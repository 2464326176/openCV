# 相机标定与三维重建基础（cameraCalibration）

本节讲解**相机内参标定**的核心流程：棋盘格角点检测 → 亚像素细化 → `calibrateCamera` 求解内参 → 畸变矫正。这是三维重建、AR/VR、视觉测量、自动驾驶感知的前置基础。对应官方示例 [camera_calibration.cpp](../../mingw-build/samples/cpp/tutorial_code/calib3d/camera_calibration/camera_calibration.cpp)。

本目录源码：[calibrate.cpp](calibrate.cpp)（单文件标定演示）。

## 1. 章节文件索引

| 文件 | 主题 |
|------|------|
| [calibrate.cpp](calibrate.cpp) | 棋盘格标定全流程（检测→细化→求解→去畸变） |

## 2. 为什么要标定

相机把三维世界投影到二维图像，其数学模型由**内参**（焦距、主点、畸变）和**外参**（位姿）决定。针孔模型下，世界点 $P = (X, Y, Z)$ 到像素 $p = (u, v)$ 的映射为：

$$
s\begin{bmatrix}u\\v\\1\end{bmatrix}
=
\underbrace{\begin{bmatrix}f_x & 0 & c_x\\0 & f_y & c_y\\0 & 0 & 1\end{bmatrix}}_{\text{内参 } K}
\underbrace{\begin{bmatrix}R & t\end{bmatrix}}_{\text{外参}}
\begin{bmatrix}X\\Y\\Z\\1\end{bmatrix}
$$

- $f_x, f_y$：以像素为单位的焦距（$f_x = f / dx$）
- $c_x, c_y$：主点（光轴与像平面交点）
- $R, t$：相机在世界坐标系的位姿

实际镜头还有**畸变**（径向 $k_1,k_2,k_3$ + 切向 $p_1,p_2$），不标定就无法恢复真实几何关系。

## 3. 标定流程四步

```
① 采集多张不同位姿的棋盘格图像（10~20 张，覆盖不同角度/距离）
        ↓
② 每张图：findChessboardCorners 检测内角点
   → cornerSubPix 亚像素细化（精度关键）
        ↓
③ 构建世界坐标（棋盘格平放于 Z=0 平面，角点间距=squareSize）
   → calibrateCamera 最小化重投影误差求解 K、畸变、R、t
        ↓
④ getOptimalNewCameraMatrix 调整内参 → remap/undistort 去畸变
```

代码来自 [calibrate.cpp](calibrate.cpp)：

```cpp
// ① 检测角点（棋盘格 9x6 内角点）
bool found = findChessboardCorners(gray, boardSize, pointBuf);

// ② 亚像素细化：11x11 窗口内拟合
cornerSubPix(gray, pointBuf, Size(11, 11), Size(-1, -1),
             TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));

// ③ 求解内参：objectPoints=世界坐标，imagePoints=像素坐标
double rms = calibrateCamera(objectPoints, imagePoints, imageSize,
                             cameraMatrix, distCoeffs, rvecs, tvecs);

// ④ 去畸变（先算映射表，视频逐帧 remap 更高效）
Mat newCamMat = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1);
initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), newCamMat,
                        imageSize, CV_32FC1, map1, map2);
remap(raw, undist, map1, map2, INTER_LINEAR);
```

## 4. 关键参数与易错点

| 项目 | 说明 |
|------|------|
| `boardSize` | 内角点个数（9x6），**不是**格子个数，外圈棋盘格不算 |
| `squareSize` | 格子物理边长，只需各图一致，单位任意（mm 即可） |
| 样本数量 | ≥ 3 张可求解，实际建议 10~20 张不同位姿 |
| `cornerSubPix` | 不做会降低一个量级的精度，标定前必做 |
| 重投影误差 RMS | 期望 < 0.5 像素，过大说明某张图角点检错或输入位姿太单一 |
| `alpha=1` | 保留全部原图像素；`alpha=0` 则裁剪黑边只留有效区域 |

**易错点**：
- 所有样本的棋盘格必须完整可见，不能有遮挡。
- 位姿变化要"够"：只在同一位置平移无法解出焦距，必须旋转棋盘格。
- 分辨率用图时保持一致；`imageSize` 来自第一张图，混用尺寸会报错。

## 5. 延伸：标定结果能做什么

- **去畸变**：`undistort` / `remap`，矫正镜头桶形/枕形畸变。
- **测距与测量**：标定后图像像素可换算为真实物理尺寸。
- **PnP 位姿估计**：已知 3D 模型点与其 2D 投影，用 `solvePnP` 求物体位姿（官方 `real_time_pose_estimation` 示例）。
- **立体视觉**：双目标定求基线 `t`，配合视差图实现深度估计（三维重建核心）。

## 6. 相关官方示例

- [camera_calibration.cpp](../../mingw-build/samples/cpp/tutorial_code/calib3d/camera_calibration/camera_calibration.cpp)：完整标定（支持棋盘格/圆点/ArUco，YAML 配置驱动）
- [real_time_pose_estimation](../../mingw-build/samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/)：标定后做 6DoF 位姿估计
- [calibrate_camera.cpp](../../mingw-build/samples/cpp/tutorial_code/objectDetection/calibrate_camera.cpp)：ArUco 标定
