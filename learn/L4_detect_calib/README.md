# L4 检测、摄影与三维

**目标**：吃透目标检测（级联/HOG/QR/ArUco）、计算摄影（inpaint/GrabCut/无缝克隆/Decolor/NPR/HDR）与三维视觉（标定/对极/立体/拼接/PnP）。本章是 OpenCV 从「2D 像素」迈向「语义与几何」的桥梁。

**黄金主线**：01 → 05 → 07 → 11 → 12 → 21

**建议顺序**：01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 → 10 → 11 → 12 → 13 → 14 → 15 → 16 → 17 → 18 → 19 → 20 → 21

**验收**：能跑通 YuNet/级联人脸检测；能用 ArUco 标记检测；能用 `grabCut` 做前景抠图；能合成 HDR、相机标定、Charuco 板检测的最小管线；能说清「为什么标定要先求内参再求外参」。

## 核心概念速览

| 主题 | 关键词 | 心智模型 |
| --- | --- | --- |
| 级联检测 | Haar/AdaBoost/级联/积分图 | 弱分类器串联，前级快速拒绝背景 |
| HOG 行人 | 梯度方向直方图/SVM 滑窗 | 局部梯度统计描述外观，线性 SVM 判决 |
| QR/ArUco | 解码/标记姿态 | 二维码定位 + 透视校正还原矩阵 |
| 图像修复 | inpaint/Navier-Stokes/快速行进 | 用周围像素填补缺损区域 |
| 前景分割 | GrabCut/GMM/图割 | 交互式 GMM + 图割迭代求精 |
| 无缝克隆 | Poisson 融合/梯度域 | 让贴图与背景梯度连续，消除接缝 |
| HDR | Debevec/Mertens/Tonemap | 多曝光融合 + 色调映射回 LDR |
| 相机标定 | 张正友/棋盘/内参外参/畸变 | 已知世界点↔图像点求 PnP 得内参 |
| 对极几何 | 基础矩阵 F/本质矩阵 E/极线 | 双目对应约束的几何关系 |
| 立体匹配 | BM/SGBM/视差/深度 | 极线校正后按行搜索视差 |
| PnP | solvePnP/EPnP/RANSAC | 已知 3D-2D 对应求相机位姿 |

## 官方对照表

| 练习文件 | 标签 | 官方 sample | tutorialDoc | 必会 API |
| - | - | - | - | - |
| [01_cascade_face.cpp](01_cascade_face.cpp) | 主线 | `facedetect.cpp` | ch06 §级联 | `CascadeClassifier::detectMultiScale` |
| [02_hog_pedestrian.cpp](02_hog_pedestrian.cpp) | 主线 | `peopledetect.cpp` | ch06 §HOG | `HOGDescriptor::setSVMDetector` |
| [03_qrcode.cpp](03_qrcode.cpp) | 主线 | `qrcode.cpp` | ch06 §码制 | `QRCodeDetector::detectAndDecode` |
| [04_aruco_create.cpp](04_aruco_create.cpp) | 进阶 | `aruco_dict_utils.cpp`、`create_marker.cpp` | ch06 §ArUco | `aruco::Dictionary` / `drawMarker` |
| [05_aruco_detect.cpp](05_aruco_detect.cpp) | 主线 | `detect_markers.cpp` | ch06 §ArUco | `aruco::detectMarkers` / `estimatePoseSingleMarkers` |
| [06_inpaint.cpp](06_inpaint.cpp) | 主线 | `inpaint.cpp` | ch06 §修复 | `inpaint`（INPAINT_TELEA / NS） |
| [07_grabcut.cpp](07_grabcut.cpp) | 主线 | `grabcut.cpp` | ch06 §GrabCut | `grabCut` / `GC_INIT_WITH_RECT` |
| [08_seamless_clone.cpp](08_seamless_clone.cpp) | 进阶 | `cloning_demo.cpp` | ch06 §克隆 | `seamlessClone`（NORMAL/MIXED） |
| [09_decolor.cpp](09_decolor.cpp) | 选修 | `decolor.cpp` | ch06 §Decolor | `decolor`（保对比度转灰） |
| [10_npr.cpp](10_npr.cpp) | 选修 | `npr_demo.cpp` | ch06 §NPR | `edgePreservingFilter` / `detailEnhance` |
| [11_hdr.cpp](11_hdr.cpp) | 主线 | `hdr_imaging.cpp` | ch06 §HDR | `createMergeDebevec` / `createTonemapDrago` / `createMergeMertens` |
| [12_camera_calib.cpp](12_camera_calib.cpp) | 主线 | `camera_calibration.cpp`、`calibration.cpp` | ch07 §标定 | `findChessboardCorners` / `calibrateCamera` |
| [13_epipolar.cpp](13_epipolar.cpp) | 进阶 | `epipolar_lines.cpp` | ch07 §对极 | `findFundamentalMat` / `computeCorrespondEpilines` |
| [14_stereo_match.cpp](14_stereo_match.cpp) | 进阶 | `stereo_match.cpp` | ch07 §立体 | `StereoBM` / `StereoSGBM` |
| [15_stitching.cpp](15_stitching.cpp) | 进阶 | `stitching.cpp`、`stitching_detailed.cpp` | ch07 §拼接 | `Stitcher::create` / `stitch` |
| [16_dbt_face_detection.cpp](16_dbt_face_detection.cpp) | 选修 | `dbt_face_detection.cpp` | ch06 §DNN 桥梁 | `FaceDetectorYN` / `FaceRecognizerSF` |
| [17_facial_features.cpp](17_facial_features.cpp) | 进阶 | `facial_features.cpp` | ch06 §面部 | YuNet + 5 点关键点 |
| [18_smile_detect.cpp](18_smile_detect.cpp) | 选修 | `smiledetect.cpp` | ch06 §微笑 | 级联嵌套（人脸→嘴 ROI→微笑） |
| [19_essential_mat.cpp](19_essential_mat.cpp) | 进阶 | `essential_mat_reconstr.cpp` | ch07 §本质矩阵 | `findEssentialMat` / `recoverPose` |
| [20_pnp_pose.cpp](20_pnp_pose.cpp) | 进阶 | `real_time_pose_estimation/main_detection.cpp` | ch07 §PnP | `solvePnP` / `solvePnPRansac` |
| [21_charuco_detect.cpp](21_charuco_detect.cpp) | 进阶 | `detect_board_charuco.cpp` | ch06/ch07 §Charuco | `aruco::CharucoBoard` / `detectBoard` |

## 主题分组与先修关系

```
检测类（01-05,16-18）   ← 不依赖几何，只跑 2D
   │
摄影类（06-11）          ← 图像域像素操作
   │
三维类（12-15,19-21）    ← 需先理解相机模型与齐次坐标
```

- **检测类**：01 级联是经典入口；02 HOG 思想与 01 对比；03/04/05 ArUco 是标记法系，04 创造、05 检测成对。
- **摄影类**：06 修复、07 抠图、08 克隆是「像素填补/迁移」三件套；09/10 是风格化；11 HDR 综合多曝光。
- **三维类**：12 标定是所有几何重建的基础（先做）；13/19 对极与本质矩阵需 12 输出内参；14 立体匹配需先做 13 极线校正；15 拼接内部已封装匹配+单应；20 PnP 需 12 内参 + 已知 3D 模型；21 Charuco 是 12 + ArUco 的结合。

## 关键参数与易错点

| 练习 | 易错点 / 关键参数 |
| --- | --- |
| 01 级联 | `scaleFactor`（1.05~1.3，越小越慢越精）、`minNeighbors`（3~5 去误检）；预处理三件套：灰度→缩小→均衡化 |
| 02 HOG | `winStride`/`padding`/`hitThreshold`；默认 `getDefaultPeopleDetector` 只对 64×128 窗 |
| 05 ArUco | `estimatePoseSingleMarkers` 需先有内参；`markerLength` 单位与标定时一致 |
| 07 GrabCut | `GC_INIT_WITH_RECT` 用矩形初始化；迭代后用 `GC_INIT_WITH_MASK` + 掩膜精修 |
| 08 克隆 | `seamlessClone` 的 `Mat` 掩膜中心需与 `Point` 对齐；NORMAL 保色、MIXED 融合更自然 |
| 11 HDR | `MergeDebevec` 需 `times` 为 `CV_32FC1`；`ldr`/`fusion` 是 0~1 浮点，保存 PNG 需 `*255` |
| 12 标定 | 棋盘内角点数 `Size(w-1,h-1)`；`calibrateCamera` 同时求内参、畸变、旋平移 |
| 14 立体 | `StereoSGBM` 比 `StereoBM` 精但慢；`numDisparities` 必须是 16 的倍数 |
| 15 拼接 | `Stitcher::PANORAMA` 比 `SCANS` 快但需旋转相机假设；失败时调 `confidence` |
| 20 PnP | `SOLVEPNP_IPPE` 适合平面目标；`SOLVEPNP_ITERATIVE` 通用；RANSAC 抗外点 |

## 资源与降级

- **人脸**：优先 `getModelPath("face_detection_yunet_2023mar.onnx")` 跑 YuNet（`FaceDetectorYN`）；无模型时降级 `haarcascade_frontalface_alt.xml`（放 `models/`）。
- **标定**：无真实棋盘序列时 12/21 用合成棋盘图演示 API 语义（`drawChessboardCorners` 自造）。
- **立体/拼接**：14/15 无真实双目或重叠图时用合成位移对演示。
- **HDR**：11 无曝光序列时用同一图加不同 gamma 模拟多曝光。

## 与官方/文档的关系

- 阅读链：`principles.md` §14-15 → `ch06` / `ch07` → **本目录练习** → `mingw-build/samples/cpp` 官方源码
- 检测与摄影主题见 [ch06_objdetect_photo.md](../../docs/ch06_objdetect_photo.md)；标定与拼接见 [ch07_calib3d_stitching.md](../../docs/ch07_calib3d_stitching.md)
- 同一主题若官方有多份 demo，**只生成一个练习文件**；YuNet/DNN 桥梁见 `dbt_face_detection.cpp`
- SURF/SIFT 等专利算法在 L3 处理；本章聚焦检测/摄影/几何

## 说明

- `01` 与 `16` 对比：级联（Haar）vs DNN（YuNet），体会精度/速度/姿态鲁棒性差异。
- `04` 与 `05` 成对：先创造标记再检测，理解 ArUco 字典与位姿估计。
- `12` 标定是 `13/14/19/20/21` 的前置——没有内参，后续几何重建无从谈起。
- `15` 拼接是 L3 特征匹配 + 单应 + 融合的综合应用，建议 L3 学完再做。
