# OpenCV 学习笔记总览（summary）

本仓库是基于 [mingw-build/samples/cpp](../mingw-build/samples/cpp) 官方示例整理的 OpenCV 中文学习笔记，每个章节包含可运行的源码与配套 `README.md`（原理公式 + 代码逐段解析 + 应用场景）。

## 1. 章节目录

| 章节 | 主题 | 核心内容 |
|------|------|----------|
| [image_process](image_process/README.md) | 图像处理基础 | Mat 容器、图像读写、绘图、滤波、形态学、金字塔、阈值、HDR |
| [image_transformation](image_transformation/README.md) | 图像变换与边缘检测 | Sobel、Scharr、Laplacian、Canny、直方图均衡化、remap |
| [image_segmentation](image_segmentation/README.md) | 图像分割与轮廓 | findContours、drawContours、检索模式、层级结构 |
| [histogram_match](histogram_match/README.md) | 直方图与模板匹配 | calcHist、compareHist、H-S 二维直方图、matchTemplate |
| [highgui](highgui/README.md) | HighGUI 交互 | 窗口管理、Trackbar、回调机制、对比度亮度调节 |
| [harris_detect](harris_detect/README.md) | Harris 角点检测 | 结构张量、角点响应函数、归一化与可视化 |
| [face_detect](face_detect/README.md) | Haar 级联人脸检测 | Haar-like 特征、积分图、AdaBoost 级联、NV21 处理 |
| [features2d](features2d/README.md) | 特征点检测 | 关键点/描述子、SIFT/SURF/ORB 对比、SURF 原理 |
| [hough_transform](hough_transform/README.md) | 霍夫变换 | 参数空间投票、直线/圆检测、概率霍夫 |
| [video_tracking](video_tracking/README.md) | 视频目标跟踪 | LK 光流、CamShift、MOG2 背景减除 |
| [image_stitching](image_stitching/README.md) | 图像拼接 | 特征匹配、RANSAC 单应、Stitcher 全景 |
| [camera_calibration](camera_calibration/README.md) | 相机标定与三维重建基础 | 棋盘格检测、cornerSubPix、calibrateCamera、畸变矫正 |
| [photo_editing](photo_editing/README.md) | 图像编辑与照片特效 | 无缝克隆、保边平滑、素描、风格化、去色 |
| [ml](ml/README.md) | 机器学习：SVM | 线性/非线性 SVM、核技巧、决策边界可视化 |

## 2. 知识体系脉络

```
基础层：Mat 内存模型（矩阵头 + 数据指针 + 引用计数）→ 图像读写 → 像素访问 → 绘图
          │
处理层：滤波降噪（高斯/中值/双边）→ 形态学（腐蚀/膨胀/开闭）→ 金字塔 → 阈值化
          │
分析层：梯度边缘（Sobel/Canny）→ 霍夫直线/圆 → 轮廓（findContours）→ 直方图 → 角点/特征点
          │
应用层：人脸检测（Haar 级联）→ 目标跟踪（光流/CamShift/背景减除）→ 图像拼接 → 相机标定/三维重建 → 图像编辑特效 → 机器学习（SVM）
```

每一层都建立在前一层之上：不理解 Mat 的深浅拷贝，就无法安全地做 ROI 与掩膜；不掌握滤波降噪，Canny 与角点检测就会被噪声干扰；不会轮廓与直方图，特征匹配就无从谈起。

## 3. 章节核心要点速览

| 章节 | 一句话要点 | 必须记住的 API |
|------|-----------|----------------|
| image_process/mat | Mat 是头+数据，ROI 是浅拷贝 | `clone` / `copyTo` / `at<>` / `ptr<>` |
| image_process/image | imread flags 决定通道，BGR 非 RGB | `imread` / `split` / `merge` / `addWeighted` |
| image_process/basic_drawing | 绘图原地写像素，Scalar 是 BGR | `line` / `circle` / `fillPoly` |
| image_process/morphology | 椒盐用中值，保边用双边 | `GaussianBlur` / `medianBlur` / `bilateralFilter` / `morphologyEx` |
| image_transformation | 梯度用 CV_16S 防截断，先降噪再边缘 | `Sobel` / `Canny` / `Laplacian` / `remap` |
| image_segmentation | 轮廓需先二值化，hierarchy 表层级 | `findContours` / `drawContours` |
| histogram_match | HSV 的 H 对光照不敏感 | `calcHist` / `compareHist` / `matchTemplate` |
| highgui | waitKey 不可省，createTrackbar 后手动触发首帧 | `namedWindow` / `createTrackbar` / `setMouseCallback` |
| harris_detect | 响应 R 需归一化才显示，k 取 0.04~0.06 | `cornerHarris` / `goodFeaturesToTrack` |
| face_detect | 预处理三件套：灰度→缩小→均衡化 | `CascadeClassifier::detectMultiScale` |
| features2d | 二进制描述子配 NORM_HAMMING | `ORB::create` / `BFMatcher` / `FlannBasedMatcher` |
| hough_transform | 霍夫输入必须是二值边缘图 | `HoughLines` / `HoughLinesP` / `HoughCircles` |
| video_tracking | LK 光流只跟角点，失败点要剔除 | `calcOpticalFlowPyrLK` / `CamShift` / `createBackgroundSubtractorMOG2` |
| image_stitching | RANSAC 求单应，重叠区需融合 | `findHomography` / `warpPerspective` / `Stitcher` |
| camera_calibration | 标定样本要旋转棋盘格，亚像素细化不可省 | `findChessboardCorners` / `calibrateCamera` / `undistort` |
| photo_editing | 无缝克隆拷梯度而非像素 | `seamlessClone` / `edgePreservingFilter` / `pencilSketch` |
| ml | SVM 只看支持向量，RBF 核处理非线性 | `SVM::train` / `svm->predict` |

## 4. 推荐学习路径

1. **入门**：[image_process/mat](image_process/mat/README.md) → [image_process/image](image_process/image/README.md) → [highgui](highgui/README.md)
2. **图像处理**：[image_process/morphology](image_process/morphology/README.md) → [image_transformation](image_transformation/README.md) → [hough_transform](hough_transform/README.md)
3. **图像分析**：[image_segmentation](image_segmentation/README.md) → [histogram_match](histogram_match/README.md) → [harris_detect](harris_detect/README.md)
4. **高级应用**：[features2d](features2d/README.md) → [image_stitching](image_stitching/README.md) → [video_tracking](video_tracking/README.md) → [face_detect](face_detect/README.md) → [image_process/image_algo](image_process/image_algo/README.md)（HDR）
5. **进阶方向**：[camera_calibration](camera_calibration/README.md)（三维重建基础）→ [photo_editing](photo_editing/README.md)（图像编辑）→ [ml](ml/README.md)（机器学习）

## 5. 构建环境

- 工具链：MinGW（见 `mingw-build` 目录）
- 依赖：OpenCV（含 `opencv_contrib` 以支持 SURF 等 `xfeatures2d` 算法）
- 构建方式参考 [example_cmake](../mingw-build/samples/cpp/example_cmake/CMakeLists.txt)

## 6. 官方资源

- 官方示例源码：[mingw-build/samples/cpp](../mingw-build/samples/cpp)
- 官方教程目录：[tutorial_code](../mingw-build/samples/cpp/tutorial_code)
- 在线文档：`docs.opencv.org`
- 系统化原理详解：[docs/principles.md](../docs/principles.md) 与分章 `ch01`–`ch08`
- 分层速学练习：[learn/](../learn/README.md)（103+ 个最小可运行练习）

## 7. 常见问题与易错点

| 现象 | 原因 | 解决 |
|------|------|------|
| 改 ROI 后原图也变了 | ROI 是浅拷贝，共享 data | 需独立副本用 `clone()` |
| 窗口一闪而过 | 忘记 `waitKey` | GUI 程序必须 `waitKey` 驱动事件循环 |
| 梯度图全黑/花屏 | 负值被截断为 0 | 用 `CV_16S` 再 `convertScaleAbs` |
| Canny 结果碎裂 | 未先降噪 | 先 `GaussianBlur`/`blur` 再 Canny |
| 直方图显示为空 | `ranges` 上界 exclusive 写错 | 用 `[0, 256)` 而非 `[0, 255]` |
| HSV 颜色分割失败 | 用了 0~360 的 H | OpenCV 中 H 范围是 0~179 |
| 角点响应图看不见 | R 值量级极小 | `normalize` 到 [0,255] 再显示 |
| SURF 编译报错 | 未编译 contrib | 需要 `opencv_contrib` 的 `xfeatures2d` |
