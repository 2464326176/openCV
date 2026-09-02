# L2 imgproc 主干

**目标**：吃透 imgproc 全主题。日常图像处理核心，**约 1 周**。

**黄金主线**：01 → 08 → 13 → 10 → 14 → 26 → 30 → 32

**建议顺序**：01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 → 10 → 11 → 12 → 13 → 14 → 15 → 16 → 17 → 18 → 19 → 20 → 21 → 22 → 23 → 24 → 25 → 26 → 27 → 28 → 29 → 30 → 31 → 32 → 33 → 34 → 35 → 36

**验收**：能脱稿写出「读图 → 灰度 → 高斯 → Canny → findContours」与「HSV inRange 颜色分割」；能说清「高斯/中值/双边三种滤波各适合什么噪声」。

## 核心概念速览

| 主题 | 关键词 | 心智模型 |
| --- | --- | --- |
| 线性滤波 | 方框/均值/高斯/卷积 | 邻域加权求和，可分离核加速 |
| 非线性滤波 | 中值/双边 | 中值抗椒盐，双边保边去噪 |
| 自定义核 | filter2D/锐化/边缘 | 手写卷积核实现任意线性运算 |
| 形态学 | 腐蚀/膨胀/开闭/梯度/顶帽黑帽 | 结构元取 min/max，组合出七种运算 |
| 击中击不中 | HitMiss/细结构 | 用两个结构元匹配精确形状 |
| 形态学提取线 | 水平/垂直核 | 用矩形结构元提取表格线 |
| 阈值 | threshold/OTSU/自适应 | 全局/自动/局部阈值二值化 |
| 颜色分割 | inRange/HSV | H 对光照不敏感，按色域范围掩膜 |
| 梯度 | Sobel/Scharr/Laplacian | 一阶/二阶导数近似边缘 |
| Canny | NMS/双阈值/滞后连接 | 五步最优边缘检测 |
| Hough 直线 | 标准Hough/概率Hough | 参数空间投票找直线 |
| Hough 圆 | 圆心+半径投票 | 三维参数空间降维加速 |
| 仿射 | warpAffine/旋转/平移/缩放 | 2×3 矩阵变换 |
| 透视 | warpPerspective/4点 | 3×3 投影变换 |
| 重映射 | remap/map_x/map_y | 浮点坐标查表做任意几何变形 |
| 边界 | copyMakeBorder/外推 | 卷积前补边避免边界截断 |
| 极坐标 | warpPolar | 笛卡尔↔极坐标转换圆环展开 |
| 距离变换 | distanceTransform/_foreground | 前景到最近背景距离，分水岭种子 |
| 连通域 | connectedComponents/stats | 标记连通分量并统计面积/质心 |
| 漫水填充 | floodFill/连通域染色 | 种子生长式区域分割 |
| 分水岭 | watershed/标记/淹没 | 基于距离变换的山脊分割 |
| 直方图 | calcHist/bin/range | 统计灰度/颜色分布 |
| 均衡化 | equalizeHist/CLAHE | CDF 映射拉平直方图增强对比度 |
| 直方图对比 | compareHist/4 种距离 | 巴氏/相关/卡方/交集 |
| 反向投影 | calcBackProject | 直方图反查像素属于目标概率 |
| 模板匹配 | matchTemplate/6 方法 | 滑窗打分定位模板位置 |
| 金字塔 | pyrUp/pyrDown/拉普拉斯 | 多尺度分析与融合 |
| 相位相关 | phaseCorrelate/FFT | 频域互相关求位移 |
| 亮度对比度 | convertTo/trackbar | $g=\alpha f+\beta$ 实时调参 |
| 各向异性分割 | 结构张量/coherence | 用局部一致性做纹理分割 |
| 运动去模糊 | DFT/Wiener | 频域逆滤波近似复原 |
| 广义霍夫 | GeneralizedHoughBallard | 非参数形状检测 |

## 官方对照表（节选 + 新增）

| 练习文件 | 标签 | 官方 sample | docs | 必会 API |
| - | - | - | - | - |
| [01_smoothing.cpp](01_smoothing.cpp) | 主线 | `Smoothing.cpp` | ch02 §1 | `GaussianBlur` / `bilateralFilter` |
| [02_filter2d.cpp](02_filter2d.cpp) | 主线 | `tutorial_code/ImgTrans/filter2D_demo.cpp` | ch02 §1 | `filter2D` |
| [03_linear_transform_colormap.cpp](03_linear_transform_colormap.cpp) | 进阶 | `BasicLinearTransforms.cpp`、`applyColorMap` 片段 | ch02 §1 | `convertTo` / `applyColorMap` |
| [04_erode_dilate.cpp](04_erode_dilate.cpp) | 主线 | `Morphology_1.cpp` | ch02 §3 | `erode` / `dilate` / `getStructuringElement` |
| [05_morphology_ex.cpp](05_morphology_ex.cpp) | 主线 | `Morphology_2.cpp` | ch02 §3 | `morphologyEx`（七种运算） |
| [06_hit_miss.cpp](06_hit_miss.cpp) | 进阶 | `HitMiss.cpp` | ch02 §3 | `morphologyEx(MORPH_HITMISS)` |
| [07_morph_lines.cpp](07_morph_lines.cpp) | 进阶 | `Morphology_3.cpp` | ch02 §3 | 水平/垂直结构元提线 |
| [08_threshold.cpp](08_threshold.cpp) | 主线 | `Threshold.cpp` | ch02 §3 | `threshold` / `THRESH_OTSU` |
| [09_inrange_hsv.cpp](09_inrange_hsv.cpp) | 主线 | `Threshold_inRange.cpp` | ch02 §3 | `cvtColor(BGR2HSV)` / `inRange` |
| [10_sobel.cpp](10_sobel.cpp) | 主线 | `Sobel_Demo.cpp` | ch02 §4 | `Sobel` / `convertScaleAbs` |
| [11_scharr.cpp](11_scharr.cpp) | 进阶 | `Sobel_Demo.cpp`（Scharr） | ch02 §4 | `Scharr`（小核更精确） |
| [12_laplacian.cpp](12_laplacian.cpp) | 进阶 | `Laplace_Demo.cpp` | ch02 §4 | `Laplacian`（二阶边缘） |
| [13_canny.cpp](13_canny.cpp) | 主线 | `CannyDetector_Demo.cpp` | ch02 §4 | `Canny`（高低阈值 2:1~3:1） |
| [14_hough_lines.cpp](14_hough_lines.cpp) | 主线 | `HoughLines_Demo.cpp`、`houghlines.cpp` | ch02 §5 | `HoughLines`（标准） |
| [15_hough_lines_p.cpp](15_hough_lines_p.cpp) | 主线 | `HoughLinesP` 片段 | ch02 §5 | `HoughLinesP`（概率，更快） |
| [16_hough_circles.cpp](16_hough_circles.cpp) | 进阶 | `HoughCircle_Demo.cpp`、`houghcircles.cpp` | ch02 §5 | `HoughCircles` |
| [17_warp_affine.cpp](17_warp_affine.cpp) | 主线 | `Geometric_Transforms_Demo.cpp` | ch02 §6 | `warpAffine` / `getRotationMatrix2D` |
| [18_warp_perspective.cpp](18_warp_perspective.cpp) | 进阶 | `warpPerspective_demo.cpp` | ch02 §6 | `warpPerspective` / `getPerspectiveTransform` |
| [19_remap.cpp](19_remap.cpp) | 进阶 | `Remap_Demo.cpp` | ch02 §6 | `remap` / `map_x` / `map_y` |
| [20_copy_make_border.cpp](20_copy_make_border.cpp) | 进阶 | `copyMakeBorder_demo.cpp` | ch02 §6 | `copyMakeBorder`（外推方式） |
| [21_polar_transform.cpp](21_polar_transform.cpp) | 进阶 | `polar_transforms.cpp` | ch02 §6 | `warpPolar` |
| [22_distance_transform.cpp](22_distance_transform.cpp) | 进阶 | `distrans.cpp` | ch02 §7 | `distanceTransform` |
| [23_connected_components.cpp](23_connected_components.cpp) | 进阶 | `connected_components.cpp` | ch02 §7 | `connectedComponentsWithStats` |
| [24_flood_fill.cpp](24_flood_fill.cpp) | 进阶 | `ffilldemo.cpp` | ch02 §7 | `floodFill` |
| [25_watershed.cpp](25_watershed.cpp) | 进阶 | `imageSegmentation.cpp`、`watershed.cpp` | ch02 §7 | `watershed`（标记驱动） |
| [26_calc_hist.cpp](26_calc_hist.cpp) | 主线 | `calcHist_Demo.cpp` | ch02 §8 | `calcHist` / `normalize` |
| [27_equalize_clahe.cpp](27_equalize_clahe.cpp) | 主线 | `EqualizeHist_Demo.cpp` | ch02 §8 | `equalizeHist` / `createCLAHE` |
| [28_compare_hist.cpp](28_compare_hist.cpp) | 进阶 | `compareHist_Demo.cpp` | ch02 §8 | `compareHist`（4 种距离） |
| [29_backproject.cpp](29_backproject.cpp) | 进阶 | `calcBackProject_Demo1.cpp` | ch02 §8 | `calcBackProject` |
| [30_match_template.cpp](30_match_template.cpp) | 主线 | `MatchTemplate_Demo.cpp` | ch02 §8 | `matchTemplate` / `minMaxLoc` |
| [31_pyramids.cpp](31_pyramids.cpp) | 进阶 | `Pyramids.cpp` | ch02 §9 | `pyrUp` / `pyrDown` |
| [32_phase_correlate.cpp](32_phase_correlate.cpp) | 进阶 | `phase_corr.cpp` | ch02 §6 | `phaseCorrelate` |
| [33_contrast_brightness_trackbar.cpp](33_contrast_brightness_trackbar.cpp) | 进阶 | `changing_contrast_brightness_image.cpp` | ch02 §1 | `convertTo` / trackbar |
| [34_anisotropic_segmentation.cpp](34_anisotropic_segmentation.cpp) | 进阶 | `anisotropic_image_segmentation.cpp` | ch02 §2.4.4 | 结构张量 / coherence |
| [35_motion_deblur.cpp](35_motion_deblur.cpp) | 选修 | `motion_deblur_filter.cpp` | ch02 §2.9.5 | DFT / Wiener 近似 |
| [36_generalized_hough.cpp](36_generalized_hough.cpp) | 选修 | `generalizedHoughTransform.cpp` | ch02 §5 | `createGeneralizedHoughBallard` |

## 重复主题说明

- `03_linear_transform_colormap` 与 `33_contrast_brightness_trackbar`：03 偏伪彩色+线性变换，33 偏交互调参，可先后做。
- `10_sobel` / `11_scharr` / `12_laplacian`：梯度算子对比，建议连续学习。
- `14_hough_lines` / `15_hough_lines_p`：标准 vs 概率霍夫，后者更快且只画检测段。
- `25_watershed` 与 `22_distance_transform`：分水岭的种子常由距离变换的极大值提供，建议连做。

## 关键易错点

| 练习 | 易错点 / 关键参数 |
| --- | --- |
| 01 滤波 | 高斯/中值的 `ksize` 必须为奇数；椒盐噪声用中值不用高斯 |
| 04 形态学 | 结构元 `Size` 越大效果越强但越粗；`MORPH_RECT/CROSS/ELLIPSE` 选形状 |
| 08 阈值 | `THRESH_OTSU` 自动求阈值但需 `threshold` 传 0；自适应阈值 `blockSize` 取奇数 |
| 09 HSV | OpenCV 中 H 范围 0~179（非 360）；S/V 为 0~255 |
| 10/11 梯度 | `ddepth=CV_16S` 防负值截断；`convertScaleAbs` 转回 8 位显示 |
| 13 Canny | 高低阈值比 2:1~3:1；先 `blur` 降噪再 Canny |
| 16 Hough 圆 | `param2` 越大越严；`minDist` 防同心圆重复检测 |
| 17 仿射 | `getRotationMatrix2D` 需指定旋转中心；输出尺寸要算好不然裁切 |
| 19 remap | `map_x/map_y` 为 `CV_32FC1` 浮点；坐标越界用 `BORDER_CONSTANT` 填充 |
| 22 距离 | `DIST_L2` 为欧氏；输出需 `normalize` 才显示 |
| 26 直方图 | `ranges` 上界 exclusive（如 `[0,256)`）；`bin` 数影响精度 |
| 27 CLAHE | `clipLimit` 控制对比度增强强度；`tileGridSize` 分块大小 |
| 30 模板 | `TM_SQDIFF` 取最小值最佳，其余取最大值；结果需 `normalize` |

## 选做

- 频域去模糊：`35_motion_deblur.cpp`（已提供最小版）
- 广义霍夫：`36_generalized_hough.cpp`
