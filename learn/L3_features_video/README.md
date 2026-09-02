# L3 结构与运动

**目标**：吃透角点/描述子检测、特征匹配与单应、轮廓与形状、光流、背景减除、跟踪与 Kalman。本章是从「像素」走向「几何结构」的关键层。

**黄金主线**：02 → 05 → 09 → 10 → 17 → 26

**建议顺序**：01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 → 10 → 11 → 12 → 13 → 14 → 15 → 16 → 17 → 18 → 19 → 20 → 21 → 22 → 23 → 24 → 25 → 26

**验收**：能跑通 ORB + Lowe ratio + RANSAC 单应定位；能用 MOG2 + findContours 提运动目标；能说清「角点为什么适合做光流跟踪点」；能解释 LK 光流的「亮度恒定 + 邻域一致」假设。

## 核心概念速览

| 主题 | 关键词 | 心智模型 |
| --- | --- | --- |
| 角点检测 | Harris/结构张量/响应 R | 梯度两方向都剧烈变化的点 |
| Shi-Tomasi | 最小特征值准则 | Harris 变体，更稳定，LK 常用初值 |
| 亚像素 | cornerSubPix/抛物线插值 | 在像素级精度上再精化半像素 |
| 特征描述子 | ORB/AKAZE/BRIEF/二进制 | 关键点邻域编码为向量供匹配 |
| 匹配 | BF/FLANN/Lowe ratio/KNN | 找最近邻描述子，比值过滤误匹配 |
| 单应 | findHomography/RANSAC | 4 点求 3×3 投影变换，RANSAC 抗外点 |
| 轮廓 | findContours/hierarchy | 二值图边界连通分量树 |
| 形状 | 矩/Hu/凸包/外接 | 把轮廓抽象成可比较的标量/几何 |
| 光流 | LK/Farneback/DIS | 帧间像素位移场 |
| 背景减除 | MOG2/KNN/学习率 | 建模背景并减去得到前景 |
| 跟踪 | MeanShift/CamShift/直方图 | 颜色直方图反向投影 + 均值漂移 |
| Kalman | 预测/更新/状态空间 | 线性高斯最优估计 |
| MSER | 最大化稳定极值区域 | 阈值序列上稳定的连通域 |
| LSD | 线段检测/梯度对齐 | 无参数直线提取 |

## 官方对照表

| 练习文件 | 标签 | 官方 sample | tutorialDoc | 必会 API |
| - | - | - | - | - |
| [01_corner_harris.cpp](01_corner_harris.cpp) | 进阶 | `cornerHarris_Demo.cpp` | ch04 §4.5 | `cornerHarris`（结构张量响应） |
| [02_good_features.cpp](02_good_features.cpp) | 主线 | `goodFeaturesToTrack_Demo.cpp` | ch04 §4.5 | `goodFeaturesToTrack`（Shi-Tomasi） |
| [03_corner_subpix.cpp](03_corner_subpix.cpp) | 进阶 | `cornerSubPix_Demo.cpp` | ch04 §4.5 | `cornerSubPix`（亚像素精化） |
| [04_corner_detector.cpp](04_corner_detector.cpp) | 进阶 | `cornerDetector_Demo.cpp` | ch04 §4.5 | Harris vs Shi-Tomasi 对比 |
| [05_orb_detect_match.cpp](05_orb_detect_match.cpp) | 主线 | `AKAZE_match.cpp`、ORB 片段 | ch03 §描述子 | `ORB::create` / `BFMatcher` |
| [06_akaze_match.cpp](06_akaze_match.cpp) | 进阶 | `AKAZE_match.cpp` | ch03 §描述子 | `AKAZE::create`（非线性尺度） |
| [07_bf_match_lowe.cpp](07_bf_match_lowe.cpp) | 进阶 | `SURF_matching_Demo.cpp` | ch03 §匹配 | `BFMatcher::knnMatch` + ratio |
| [08_flann_match.cpp](08_flann_match.cpp) | 进阶 | `SURF_FLANN_matching_Demo.cpp` | ch03 §匹配 | `FlannBasedMatcher`（KD 树/LSH） |
| [09_homography.cpp](09_homography.cpp) | 主线 | `SURF_FLANN_matching_homography_Demo.cpp` | ch03 §单应 | `findHomography`（RANSAC） + `perspectiveTransform` |
| [10_find_contours.cpp](10_find_contours.cpp) | 主线 | `findContours_demo.cpp` | ch03 §轮廓 | `findContours` / `drawContours` |
| [11_convex_hull.cpp](11_convex_hull.cpp) | 进阶 | `hull_demo.cpp` | ch03 §形状 | `convexHull` |
| [12_bounding_shapes.cpp](12_bounding_shapes.cpp) | 进阶 | `generalContours_demo1.cpp` | ch03 §形状 | `boundingRect` / `minAreaRect` / `fitEllipse` |
| [13_moments_hu.cpp](13_moments_hu.cpp) | 进阶 | `moments_demo.cpp` | ch03 §形状 | `moments` / `HuMoments` |
| [14_point_polygon_test.cpp](14_point_polygon_test.cpp) | 进阶 | `pointPolygonTest_demo.cpp` | ch03 §形状 | `pointPolygonTest`（点对轮廓距离） |
| [15_general_contours.cpp](15_general_contours.cpp) | 进阶 | `generalContours_demo2.cpp` | ch03 §形状 | `approxPolyDP` / `arcLength` / `contourArea` |
| [16_bg_subtract_mog2.cpp](16_bg_subtract_mog2.cpp) | 主线 | `bg_sub.cpp` | ch04 §背景 | `createBackgroundSubtractorMOG2` |
| [17_lk_optical_flow.cpp](17_lk_optical_flow.cpp) | 主线 | `lkdemo.cpp` | ch04 §光流 | `calcOpticalFlowPyrLK`（稀疏） |
| [18_farneback_dense.cpp](18_farneback_dense.cpp) | 进阶 | `optical_flow_dense.cpp`、`fback.cpp` | ch04 §光流 | `calcOpticalFlowFarneback`（稠密） |
| [19_dis_opticalflow.cpp](19_dis_opticalflow.cpp) | 选修 | `dis_opticalflow.cpp` | ch04 §光流 | `DISOpticalFlow::create`（快速） |
| [20_camshift.cpp](20_camshift.cpp) | 进阶 | `camshift.cpp` | ch04 §跟踪 | `CamShift`（自适应窗口） |
| [21_meanshift.cpp](21_meanshift.cpp) | 进阶 | `meanshift.cpp` | ch04 §跟踪 | `meanShift`（固定窗口） |
| [22_kalman.cpp](22_kalman.cpp) | 进阶 | `kalman.cpp` | ch04 §Kalman | `KalmanFilter`（predict/correct） |
| [23_mser.cpp](23_mser.cpp) | 选修 | `detect_mser.cpp` | ch03 §区域 | `MSER::create` |
| [24_blob_lsd.cpp](24_blob_lsd.cpp) | 选修 | `detect_blob.cpp`、`lsd_lines.cpp` | ch03 | `SimpleBlobDetector` / `LineSegmentDetector` |
| [25_homography_decompose.cpp](25_homography_decompose.cpp) | 进阶 | `decompose_homography.cpp` | ch03 §单应 | `decomposeHomographyMat`（多解） |
| [26_lk_stepwise.cpp](26_lk_stepwise.cpp) | 进阶 | `optical_flow.cpp` | ch04 §光流 | 官方分步讲解版 LK |

> **角点检测**官方样例在 `tutorial_code/TrackingMotion/`，文档主章为 [ch04 §4.5](../../docs/ch04_video.md)；轮廓/描述子/单应在 [ch03](../../docs/ch03_features.md)。

## 主题分组与先修关系

```
角点（01-04）           ← 像素级结构，无对应关系
   │
描述子+匹配+单应（05-09）← 需两图；09 是匹配的高潮
   │
轮廓+形状（10-15）       ← 单图几何抽象，与匹配正交
   │
运动（16-22）            ← 需视频/序列；17 LK 用 02 的角点
   │
区域/线段（23-24）       ← 替代结构提取方案
   │
单应分解（25）/ 分步 LK（26）← 进阶深挖
```

- **角点**：01 Harris 原理→02 Shi-Tomasi 实用→03 亚像素→04 对比。角点是光流/标定的初值来源。
- **描述子+匹配**：05 ORB（开源快速）→06 AKAZE（抗模糊）→07 BF+Lowe→08 FLANN→09 单应定位（匹配的高潮）。
- **轮廓+形状**：10 提取→11 凸包→12 外接形状→13 矩/Hu→14 点测试→15 综合分析。把轮廓变成可比较的标量。
- **运动**：16 背景减除→17 稀疏光流→18 稠密光流→19 DIS→20 CamShift→21 MeanShift→22 Kalman。20/21 是跟踪，22 是状态估计。
- **区域/线段**：23 MSER 找稳定文本/牌照区域；24 Blob+LSD 提斑点与直线。
- **进阶**：25 单应分解（3D 位姿多解）；26 官方分步 LK 讲解版，对照 17 学原理。

## 关键参数与易错点

| 练习 | 易错点 / 关键参数 |
| --- | --- |
| 01 Harris | `blockSize`/`ksize`/`k`(0.04~0.06)；响应需 `normalize` 后才好显示 |
| 02 goodFeatures | `maxCorners`/`qualityLevel`(0.01~0.1)/`minDistance`；quality 越大越严 |
| 05 ORB | `nfeatures` 限点数；`BFMatcher` 配 `NORM_HAMMING`（二进制描述子） |
| 07 Lowe | 比值阈值 0.7~0.8；越小越严，误匹配少但召回低 |
| 09 单应 | 至少 4 对点；`RANSAC` 的 `confidence`/`maxIters`；单应只对平面或纯旋转有效 |
| 10 轮廓 | `RETR_TREE`/`RETR_EXTERNAL` 选层级；`CHAIN_APPROX_SIMPLE` 压缩冗余点 |
| 13 矩 | `HuMoments` 需归一化才可跨尺度比较；对平移/旋转/缩放不变 |
| 17 LK | `winSize` 太小对噪声敏感、太大跨越运动；需金字塔层数；假设亮度恒定 |
| 18 Farneback | 稠密但慢；`polyN`/`polySigma`/`winsize` 调优 |
| 20 CamShift | 需先有 H-S 直方图（L2 `26`/`29`）；窗口自适应缩放 |
| 22 Kalman | `transitionMatrix`/`measurementMatrix` 必须正确建模；先 predict 后 correct |
| 24 LSD | `detect` 返回 `Vec4f`（x1,y1,x2,y2）；对长直线效果好 |

## 资源与降级

- **视频/相机**：17/18/19/20/21/26 无摄像头时用静态图 + 人工位移合成帧对，或读视频文件。
- **双图匹配**：05/06/07/08/09 用同一图加仿射变换做匹配验证，避免缺数据。
- **SURF/SIFT**：专利算法在 L3 不强制；优先 ORB/AKAZE（开源），需对照 SURF 时查 ch03 文档。

## 与官方/文档的关系

- 阅读链：`principles.md` §8/11/13 → `ch03` / `ch04` → **本目录练习** → `mingw-build/samples/cpp` 官方源码
- 特征/轮廓/单应见 [ch03_features.md](../../docs/ch03_features.md)；光流/背景/跟踪/Kalman 见 [ch04_video.md](../../docs/ch04_video.md)
- 同一主题若官方有多份 demo，**只生成一个练习文件**；SURF/SIFT/LATCH 专利算法可编译降级或 README 标明跳过

## 说明

- 角点四题（01–04）全部保留：01/03 偏算法细节，02/04 偏实用与对比。
- 匹配三题（05/07/08）全部保留：分别对应描述子、Lowe、FLANN；06 AKAZE 与 05 ORB 对比抗模糊性。
- 轮廓六题（10–15）成体系：从提取到凸包、外接、矩、点测试、综合，构成「轮廓分析工具箱」。
- 无摄像头时 17/20/21/26 用静态图或合成帧对演示 API 语义。
- `25` 单应分解是 `09` 的进阶：09 求单应，25 把单应拆成旋转/平移/法向（注意有多解）。
