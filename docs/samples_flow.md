# OpenCV C++ 示例处理流程详解

> 本文聚焦 `mingw-build/samples/cpp` **根目录 97** 个*官方示例*的**流程切片**（**输入 → 预处理 → 核心 API → 输出**）。
> **原理与调参**请读 [principles.md](./principles.md)（主入口）；**逐文件深读**见 [ch01](./ch01_core.md)–[ch08](./ch08_gui_gapi_gpu.md)。
> `tutorial_code` 136 个文件见 [F.T 主题索引](#ft-tutorial_code-主题索引136-文件) 与 [附录 B 示例清单](./README.md#附录-b-示例清单demo_map)。

## 目录索引

- [F.0 使用说明](#f0-使用说明)
- [F.1 核心与 I/O（ch01，12 文件）](#f1-核心与-ioch0112-文件)
- [F.2 图像增强与滤波（ch02，15 文件）](#f2-图像增强与滤波ch0215-文件)
- [F.3 特征与形状（ch03，14 文件）](#f3-特征与形状ch0314-文件)
- [F.4 视频与运动（ch04，7 文件）](#f4-视频与运动ch047-文件)
- [F.5 机器学习（ch05，11 文件）](#f5-机器学习ch0511-文件)
- [F.6 目标检测与计算摄影（ch06，14 文件）](#f6-目标检测与计算摄影ch0614-文件)
- [F.7 calib3d 与拼接（ch07，9 文件）](#f7-calib3d-与拼接ch079-文件)
- [F.8 GUI / G-API / GPU（ch08，15 文件）](#f8-gui--g-api--gpuch0815-文件)
- [F.T tutorial_code 主题索引（136 文件）](#ft-tutorial_code-主题索引136-文件)

---

## F.0 使用说明

每个示例条目给出四要素：

- **功能**：该示例演示什么算法/能力；
- **流程**：数据从输入到输出的关键步骤链；
- **关键 API**：示例中最值得记住的 OpenCV 函数/类；
- **关联**：对应分章深读入口与原理节。

> 流程链中的 `→` 仅表"先后"，不一定是单步函数调用；同一箭头段可能含多步。
> 根目录示例全部为"完整流程"型，可直接对照源码编译运行；原理深读请进对应分章。

***

## F.1 核心与 I/O（ch01，12 文件）

> 对应 [ch01_core.md](./ch01_core.md)；原理见 [principles.md §1 Mat](./principles.md#1-图像的像素结构与-mat)、[§10 频域/ECC](./principles.md#10-频域变换与-ecc-配准)。

### application_trace.cpp
**功能**：演示 `cv::trace` 应用调用轨迹，定位函数入口与耗时。
**流程**：启动 trace → 执行被测代码段 → `cv::Mat trace` 收集事件 → 输出耗时树。
**关键 API**：`cv::trace`、`cv::Mat`、`cv::getTickCount`。
**关联**：[ch01_core.md](./ch01_core.md) §性能与诊断。

### cout_mat.cpp
**功能**：打印 Mat 的尺寸/类型/通道/部分像素，演示矩阵格式化输出。
**流程**：构造 Mat → `cv::format(mat, fmt)` → `std::cout << mat` → 控制台文本。
**关键 API**：`cv::Mat::create`、`cv::format`、`operator<<`。
**关联**：[principles.md §1](./principles.md#1-图像的像素结构与-mat) ｜ [ch01_core.md](./ch01_core.md)。

### create_mask.cpp
**功能**：用阈值/比较生成二值掩膜，演示掩膜与 ROI 的关系。
**流程**：读图 → 转灰度 → `compare`/`threshold` → 输出二值掩膜 → 与原图按位与。
**关键 API**：`cv::threshold`、`cv::compare`、`cv::bitwise_and`、`cv::Mat::operator()`。
**关联**：[ch01_core.md](./ch01_core.md) §掩膜与 ROI。

### dft.cpp
**功能**：离散傅里叶变换完整流程，展示频谱中心化与逆变换重建。
**流程**：读图转灰度 → 扩展到最优 DFT 尺寸 → `dft` → `magnitude` → `log` 归一化中心化 → 可视化频谱 → `idft` 还原。
**关键 API**：`cv::dft`、`cv::idft`、`cv::magnitude`、`cv::getOptimalDFTSize`、`cv::copyMakeBorder`。
**关联**：[principles.md §10](./principles.md#10-频域变换与-ecc-配准) ｜ [ch01_core.md](./ch01_core.md)。

### image_alignment.cpp
**功能**：基于 ECC 的图像配准，估计模板与输入间的几何变换。
**流程**：读入模板/输入 → 初始化 warp 矩阵 → `findTransformECC` 迭代 → `warpAffine` 对齐 → 输出误差与配准图。
**关键 API**：`cv::findTransformECC`、`cv::warpAffine`、`MOTION_*`。
**关联**：[principles.md §10](./principles.md#10-频域变换与-ecc-配准) ｜ [ch01_core.md](./ch01_core.md)。

### imagelist_creator.cpp
**功能**：把多张图像路径写成 XML/YAML 图像列表文件，供标定/拼接读取。
**流程**：命令行收集路径 → 构造 `FileStorage` → 逐张 `write` → 关闭输出。
**关键 API**：`cv::FileStorage`、`cv::Mat`、`operator<<`。
**关联**：[ch01_core.md](./ch01_core.md) §文件 I/O ｜ [ch07](./ch07_calib3d_stitching.md) 标定输入。

### imagelist_reader.cpp
**功能**：读取 image list 文件并按 Z=0 平面生成标定板物点，演示批量图加载。
**流程**：`FileStorage` 读 list → 逐张 `imdecode`/`imread` → 拼接成单 Mat 或列表 → 输出。
**关键 API**：`cv::FileStorage`、`cv::imdecode`、`cv::hconcat`/`vconcat`。
**关联**：[ch01_core.md](./ch01_core.md) ｜ [ch07](./ch07_calib3d_stitching.md) `stereo_calib`。

### imgcodecs_jpeg.cpp
**功能**：JPEG 压缩质量与采样的编解码演示，量化参数对失真的影响。
**流程**：读图 → `imencode`(.jpg, params) → `imdecode` 回读 → 计算 PSNR/BPP → 输出对比。
**关键 API**：`cv::imencode`、`cv::imdecode`、`IMWRITE_JPEG_QUALITY`、`IMWRITE_JPEG_SAMPLING_FACTOR`。
**关联**：[ch01_core.md](./ch01_core.md) §编解码。

### intersectExample.cpp
**功能**：两矩形/ROI 的相交判断，演示 `Rect` 运算与有效区域裁剪。
**流程**：构造两个 `Rect` → `rect & rect` → 检查面积 → 输出交集。
**关键 API**：`cv::Rect`、`operator&`、`cv::Rect::area`。
**关联**：[ch01_core.md](./ch01_core.md) §几何类型。

### opencv_version.cpp
**功能**：打印 OpenCV 版本/编译配置/支持模块，诊断环境。
**流程**：`getVersionString` → `getBuildInformation` → 输出。
**关键 API**：`cv::getVersionString`、`cv::getBuildInformation`。
**关联**：[ch01_core.md](./ch01_core.md) §环境诊断。

### simd_basic.cpp
**功能**：通用 SIMD (`universal_intrinsics`) 入门，演示向量类型与 lane 运算。
**流程**：定义 `v_float32` → `v_load`/`v_add` → `v_store` → 标量对照。
**关键 API**：`cv::v_float32`、`cv::v_load`、`cv::v_add`、`cv::v_store`、`cv::checkHardwareSupport`。
**关联**：[ch01_core.md](./ch01_core.md) §并行与向量化 ｜ [ch08](./ch08_gui_gapi_gpu.md)。

### travelsalesman.cpp
**功能**：用模拟退火/EMD 解旅行商问题，演示元启发式优化与 Mat 随机访问。
**流程**：构造距离矩阵 → 初始路线 → 迭代扰动+接受准则 → 输出最优路径。
**关键 API**：`cv::Mat`、`cv::randu`、`cv::theRNG`。
**关联**：[ch01_core.md](./ch01_core.md) §优化与随机数 ｜ [ch05](./ch05_ml.md)。

---

## F.2 图像增强与滤波（ch02，15 文件）

> 对应 [ch02_imgproc.md](./ch02_imgproc.md)；原理见 [principles.md §5 滤波](./principles.md#5-卷积与滤波邻域运算的基石)、[§6 形态学](./principles.md#6-形态学操作腐蚀膨胀与开闭运算)、[§7 边缘/几何](./principles.md#7-边缘hough-与几何变换)、[§11 轮廓](./principles.md#11-轮廓与形状分析)、[§12 分割](./principles.md#12-分割阈值距离变换与分水岭)。

### connected_components.cpp
**功能**：二值图连通域标记，输出各域标签与统计。
**流程**：读图转灰度 → `threshold` → `connectedComponentsWithStats` → 输出标签图与统计。
**关键 API**：`cv::connectedComponentsWithStats`、`cv::connectedComponents`、`cv::threshold`。
**关联**：[principles.md §11](./principles.md#11-轮廓与形状分析) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### demhist.cpp
**功能**：交互式直方图均衡化演示，滑条改变参数对照原图。
**流程**：读图转灰度 → `calcHist` → 滑条控制 `equalizeHist`/`normalize` → 显示对比。
**关键 API**：`cv::calcHist`、`cv::equalizeHist`、`cv::normalize`、`cv::createTrackbar`。
**关联**：[principles.md §4](./principles.md#4-直方图与模板匹配) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### distrans.cpp
**功能**：距离变换与可视化，输出每点到最近零像素的距离。
**流程**：读图转灰度 → `threshold` 反相 → `distanceTransform` → `normalize` 着色。
**关键 API**：`cv::distanceTransform`、`cv::DIST_L2`/`DIST_C`/`DIST_L1`、`cv::normalize`。
**关联**：[principles.md §12](./principles.md#12-分割阈值距离变换与分水岭) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### edge.cpp
**功能**：Canny/Scharr 边缘检测交互演示，滑条调阈值。
**流程**：读图转灰度 → `Canny`/`Scharr` → 显示边缘图 → 滑条改阈值。
**关键 API**：`cv::Canny`、`cv::Scharr`、`cv::createTrackbar`。
**关联**：[principles.md §7](./principles.md#7-边缘hough-与几何变换) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### ela.cpp
**功能**：错误级分析（ELA），通过重 JPEG 压缩定位被篡改区域。
**流程**：读图 → `imencode` 质量Q → `imdecode` → 与原图 `absdiff` → 阈值化可疑区。
**关键 API**：`cv::imencode`、`cv::imdecode`、`cv::absdiff`、`cv::threshold`。
**关联**：[ch02_imgproc.md](./ch02_imgproc.md) §取证分析。

### falsecolor.cpp
**功能**：用 `applyColorMap` 给灰度图上伪彩，突出可视化对比。
**流程**：读图转灰度 → `normalize` → `applyColorMap` → 显示多套色图。
**关键 API**：`cv::applyColorMap`、`cv::COLORMAP_*`、`cv::normalize`。
**关联**：[ch02_imgproc.md](./ch02_imgproc.md) §可视化。

### ffilldemo.cpp
**功能**：泛洪填充交互演示，演示连通性与颜色范围控制。
**流程**：读图 → 鼠标选种子 → `floodFill` → 显示填充结果 → 滑条改 `loDiff/upDiff`。
**关键 API**：`cv::floodFill`、`cv::setMouseCallback`、`cv::createTrackbar`。
**关联**：[principles.md §12](./principles.md#12-分割阈值距离变换与分水岭) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### laplace.cpp
**功能**：Laplacian 边缘与高频成分，演示二阶导数响应。
**流程**：读图转灰度 → `GaussianBlur` 预处理 → `Laplacian` → `convertScaleAbs` → 显示。
**关键 API**：`cv::Laplacian`、`cv::GaussianBlur`、`cv::convertScaleAbs`。
**关联**：[principles.md §5/§7](./principles.md#5-卷积与滤波邻域运算的基石) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### mask_tmpl.cpp
**功能**：带掩膜的模板匹配，演示 `matchTemplate` 的 mask 用法。
**流程**：读图与模板 → 构造 mask → `matchTemplate` → `minMaxLoc` → 框选匹配点。
**关键 API**：`cv::matchTemplate`、`cv::TM_*`、`cv::minMaxLoc`。
**关联**：[principles.md §4](./principles.md#4-直方图与模板匹配) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### morphology2.cpp
**功能**：形态学开闭/梯度/顶帽黑帽交互演示，滑条调核大小。
**流程**：读图转灰度 → `getStructuringElement` → `morphologyEx` → 显示各操作结果。
**关键 API**：`cv::getStructuringElement`、`cv::morphologyEx`、`MORPH_OPEN/CLOSE/GRADIENT/TOPHAT/BLACKHAT`。
**关联**：[principles.md §6](./principles.md#6-形态学操作腐蚀膨胀与开闭运算) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### phase_corr.cpp
**功能**：相位相关估计两图平移位移，演示频域配准。
**流程**：读两图转灰度 → `phaseCorrelate` → 输出 (dx, dy) → 按位移对齐。
**关键 API**：`cv::phaseCorrelate`、`cv::createHanningWindow`、`cv::dft`。
**关联**：[principles.md §10](./principles.md#10-频域变换与-ecc-配准) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### polar_transforms.cpp
**功能**：线性/对数极坐标变换，演示笛卡尔与极坐标互转。
**流程**：读图 → `warpPolar`(线性) → `warpPolar`(对数, `WARP_INVERSE_MAP`) → 显示对照。
**关键 API**：`cv::warpPolar`、`cv::WARP_POLAR_LINEAR`/`LOG`/`INVERSE_MAP`。
**关联**：[principles.md §7](./principles.md#7-边缘hough-与几何变换) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### text_skewness_correction.cpp
**功能**：文本图像倾斜校正，用 minAreaRect+旋转矫正。
**流程**：读图转灰度 → `threshold` 反相 → `findContours` → `minAreaRect` 求倾角 → `getRotationMatrix2D`+`warpAffine` 矫正。
**关键 API**：`cv::minAreaRect`、`cv::getRotationMatrix2D`、`cv::warpAffine`、`cv::findContours`。
**关联**：[principles.md §7/§11](./principles.md#7-边缘hough-与几何变换) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### warpPerspective_demo.cpp
**功能**：透视变换交互演示，鼠标选四点映射到目标矩形。
**流程**：读图 → 鼠标采 4 点 → `getPerspectiveTransform` → `warpPerspective` → 显示。
**关键 API**：`cv::getPerspectiveTransform`、`cv::warpPerspective`、`cv::setMouseCallback`。
**关联**：[principles.md §7](./principles.md#7-边缘hough-与几何变换) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

### watershed.cpp
**功能**：分水岭分割交互演示，标记驱动分割。
**流程**：读图 → 鼠标画标记 → `watershed` → 输出分割边界 → 覆盖显示。
**关键 API**：`cv::watershed`、`cv::setMouseCallback`、`cv::addWeighted`。
**关联**：[principles.md §12](./principles.md#12-分割阈值距离变换与分水岭) ｜ [ch02_imgproc.md](./ch02_imgproc.md)。

---

## F.3 特征与形状（ch03，14 文件）

> 对应 [ch03_features.md](./ch03_features.md)；原理见 [principles.md §8 特征](./principles.md#8-特征检测与描述从角点到描述子)、[§11 轮廓](./principles.md#11-轮廓与形状分析)。

### asift.cpp
**功能**：仿射 SIFT（ASIFT），多视角模拟提升 SIFT 匹配鲁棒性。
**流程**：读两图 → 对每图做一系列仿射模拟 → 各组 SIFT+FLANN 匹配 → 合并匹配 → 单应估计。
**关键 API**：`cv::SIFT`、`cv::FlannBasedMatcher`、`cv::findHomography`、`cv::warpAffine`。
**关联**：[ch03_features.md](./ch03_features.md) §SIFT/ASIFT ｜ [principles.md §8](./principles.md#8-特征检测与描述从角点到描述子)。

### contours2.cpp
**功能**：轮廓层级与多边形逼近演示，输出内外轮廓结构。
**流程**：读图转灰度 → `threshold` → `findContours`(RETR_TREE) → `approxPolyDP` → `drawContours`。
**关键 API**：`cv::findContours`、`cv::approxPolyDP`、`cv::drawContours`、`RETR_TREE/EXTERNAL`。
**关联**：[principles.md §11](./principles.md#11-轮廓与形状分析) ｜ [ch03_features.md](./ch03_features.md)。

### convexhull.cpp
**功能**：点集凸包计算与绘制。
**流程**：随机/轮廓点 → `convexHull` → `polylines` 绘制。
**关键 API**：`cv::convexHull`、`cv::polylines`、`cv::fillConvexPoly`。
**关联**：[principles.md §11](./principles.md#11-轮廓与形状分析) ｜ [ch03_features.md](./ch03_features.md)。

### delaunay2.cpp
**功能**：Delaunay 三角剖分与 Voronoi 图绘制。
**流程**：构造 `Subdiv2D` → 插入点 → `getTriangleList`/`getVoronoiFacetList` → 绘制。
**关键 API**：`cv::Subdiv2D`、`cv::Subdiv2D::insert`、`getTriangleList`、`getVoronoiFacetList`。
**关联**：[principles.md §11](./principles.md#11-轮廓与形状分析) ｜ [ch03_features.md](./ch03_features.md)。

### detect_blob.cpp
**功能**：SimpleBlobDetector 斑点检测，按面积/圆度/凸度过滤。
**流程**：读图 → `SimpleBlobDetector::Params` 设参 → `create`+`detect` → `drawKeypoints`。
**关键 API**：`cv::SimpleBlobDetector`、`cv::FeatureDetector::detect`、`cv::drawKeypoints`。
**关联**：[principles.md §8](./principles.md#8-特征检测与描述从角点到描述子) ｜ [ch03_features.md](./ch03_features.md)。

### detect_mser.cpp
**功能**：MSER 最大稳定极值区域检测，用于文字/稳定区域提取。
**流程**：读图转灰度 → `MSER::create` → `detectRegions` → 绘制区域多边形。
**关键 API**：`cv::MSER::create`、`cv::MSER::detectRegions`、`cv::polylines`。
**关联**：[principles.md §8/§12](./principles.md#8-特征检测与描述从角点到描述子) ｜ [ch03_features.md](./ch03_features.md)。

### fitellipse.cpp
**功能**：最小二乘椭圆拟合与几何参数输出。
**流程**：读图转灰度 → `threshold`/`findContours` → `fitEllipse` → `ellipse` 绘制。
**关键 API**：`cv::fitEllipse`、`cv::RotatedRect`、`cv::ellipse`。
**关联**：[principles.md §11](./principles.md#11-轮廓与形状分析) ｜ [ch03_features.md](./ch03_features.md)。

### flann_search_dataset.cpp
**功能**：FLANN 近邻搜索在数据集上的精度/速度演示。
**流程**：构造随机/读数据集 → `flann::Index`(KDTree/LSH) → `knnSearch` → 统计召回率。
**关键 API**：`cv::flann::Index`、`cv::flann::KDTreeIndexParams`/`LshIndexParams`、`knnSearch`。
**关联**：[ch03_features.md](./ch03_features.md) §FLANN ｜ [ch05_ml.md](./ch05_ml.md)。

### intelligent_scissors.cpp
**功能**：智能剪刀交互式分割，图论最短路径提取边界。
**流程**：读图算梯度代价图 → 鼠标选种子 → Dijkstra 最短路径 → 绘制轮廓 → `grabCut`/`floodFill` 闭合。
**关键 API**：`cv::Canny`、`cv::addWeighted`（代价图）、自定义 Dijkstra。
**关联**：[principles.md §11/§12](./principles.md#11-轮廓与形状分析) ｜ [ch03_features.md](./ch03_features.md)。

### lsd_lines.cpp
**功能**：LSD 线段检测器，提取像素级直线段。
**流程**：读图转灰度 → `LineSegmentDetector::create` → `detect` → `drawSegments`。
**关键 API**：`cv::LineSegmentDetector::create`、`cv::LineSegmentDetector::detect`、`drawSegments`。
**关联**：[principles.md §7](./principles.md#7-边缘hough-与几何变换) ｜ [ch03_features.md](./ch03_features.md)。

### matchmethod_orb_akaze_brisk.cpp
**功能**：ORB/AKAZE/BRISK 三种二值描述子匹配对比。
**流程**：读两图 → 各 `detectAndCompute` → `BFMatcher`(HAMMING) + `knnMatch` → Lowe 比率筛选 → 对照匹配数。
**关键 API**：`cv::ORB`/`AKAZE`/`BRISK`、`cv::BFMatcher`、`cv::NORM_HAMMING`、`knnMatch`。
**关联**：[principles.md §8](./principles.md#8-特征检测与描述从角点到描述子) ｜ [ch03_features.md](./ch03_features.md)。

### minarea.cpp
**功能**：点集最小外接矩形/圆/凸多边形计算。
**流程**：随机/轮廓点 → `minAreaRect`/`minEnclosingCircle`/`minEnclosingTriangle` → 绘制对照。
**关键 API**：`cv::minAreaRect`、`cv::minEnclosingCircle`、`cv::minEnclosingTriangle`。
**关联**：[principles.md §11](./principles.md#11-轮廓与形状分析) ｜ [ch03_features.md](./ch03_features.md)。

### segment_objects.cpp
**功能**：基于背景差分的视频对象分割，逐帧提取运动前景。
**流程**：开视频 → 背景建模 → `absdiff`+`threshold` → 形态学清理 → `findContours` → 框选目标。
**关键 API**：`cv::absdiff`、`cv::threshold`、`cv::findContours`、`cv::BackgroundSubtractor`。
**关联**：[ch03_features.md](./ch03_features.md) ｜ [ch04_video.md](./ch04_video.md) §背景差分。

### squares.cpp
**功能**：多边形矩形度判定，从轮廓里筛选"似矩形"。
**流程**：读图多通道 `Canny`/`threshold` → `findContours` → `approxPolyDP` → 4 顶点+角度判定 → 输出矩形集。
**关键 API**：`cv::findContours`、`cv::approxPolyDP`、`cv::cos`/角度计算。
**关联**：[principles.md §11](./principles.md#11-轮廓与形状分析) ｜ [ch03_features.md](./ch03_features.md)。

---

## F.4 视频与运动（ch04，7 文件）

> 对应 [ch04_video.md](./ch04_video.md)；原理见 [principles.md §13 运动分析](./principles.md#13-运动分析与机器学习)。

### bgfg_segm.cpp
**功能**：背景/前景分割，对比 MOG2/KNN 等多种背景模型。
**流程**：开视频 → `createBackgroundSubtractorMOG2`/`KNN` → 逐帧 `apply` → `threshold`+形态学 → 显示前景掩膜。
**关键 API**：`cv::BackgroundSubtractorMOG2`、`cv::BackgroundSubtractorKNN`、`apply`、`getStructuringElement`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch04_video.md](./ch04_video.md)。

### camshiftdemo.cpp
**功能**：CamShift 均值漂移目标跟踪，色相直方图驱动。
**流程**：开视频 → 选 ROI → `cvtColor` HSV → `calcHist` → `CamShift` 逐帧 → 框选新位置。
**关键 API**：`cv::CamShift`、`cv::calcHist`、`cv::calcBackProject`、`cv::cvtColor`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch04_video.md](./ch04_video.md)。

### dis_opticalflow.cpp
**功能**：DIS 稠密光流，演示快速光流估计与可视化。
**流程**：开视频取两帧 → `DISOpticalFlow::create` → `calc` → `flow` 到极坐标 → 可视化 HSV。
**关键 API**：`cv::DISOpticalFlow::create`、`DISOpticalFlow::calc`、`cv::cartToPolar`、`cv::cvtColor`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch04_video.md](./ch04_video.md)。

### fback.cpp
**功能**：Farneback 稠密光流演示与 HSV 可视化。
**流程**：开视频取两帧 → `calcOpticalFlowFarneback` → `cartToPolar` 取幅相 → HSV 显示。
**关键 API**：`cv::calcOpticalFlowFarneback`、`cv::cartToPolar`、`cv::cvtColor`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch04_video.md](./ch04_video.md)。

### kalman.cpp
**功能**：Kalman 滤波对二维点轨迹预测与平滑。
**流程**：定义状态/测量方程 → 初始化 `KalmanFilter` → 逐帧 `predict`+`correct` → 绘制真值/观测/估计。
**关键 API**：`cv::KalmanFilter`、`predict`/`correct`、`cv::randn`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch04_video.md](./ch04_video.md)。

### lkdemo.cpp
**功能**：Lucas-Kanade 稀疏光流，鼠标选点逐帧跟踪。
**流程**：开视频 → 鼠标选特征点 → `goodFeaturesToTrack` → 逐帧 `calcOpticalFlowPyrLK` → 绘制轨迹。
**关键 API**：`cv::calcOpticalFlowPyrLK`、`cv::goodFeaturesToTrack`、`cv::setMouseCallback`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch04_video.md](./ch04_video.md)。

### points_classifier.cpp
**功能**：交互式二维点分类，对比多种 ML 分类器决策边界。
**流程**：鼠标添加带标签点 → 构造训练集 → 训练 SVM/RTrees/Boost 等 → `predict` 网格 → 绘制决策面。
**关键 API**：`cv::ml::SVM`/`RTrees`/`Boost`/`NormalBayesClassifier`、`cv::ml::TrainData`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

---

## F.5 机器学习（ch05，11 文件）

> 对应 [ch05_ml.md](./ch05_ml.md)；原理见 [principles.md §13 ML](./principles.md#13-运动分析与机器学习)。

### digits_lenet.cpp
**功能**：DNN 版 LeNet 训练/推理手写数字识别。
**流程**：读 digits 数据 → 构造 LeNet 模型 → `dnn::Net` 训练/前向 → 评估准确率。
**关键 API**：`cv::dnn::readNet`/`LayerParams`、`cv::dnn::Net::forward`、`cv::ml::TrainData`。
**关联**：[principles.md §13/§14](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md) §DNN。

### digits_svm.cpp
**功能**：SVM 手写数字分类，HOG+RBF 核。
**流程**：读 digits → `HOGDescriptor` 提特征 → `SVM::train` → `predict` → 输出混淆矩阵。
**关键 API**：`cv::ml::SVM`、`cv::HOGDescriptor`、`cv::ml::TrainData`、`cv::ml::ROW_SAMPLE`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### em.cpp
**功能**：期望最大化（EM）高斯混合聚类，估计分量参数。
**流程**：生成/读数据 → `EM::create` 设聚类数 → `trainEM` → `predict` 分量 → 绘制分布。
**关键 API**：`cv::ml::EM::create`、`trainEM`、`predict`、`cv::ml::TrainData`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### kmeans.cpp
**功能**：K-Means 聚类交互演示，多簇点集聚类与可视化。
**流程**：随机生成/读点集 → `kmeans` → 标签 → 按簇着色 → 滑条改 K。
**关键 API**：`cv::kmeans`、`cv::TermCriteria`、`KMEANS_PP_CENTERS`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### letter_recog.cpp
**功能**：UCI letter 数据集多分类，对比多种分类器。
**流程**：读 CSV → `TrainData::read` → 训练 Boost/RTrees/SVM/MLP → `predict` → 输出准确率。
**关键 API**：`cv::ml::TrainData::read`、`cv::ml::Boost`/`RTrees`/`SVM`/`ANN_MLP`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### logistic_regression.cpp
**功能**：逻辑回归二分类演示。
**流程**：构造数据 → `LogisticRegression::create` → `train` → `predict` → 输出准确率/损失。
**关键 API**：`cv::ml::LogisticRegression::create`、`train`、`predict`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### neural_network.cpp
**功能**：多层感知机（MLP）训练与决策边界可视化。
**流程**：构造数据 → `ANN_MLP::create` 设层 → `train` → `predict` 网格 → 绘制决策面。
**关键 API**：`cv::ml::ANN_MLP::create`、`setLayerSizes`、`train`、`predict`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### pca.cpp
**功能**：PCA 主成分分析降维与重构可视化。
**流程**：读图/数据展开成行向量 → `PCA` 计算 → 投影到前 K 维 → `backProject` 重构 → 对照原图。
**关键 API**：`cv::PCA`、`PCA::project`、`PCA::backProject`、`cv::Mat::reshape`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### train_HOG.cpp
**功能**：HOG 描述子 + SVM 行人检测器训练流程。
**流程**：正/负样本 → `HOGDescriptor` 提特征 → `SVM::train` → `svm->getSupportVectors` → `HOG::setSVMDetector` → 测试。
**关键 API**：`cv::HOGDescriptor`、`cv::ml::SVM`、`setSVMDetector`、`detectMultiScale`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch05_ml.md](./ch05_ml.md) ｜ [ch06](./ch06_objdetect_photo.md)。

### train_svmsgd.cpp
**功能**：SVMSGD（随机梯度下降 SVM）训练演示。
**流程**：构造线性可分数据 → `SVMSGD::create` → `train` → `predict` → 绘制分离超平面。
**关键 API**：`cv::ml::SVMSGD::create`、`train`、`predict`、`getWeights`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

### tree_engine.cpp
**功能**：决策树/随机森林训练与特征重要性演示。
**流程**：构造数据 → `DTrees`/`RTrees::create` → `train` → `predict` → 输出准确率与变量重要性。
**关键 API**：`cv::ml::RTrees`/`DTrees`、`getVariableImportance`、`train`、`predict`。
**关联**：[principles.md §13](./principles.md#13-运动分析与机器学习) ｜ [ch05_ml.md](./ch05_ml.md)。

---

## F.6 目标检测与计算摄影（ch06，14 文件）

> 对应 [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)；原理见 [principles.md §14 检测/计算摄影](./principles.md#14-目标检测与计算摄影)、[§12 分割](./principles.md#12-分割阈值距离变换与分水岭)。

### aruco_dict_utils.cpp
**功能**：ArUco 字典生成/校验工具，演示标记字典构造。
**流程**：`Dictionary::create`/`getPredefinedDictionary` → `drawMarker` → 校验汉明距离。
**关键 API**：`cv::aruco::Dictionary`、`cv::aruco::drawMarker`、`cv::aruco::DetectorParameters`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### barcode.cpp
**功能**：条形码检测与解码演示。
**流程**：读图 → `barcode::BarCodeDetector` → `detect`+`decode` → `polylines` 框选 → 输出内容。
**关键 API**：`cv::barcode::BarCodeDetector`、`detect`、`decode`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### cloning_demo.cpp
**功能**：无缝克隆（Poisson 融合）命令行版，把前景自然嵌入背景。
**流程**：读背景/前景/掩膜 → `seamlessClone` → 输出融合图。
**关键 API**：`cv::seamlessClone`、`cv::NORMAL_CLONE`/`MIXED_CLONE`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### cloning_gui.cpp
**功能**：无缝克隆交互版，鼠标拖动前景与掩膜。
**流程**：读图 → 鼠标定位 → `seamlessClone` → 实时显示 → 滑条改融合模式。
**关键 API**：`cv::seamlessClone`、`cv::setMouseCallback`、`cv::createTrackbar`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### dbt_face_detection.cpp
**功能**：基于跟踪的检测（Detected By Tracking），结合检测器与光流稳定人脸。
**流程**：开视频 → `CascadeClassifier` 隔帧检测 → `calcOpticalFlowPyrLK` 框间跟踪 → 输出轨迹。
**关键 API**：`cv::CascadeClassifier`、`cv::calcOpticalFlowPyrLK`、`cv::goodFeaturesToTrack`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### facedetect.cpp
**功能**：Haar 级联人脸检测经典示例。
**流程**：加载 `haarcascade_frontalface_alt.xml` → 读图/视频 `detectMultiScale` → `rectangle` 框选 → 显示。
**关键 API**：`cv::CascadeClassifier`、`detectMultiScale`、`cv::rectangle`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### facial_features.cpp
**功能**：人脸 + 五官（眼/嘴）多级联套检测。
**流程**：`CascadeClassifier` 人脸 → ROI 内套用眼/嘴级联 → `detectMultiScale` → 绘制。
**关键 API**：`cv::CascadeClassifier`、`detectMultiScale`、`cv::Rect` 嵌套 ROI。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### gauge.cpp
**功能**：仪表盘读数识别，指针角度与刻度映射。
**流程**：读表盘 → Hough 圆/线 → 指针角度 → 映射到量程 → 输出数值。
**关键 API**：`cv::HoughCircles`、`cv::HoughLinesP`、`cv::LineSegmentDetector`。
**关联**：[principles.md §7/§14](./principles.md#7-边缘hough-与几何变换) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### grabcut.cpp
**功能**：GrabCut 交互式前景分割。
**流程**：读图 → 鼠标框 ROI → `grabCut` 迭代 → `compare` 掩膜 → 提取前景。
**关键 API**：`cv::grabCut`、`cv::GC_INIT_WITH_RECT`/`GC_INIT_WITH_MASK`、`cv::setMouseCallback`。
**关联**：[principles.md §12](./principles.md#12-分割阈值距离变换与分水岭) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### inpaint.cpp
**功能**：图像修复（Navier-Stokes/Telea），去除划痕/水印。
**流程**：读图与掩膜 → `inpaint` → 输出修复图。
**关键 API**：`cv::inpaint`、`cv::INPAINT_TELEA`/`INPAINT_NS`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### npr_demo.cpp
**功能**：非真实感渲染（NPR），边缘保持平滑/铅笔/水彩风格化。
**流程**：读图 → `edgePreservingFilter`/`detailEnhance`/`pencilSketch`/`stylization` → 输出多风格。
**关键 API**：`cv::edgePreservingFilter`、`cv::detailEnhance`、`cv::pencilSketch`、`cv::stylization`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### peopledetect.cpp
**功能**：HOG 行人检测器演示。
**流程**：读图 → `HOGDescriptor` → `setSVMDetector`(默认行人) → `detectMultiScale` → 框选。
**关键 API**：`cv::HOGDescriptor`、`setSVMDetector`、`detectMultiScale`、`cv::HOGDescriptor::getDefaultPeopleDetector`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md) ｜ [ch05](./ch05_ml.md)。

### qrcode.cpp
**功能**：二维码检测与解码演示。
**流程**：读图 → `QRCodeDetector` → `detect`+`decode` → 框选与输出内容。
**关键 API**：`cv::QRCodeDetector`、`detect`、`decode`、`detectAndDecode`。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

### smiledetect.cpp
**功能**：人脸 + 笑容级联分层检测。
**流程**：人脸 `CascadeClassifier` → ROI 套笑容级联 `detectMultiScale` → 标注是否笑。
**关键 API**：`cv::CascadeClassifier`、`detectMultiScale`、嵌套 ROI。
**关联**：[principles.md §14](./principles.md#14-目标检测与计算摄影) ｜ [ch06_objdetect_photo.md](./ch06_objdetect_photo.md)。

---

## F.7 calib3d 与拼接（ch07，9 文件）

> 对应 [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)；原理见 [principles.md §15 相机/多视/拼接](./principles.md#15-相机模型多视几何与拼接)。

### 3calibration.cpp
**功能**：三相机系统联合标定，估计外参相对位姿。
**流程**：多机棋盘采集 → `findChessboardCorners`+`cornerSubPix` → `stereoCalibrate`/`calibrateCamera` → 输出 R/T。
**关键 API**：`cv::findChessboardCorners`、`cv::cornerSubPix`、`cv::stereoCalibrate`、`cv::calibrateCamera`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)。

### calibration.cpp
**功能**：单目张正友平面标定与去畸变完整流程。
**流程**：棋盘多姿态 → `findChessboardCorners`+`cornerSubPix` → `calibrateCameraRO` → `projectPoints` 误差 → `initUndistortRectifyMap`+`remap`。
**关键 API**：`cv::findChessboardCorners`、`cv::calibrateCameraRO`、`cv::projectPoints`、`cv::initUndistortRectifyMap`、`cv::remap`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md) §7.1。

### epipolar_lines.cpp
**功能**：对极几何与极线绘制，演示 `F/E` 约束。
**流程**：标定两视图 → `findFundamentalMat`/`findEssentialMat` → `computeCorrespondEpilines` → 绘制极线。
**关键 API**：`cv::findFundamentalMat`、`cv::findEssentialMat`、`cv::computeCorrespondEpilines`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)。

### essential_mat_reconstr.cpp
**功能**：本征矩阵 + 位姿恢复 + 三角化三维重建。
**流程**：匹配点 → `findEssentialMat` → `recoverPose` → `triangulatePoints` → 输出点云。
**关键 API**：`cv::findEssentialMat`、`cv::recoverPose`、`cv::triangulatePoints`、`cv::convertPointsFromHomogeneous`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)。

### select3dobj.cpp
**功能**：3D 物体选取与 PnP 位姿跟踪。
**流程**：训练特征 → 匹配 → `solvePnP` → `projectPoints` 投影 3D 框 → 显示位姿。
**关键 API**：`cv::solvePnP`/`solvePnPRansac`、`cv::projectPoints`、`cv::findHomography`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)。

### stereo_calib.cpp
**功能**：双目标定与极线校正，估计左右内外参。
**流程**：双目棋盘采集 → `stereoCalibrate` → `stereoRectify` → `initUndistortRectifyMap` 左右 → 显示校正行对齐。
**关键 API**：`cv::stereoCalibrate`、`cv::stereoRectify`、`cv::initUndistortRectifyMap`、`cv::findChessboardCorners`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)。

### stereo_match.cpp
**功能**：双目立体匹配生成视差图与深度。
**流程**：校正左右图 → `StereoBM`/`StereoSGBM::create` → `compute` 视差 → `reprojectImageTo3D` 深度。
**关键 API**：`cv::StereoBM`/`StereoSGBM`、`compute`、`cv::reprojectImageTo3D`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)。

### stitching.cpp
**功能**：图像拼接简版，`Stitcher` 一键全景。
**流程**：读多图 → `Stitcher::create` → `stitch` → 输出全景图。
**关键 API**：`cv::Stitcher::create`、`cv::Stitcher::stitch`、`cv::Stitcher::OK`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md)。

### stitching_detailed.cpp
**功能**：图像拼接详细版，可调特征/匹配/融合/曝光补偿各阶段。
**流程**：特征检测 → 匹配/单应估计 → `detail::BundleAdjuster` → `detail::ExposureCompensator` → `detail::MultiBandBlender` → 输出。
**关键 API**：`cv::detail::SurfFeaturesFinder`、`cv::detail::BestOf2NearestMatcher`、`cv::detail::BundleAdjusterRay`、`cv::detail::MultiBandBlender`。
**关联**：[principles.md §15](./principles.md#15-相机模型多视几何与拼接) ｜ [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md) §7.5。

---

## F.8 GUI / G-API / GPU（ch08，15 文件）

> 对应 [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)；原理见 [principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu)。

### audio_spectrogram.cpp
**功能**：音频频谱图绘制，演示 `MediaFoundation`/音频帧提取。
**流程**：开音频流 → 取帧 → `dft` 频谱 → `applyColorMap` → 显示瀑布图。
**关键 API**：`cv::VideoCapture`（音频）、`cv::dft`、`cv::applyColorMap`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### drawing.cpp
**功能**：基本绘图原子（线/圆/多边形/文字）演示。
**流程**：构造画布 Mat → `line`/`circle`/`ellipse`/`putText`/`fillPoly` → 显示。
**关键 API**：`cv::line`、`cv::circle`、`cv::ellipse`、`cv::putText`、`cv::fillPoly`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_audio.cpp
**功能**：视频同步音频捕获基础。
**流程**：`VideoCapture::open`(含音频流) → 逐帧读视频/音频 → 输出流信息。
**关键 API**：`cv::VideoCapture`、`CAP_PROP_*`、`retrieve`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_audio_combination.cpp
**功能**：多路音视频流组合捕获。
**流程**：开多 `VideoCapture` → 同步取帧 → 合并/转码 → 显示。
**关键 API**：`cv::VideoCapture`、`grab`/`retrieve`、`CAP_PROP_AUDIO_STREAM`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_basic.cpp
**功能**：视频/相机捕获最简入门。
**流程**：`VideoCapture::open`(文件/相机) → 循环 `read` → `imshow` → ESC 退出。
**关键 API**：`cv::VideoCapture`、`cv::VideoCapture::read`、`cv::imshow`、`cv::waitKey`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_camera.cpp
**功能**：相机分辨率/曝光参数枚举与设置。
**流程**：`VideoCapture::open`(相机索引) → `set` `CAP_PROP_*` → 显示当前值与帧。
**关键 API**：`cv::VideoCapture::set`/`get`、`CAP_PROP_FRAME_WIDTH/HEIGHT/FPS/EXPOSURE`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_gphoto2_autofocus.cpp
**功能**：gPhoto2 驱动相机的自动对焦演示。
**流程**：`VideoCapture::open`(CAP_GPHOTO2) → `set` `CAP_PROP_AUTOFOCUS` → 显示对焦状态。
**关键 API**：`cv::VideoCapture`、`CAP_GPHOTO2`、`CAP_PROP_AUTOFOCUS`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_gstreamer_pipeline.cpp
**功能**：通过 GStreamer pipeline 字符串打开视频流。
**流程**：构造 pipeline → `VideoCapture::open`(pipeline, CAP_GSTREAMER) → `read` → 显示。
**关键 API**：`cv::VideoCapture`、`CAP_GSTREAMER`、`CAP_PROP_GSTREAMER_*`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_image_sequence.cpp
**功能**：把图像序列当视频流读取。
**流程**：构造 `ImageCollection`/通配路径 → `VideoCapture::open`(序列) → 逐帧 `read` → 显示。
**关键 API**：`cv::VideoCapture`、`CAP_PROP_IMAGES_BASE`、`CAP_PROP_IMAGE_SEQUENCE`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_microphone.cpp
**功能**：麦克风音频流捕获。
**流程**：`VideoCapture::open`(CAP_MSMF, 音频) → 取音频帧 → 输出采样信息。
**关键 API**：`cv::VideoCapture`、`CAP_MSMF`、`CAP_PROP_AUDIO_*`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_obsensor.cpp
**功能**：Orbbec 深度相机捕获（RGB + depth）。
**流程**：`VideoCapture::open`(CAP_OBSENSOR) → `grab`+`retrieve` 取深度/彩色 → 显示。
**关键 API**：`cv::VideoCapture`、`CAP_OBSENSOR`、`CAP_OBSENSOR_DEPTH_MAP`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_openni.cpp
**功能**：OpenNI 兼容深度相机捕获。
**流程**：`VideoCapture::open`(CAP_OPENNI2) → `retrieve` 深度/点云 → `imshow` 着色。
**关键 API**：`cv::VideoCapture`、`CAP_OPENNI2`、`CAP_OPENNI_DEPTH_MAP`/`CAP_OPENNI_POINT_CLOUD_MAP`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_realsense.cpp
**功能**：Intel RealSense 深度相机捕获与对齐。
**流程**：`VideoCapture::open`(CAP_REALSENSE) → 取 RGB + depth + IR → 显示多流。
**关键 API**：`cv::VideoCapture`、`CAP_REALSENSE`、`CAP_PROP_REALSENSE_*`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videocapture_starter.cpp
**功能**：视频捕获最简可编译入门模板。
**流程**：`VideoCapture::open`(参数) → `isOpened` 校验 → 循环 `read`+`imshow` → `waitKey`。
**关键 API**：`cv::VideoCapture`、`isOpened`、`read`、`imshow`、`waitKey`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

### videowriter_basic.cpp
**功能**：视频写入最简入门，演示编码参数与帧率。
**流程**：构造 `VideoWriter`(`fourcc`, fps, size) → 循环 `write`/`<<` → `release` 输出文件。
**关键 API**：`cv::VideoWriter`、`cv::VideoWriter::fourcc`、`write`、`CAP_PROP_FPS`。
**关联**：[principles.md §16](./principles.md#16-highguivideo-iog-api-与-gpu) ｜ [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md)。

---

## F.T tutorial_code 主题索引（136 文件）

> 下列按 `tutorial_code` 的 20 个主题目录分组，仅作主题索引；流程深读请对照同主题根目录示例与对应分章。
> 路径相对 `mingw-build/samples/cpp/tutorial_code/`。

### F.T.1 calib3d（11 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `calib3d/camera_calibration/camera_calibration.cpp` | 张正友平面标定 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/CsvReader.cpp` | CSV 工具（多文件工程） | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/CsvWriter.cpp` | CSV 工具 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/main_detection.cpp` | 实时位姿检测入口 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/main_registration.cpp` | 实时位姿注册入口 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/Mesh.cpp` | 3D 网格工具 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/Model.cpp` | 3D 模型工具 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/ModelRegistration.cpp` | 模型注册 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/PnPProblem.cpp` | PnP 求解封装 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/RobustMatcher.cpp` | 鲁棒匹配器 | [ch07](./ch07_calib3d_stitching.md) |
| `calib3d/real_time_pose_estimation/src/Utils.cpp` | 通用工具 | [ch07](./ch07_calib3d_stitching.md) |

### F.T.2 core（13 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `core/AddingImages/AddingImages.cpp` | 线性混合叠加 | [ch01](./ch01_core.md) |
| `core/discrete_fourier_transform/discrete_fourier_transform.cpp` | DFT 频域处理 | [ch01](./ch01_core.md) |
| `core/file_input_output/file_input_output.cpp` | XML/YAML 文件 I/O | [ch01](./ch01_core.md) |
| `core/how_to_scan_images/how_to_scan_images.cpp` | Mat 像素遍历 | [ch01](./ch01_core.md) |
| `core/how_to_use_OpenCV_parallel_for_/how_to_use_OpenCV_parallel_for_.cpp` | parallel_for_ 并行 | [ch01](./ch01_core.md) |
| `core/how_to_use_OpenCV_parallel_for_/how_to_use_OpenCV_parallel_for_new.cpp` | parallel_for_ 新写法 | [ch01](./ch01_core.md) |
| `core/mat_mask_operations/mat_mask_operations.cpp` | 掩膜运算 | [ch01](./ch01_core.md) |
| `core/mat_operations/mat_operations.cpp` | Mat 基本操作 | [ch01](./ch01_core.md) |
| `core/mat_the_basic_image_container/mat_the_basic_image_container.cpp` | Mat 容器概念 | [ch01](./ch01_core.md) |
| `core/parallel_backend/example-openmp.cpp` | OpenMP 后端 | [ch01](./ch01_core.md) |
| `core/parallel_backend/example-tbb.cpp` | TBB 后端 | [ch01](./ch01_core.md) |
| `core/univ_intrin/univ_intrin.cpp` | 通用 SIMD | [ch01](./ch01_core.md) |

### F.T.3 features2D（11 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `features2D/AKAZE_match.cpp` | AKAZE 描述与匹配 | [ch03](./ch03_features.md) |
| `features2D/AKAZE_tracking/planar_tracking.cpp` | AKAZE 平面跟踪 | [ch03](./ch03_features.md) |
| `features2D/feature_description/SURF_matching_Demo.cpp` | SURF 描述匹配 | [ch03](./ch03_features.md) |
| `features2D/feature_detection/SURF_detection_Demo.cpp` | SURF 检测 | [ch03](./ch03_features.md) |
| `features2D/feature_flann_matcher/SURF_FLANN_matching_Demo.cpp` | SURF+FLANN 匹配 | [ch03](./ch03_features.md) |
| `features2D/feature_homography/SURF_FLNN_matching_homography_Demo.cpp` | SURF 匹配+单应 | [ch03](./ch03_features.md) |
| `features2D/Homography/decompose_homography.cpp` | 单应矩阵分解 | [ch03](./ch03_features.md) |
| `features2D/Homography/homography_from_camera_displacement.cpp` | 相机位移单应 | [ch03](./ch03_features.md) |
| `features2D/Homography/panorama_stitching_rotating_camera.cpp` | 旋转相机全景拼接 | [ch03](./ch03_features.md) |
| `features2D/Homography/perspective_correction.cpp` | 透视校正 | [ch03](./ch03_features.md) |
| `features2D/Homography/pose_from_homography.cpp` | 由单应求位姿 | [ch03](./ch03_features.md) |

### F.T.4 gapi（10 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `gapi/age_gender_emotion_recognition/age_gender_emotion_recognition.cpp` | 年龄/性别/情绪流水线 | [ch08](./ch08_gui_gapi_gpu.md) |
| `gapi/doc_snippets/api_ref_snippets.cpp` | API 参考片段 | [ch08](./ch08_gui_gapi_gpu.md) |
| `gapi/doc_snippets/dynamic_graph_snippets.cpp` | 动态图片段 | [ch08](./ch08_gui_gapi_gpu.md) |
| `gapi/doc_snippets/kernel_api_snippets.cpp` | 内核 API 片段 | [ch08](./ch08_gui_gapi_gpu.md) |
| `gapi/face_beautification/face_beautification.cpp` | 人脸美颜流水线 | [ch08](./ch08_gui_gapi_gpu.md) |
| `gapi/porting_anisotropic_image_segmentation/porting_anisotropic_image_segmentation_gapi.cpp` | 各向异性分割移植 | [ch08](./ch08_gui_gapi_gpu.md) |
| `gapi/porting_anisotropic_image_segmentation/porting_anisotropic_image_segmentation_gapi_fluid.cpp` | Fluid 后端分割 | [ch08](./ch08_gui_gapi_gpu.md) |
| `gapi/security_barrier_camera/security_barrier_camera.cpp` | 安防道闸流水线 | [ch08](./ch08_gui_gapi_gpu.md) |

### F.T.5 gpu（1 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `gpu/gpu-basics-similarity/gpu-basics-similarity.cpp` | GPU 基础相似运算 | [ch08](./ch08_gui_gapi_gpu.md) |

### F.T.6 HighGUI（2 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `HighGUI/AddingImagesTrackbar.cpp` | 滑条线性混合 | [ch01](./ch01_core.md)/[ch02](./ch02_imgproc.md) |
| `HighGUI/BasicLinearTransformsTrackbar.cpp` | 滑条对比度亮度 | [ch02](./ch02_imgproc.md) |

### F.T.7 Histograms_Matching（6 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `Histograms_Matching/calcBackProject_Demo1.cpp` | 直方图反投影 1 | [ch02](./ch02_imgproc.md) |
| `Histograms_Matching/calcBackProject_Demo2.cpp` | 直方图反投影 2 | [ch02](./ch02_imgproc.md) |
| `Histograms_Matching/calcHist_Demo.cpp` | 直方图计算 | [ch02](./ch02_imgproc.md) |
| `Histograms_Matching/compareHist_Demo.cpp` | 直方图比较 | [ch02](./ch02_imgproc.md) |
| `Histograms_Matching/EqualizeHist_Demo.cpp` | 直方图均衡 | [ch02](./ch02_imgproc.md) |
| `Histograms_Matching/MatchTemplate_Demo.cpp` | 模板匹配 | [ch02](./ch02_imgproc.md) |

### F.T.8 imgcodecs（2 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `imgcodecs/animations.cpp` | 动图/序列读写 | [ch01](./ch01_core.md) |
| `imgcodecs/GDAL_IO/gdal-image.cpp` | GDAL 地理影像 I/O | [ch01](./ch01_core.md) |

### F.T.9 ImgProc（19 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `ImgProc/anisotropic_image_segmentation/anisotropic_image_segmentation.cpp` | 各向异性分割 | [ch02](./ch02_imgproc.md) |
| `ImgProc/basic_drawing/Drawing_1.cpp` | 基本绘图 1 | [ch02](./ch02_imgproc.md) |
| `ImgProc/basic_drawing/Drawing_2.cpp` | 基本绘图 2 | [ch02](./ch02_imgproc.md) |
| `ImgProc/BasicLinearTransforms.cpp` | 对比度亮度线性变换 | [ch02](./ch02_imgproc.md) |
| `ImgProc/changing_contrast_brightness_image/changing_contrast_brightness_image.cpp` | 对比度亮度交互 | [ch02](./ch02_imgproc.md) |
| `ImgProc/HitMiss/HitMiss.cpp` | 击中击不中变换 | [ch02](./ch02_imgproc.md) |
| `ImgProc/morph_lines_detection/Morphology_3.cpp` | 形态学直线提取 | [ch02](./ch02_imgproc.md) |
| `ImgProc/Morphology_1.cpp` | 形态学腐蚀膨胀 | [ch02](./ch02_imgproc.md) |
| `ImgProc/Morphology_2.cpp` | 形态学开闭运算 | [ch02](./ch02_imgproc.md) |
| `ImgProc/motion_deblur_filter/motion_deblur_filter.cpp` | 运动去模糊维纳滤波 | [ch02](./ch02_imgproc.md) |
| `ImgProc/out_of_focus_deblur_filter/out_of_focus_deblur_filter.cpp` | 离焦去模糊 | [ch02](./ch02_imgproc.md) |
| `ImgProc/periodic_noise_removing_filter/periodic_noise_removing_filter.cpp` | 周期噪声去除 | [ch02](./ch02_imgproc.md) |
| `ImgProc/Pyramids/Pyramids.cpp` | 图像金字塔 | [ch02](./ch02_imgproc.md) |
| `ImgProc/Smoothing/Smoothing.cpp` | 平滑滤波 | [ch02](./ch02_imgproc.md) |
| `ImgProc/Threshold.cpp` | 阈值化 | [ch02](./ch02_imgproc.md) |
| `ImgProc/Threshold_inRange.cpp` | 色彩区间阈值 | [ch02](./ch02_imgproc.md) |

### F.T.10 ImgTrans（14 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `ImgTrans/CannyDetector_Demo.cpp` | Canny 边缘演示 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/copyMakeBorder_demo.cpp` | 边界填充 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/filter2D_demo.cpp` | filter2D 自定义核 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/generalizedHoughTransform.cpp` | 广义霍夫变换 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/Geometric_Transforms_Demo.cpp` | 几何变换交互 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/HoughCircle_Demo.cpp` | 霍夫圆检测 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/houghcircles.cpp` | 霍夫圆（教程版） | [ch02](./ch02_imgproc.md) |
| `ImgTrans/houghlines.cpp` | 霍夫线（教程版） | [ch02](./ch02_imgproc.md) |
| `ImgTrans/HoughLines_Demo.cpp` | 霍夫线演示 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/imageSegmentation.cpp` | 交互式分割 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/Laplace_Demo.cpp` | Laplacian 演示 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/Remap_Demo.cpp` | Remap 重映射 | [ch02](./ch02_imgproc.md) |
| `ImgTrans/Sobel_Demo.cpp` | Sobel 演示 | [ch02](./ch02_imgproc.md) |

### F.T.11 introduction（3 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `introduction/display_image/display_image.cpp` | 显示图像入门 | [ch01](./ch01_core.md) |
| `introduction/documentation/documentation.cpp` | 文档引导 | [ch01](./ch01_core.md) |
| `introduction/windows_visual_studio_opencv/introduction_windows_vs.cpp` | VS 环境入门 | [ch01](./ch01_core.md) |

### F.T.12 ml（3 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `ml/introduction_to_pca/introduction_to_pca.cpp` | PCA 入门 | [ch05](./ch05_ml.md) |
| `ml/introduction_to_svm/introduction_to_svm.cpp` | SVM 入门 | [ch05](./ch05_ml.md) |
| `ml/non_linear_svms/non_linear_svms.cpp` | 非线性 SVM | [ch05](./ch05_ml.md) |

### F.T.13 objectDetection（11 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `objectDetection/calibrate_camera.cpp` | 相机标定（ChArUco 配套） | [ch06](./ch06_objdetect_photo.md)/[ch07](./ch07_calib3d_stitching.md) |
| `objectDetection/calibrate_camera_charuco.cpp` | ChArUco 相机标定 | [ch06](./ch06_objdetect_photo.md)/[ch07](./ch07_calib3d_stitching.md) |
| `objectDetection/create_board.cpp` | ArUco 板生成 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/create_board_charuco.cpp` | ChArUco 板生成 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/create_diamond.cpp` | 钻石标记生成 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/create_marker.cpp` | 单标记生成 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/detect_board.cpp` | ArUco 板检测 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/detect_board_charuco.cpp` | ChArUco 板检测 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/detect_diamonds.cpp` | 钻石标记检测 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/detect_markers.cpp` | ArUco 标记检测 | [ch06](./ch06_objdetect_photo.md) |
| `objectDetection/objectDetection.cpp` | HOG 行人检测 | [ch06](./ch06_objdetect_photo.md) |

### F.T.14 photo（5 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `photo/decolorization/decolor.cpp` | 彩色转灰度去色 | [ch06](./ch06_objdetect_photo.md) |
| `photo/hdr_imaging/hdr_imaging.cpp` | HDR 高动态范围成像 | [ch06](./ch06_objdetect_photo.md) |
| `photo/non_photorealistic_rendering/npr_demo.cpp` | 非真实感渲染 | [ch06](./ch06_objdetect_photo.md) |
| `photo/seamless_cloning/cloning_demo.cpp` | 无缝克隆（教程版） | [ch06](./ch06_objdetect_photo.md) |
| `photo/seamless_cloning/cloning_gui.cpp` | 无缝克隆交互（教程版） | [ch06](./ch06_objdetect_photo.md) |

### F.T.15 ShapeDescriptors（6 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `ShapeDescriptors/findContours_demo.cpp` | 轮廓查找 | [ch03](./ch03_features.md) |
| `ShapeDescriptors/generalContours_demo1.cpp` | 通用轮廓属性 1 | [ch03](./ch03_features.md) |
| `ShapeDescriptors/generalContours_demo2.cpp` | 通用轮廓属性 2 | [ch03](./ch03_features.md) |
| `ShapeDescriptors/hull_demo.cpp` | 凸包演示 | [ch03](./ch03_features.md) |
| `ShapeDescriptors/moments_demo.cpp` | 矩与质心 | [ch03](./ch03_features.md) |
| `ShapeDescriptors/pointPolygonTest_demo.cpp` | 点-多边形测试 | [ch03](./ch03_features.md) |

### F.T.16 snippets（12 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `snippets/core_mat_checkVector.cpp` | Mat 通道检查片段 | [ch01](./ch01_core.md) |
| `snippets/core_merge.cpp` | 通道合并片段 | [ch01](./ch01_core.md) |
| `snippets/core_reduce.cpp` | 归约片段 | [ch01](./ch01_core.md) |
| `snippets/core_split.cpp` | 通道拆分片段 | [ch01](./ch01_core.md) |
| `snippets/core_various.cpp` | 杂项片段 | [ch01](./ch01_core.md) |
| `snippets/imgcodecs_imwrite.cpp` | imwrite 片段 | [ch01](./ch01_core.md) |
| `snippets/imgproc_applyColorMap.cpp` | 伪彩片段 | [ch02](./ch02_imgproc.md) |
| `snippets/imgproc_calcHist.cpp` | 直方图片段 | [ch02](./ch02_imgproc.md) |
| `snippets/imgproc_drawContours.cpp` | 绘轮廓片段 | [ch03](./ch03_features.md) |
| `snippets/imgproc_HoughLinesCircles.cpp` | 霍夫圆片段 | [ch02](./ch02_imgproc.md) |
| `snippets/imgproc_HoughLinesP.cpp` | 概率霍夫线片段 | [ch02](./ch02_imgproc.md) |
| `snippets/imgproc_HoughLinesPointSet.cpp` | 点集霍夫片段 | [ch02](./ch02_imgproc.md) |
| `snippets/imgproc_segmentation.cpp` | 分割片段 | [ch02](./ch02_imgproc.md) |

### F.T.17 TrackingMotion（4 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `TrackingMotion/cornerDetector_Demo.cpp` | 角点检测器 | [ch03](./ch03_features.md)/[ch04](./ch04_video.md) |
| `TrackingMotion/cornerHarris_Demo.cpp` | Harris 角点 | [ch03](./ch03_features.md) |
| `TrackingMotion/cornerSubPix_Demo.cpp` | 亚像素角点 | [ch03](./ch03_features.md) |
| `TrackingMotion/goodFeaturesToTrack_Demo.cpp` | Shi-Tomasi 角点 | [ch03](./ch03_features.md) |

### F.T.18 video（5 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `video/bg_sub.cpp` | 背景减除 | [ch04](./ch04_video.md) |
| `video/meanshift/camshift.cpp` | CamShift 跟踪 | [ch04](./ch04_video.md) |
| `video/meanshift/meanshift.cpp` | MeanShift 跟踪 | [ch04](./ch04_video.md) |
| `video/optical_flow/optical_flow.cpp` | 稀疏光流 | [ch04](./ch04_video.md) |
| `video/optical_flow/optical_flow_dense.cpp` | 稠密光流 | [ch04](./ch04_video.md) |

### F.T.19 videoio（3 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `videoio/openni_orbbec_astra/openni_orbbec_astra.cpp` | Orbbec Astra 深度捕获 | [ch08](./ch08_gui_gapi_gpu.md) |
| `videoio/video-input-psnr-ssim/video-input-psnr-ssim.cpp` | 视频 PSNR/SSIM 评测 | [ch08](./ch08_gui_gapi_gpu.md) |
| `videoio/video-write/video-write.cpp` | 视频写入 | [ch08](./ch08_gui_gapi_gpu.md) |

### F.T.20 xfeatures2D（1 文件）

| 文件 | 主题 | 归章 |
| --- | --- | --- |
| `xfeatures2D/LATCH_match.cpp` | LATCH 描述子匹配 | [ch03](./ch03_features.md) |

---

> 本流程篇与 [principles.md](./principles.md)（理论）、[ch01](./ch01_core.md)–[ch08](./ch08_gui_gapi_gpu.md)（分章深读）、[README.md](./README.md) 附录 B（233 文件清单）配套使用。
