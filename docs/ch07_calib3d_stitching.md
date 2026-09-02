# 第 7 章 calib3d 与 stitching：从投影几何到全景优化

> 本章以仓库内 `mingw-build/samples/cpp` 的官方示例为源码基准。主线是“成像模型 → 标定 → 两视图几何 → 深度/位姿 → 拼接”，不包含编译与环境配置。

---

## 7.0 章节导言

三维视觉不是“从二维图里直接读出三维”，而是先写出图像如何由三维生成，再利用多帧、多相机或已知物体提供的约束反求未知量。全章统一使用世界到相机的外参约定：

$$
\mathbf X_c=R\mathbf X_w+t,\qquad
Z_c\begin{bmatrix}u\\v\\1\end{bmatrix}
=K\begin{bmatrix}R&t\end{bmatrix}
\begin{bmatrix}X_w\\Y_w\\Z_w\\1\end{bmatrix},
\quad
K=\begin{bmatrix}f_x&s&c_x\\0&f_y&c_y\\0&0&1\end{bmatrix}.
$$

`K` 是相机内部的像素尺度与主点，`R,t` 是某次拍摄时世界坐标到相机坐标的刚体变换。内参与外参不能混为一谈：换机位会改变 `R,t`，但镜头焦距和成像尺寸不变时 `K` 不变；图像缩放则会等比例改变 `f_x,f_y,c_x,c_y`。

概念阅读顺序：

1. `calibration.cpp`：针孔模型、畸变与张正友平面标定；
2. `stereo_calib.cpp`、`stereo_match.cpp`：对极约束、校正、视差与深度；
3. `epipolar_lines.cpp`、`essential_mat_reconstr.cpp`：`F/E`、位姿恢复与三角化；
4. `select3dobj.cpp` 与[第 3 章单应分解示例](./ch03_features.md#326-homographydecompose_homographycpp)：PnP 和平面单应分解；
5. `stitching_detailed.cpp`：从匹配图到光束平差、曝光补偿与融合。

---

## 7.1 相机模型与单目标定

### 7.1.1 `calibration.cpp` —— 张正友平面标定与去畸变
> **源文件**：`samples/cpp/calibration.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`完整流程`

#### 功能概述

从多张棋盘格、圆点阵或 ChArUco 板图像中检测二维角点，建立平面标定板三维点与图像点的对应，联合估计内参、畸变和每张图的外参，并以重投影误差评估结果。源码还展示 `calibrateCameraRO` 的标定板非理想修正以及查表式去畸变。

#### 核心原理

**30 秒心智模型**：每张标定图都提供一个“已知平面经过未知相机投影后落在哪里”的约束；多种倾斜姿态让焦距、主点、畸变和板位姿不再互相冒充，最后用所有角点的重投影误差统一精化。

针孔模型先做透视除法：

$$
x=X_c/Z_c,\quad y=Y_c/Z_c,\qquad
u=f_x\,x+s\,y+c_x,\quad v=f_y\,y+c_y.
$$

OpenCV 常用的径向、切向畸变作用在归一化坐标上。令 $r^2=x^2+y^2$，基础五参数模型为：

$$
\begin{aligned}
x_d&=x(1+k_1r^2+k_2r^4+k_3r^6)+2p_1xy+p_2(r^2+2x^2),\\
y_d&=y(1+k_1r^2+k_2r^4+k_3r^6)+p_1(r^2+2y^2)+2p_2xy.
\end{aligned}
$$

径向项描述桶形/枕形弯曲，离主点越远影响越大；切向项来自镜头与传感器不完全平行，会产生方向相关偏移。`CALIB_RATIONAL_MODEL` 使用

$$
\frac{1+k_1r^2+k_2r^4+k_3r^6}{1+k_4r^2+k_5r^4+k_6r^6}
$$

替代简单径向多项式；`CALIB_THIN_PRISM_MODEL` 再加入 $s_1\ldots s_4$。模型越复杂并不必然越准确：观测不足时高阶系数会吸收噪声。

张正友法利用标定板 $Z=0$。每张图满足平面单应

$$
\lambda\mathbf x=K[r_1\ r_2\ t]\begin{bmatrix}X\\Y\\1\end{bmatrix}=H\mathbf X_\pi.
$$

由于 $r_1,r_2$ 正交且等长，$h_1^\top K^{-T}K^{-1}h_2=0$、$h_1^\top K^{-T}K^{-1}h_1=h_2^\top K^{-T}K^{-1}h_2$。每个姿态给出两条关于 $B=K^{-T}K^{-1}$ 的线性约束，先求 `K` 初值，再恢复各视图 `R,t`，最终最小化：

$$
\min_{K,d,\{R_i,t_i\}}\sum_{i,j}
\left\|\mathbf x_{ij}-\pi(K,d,R_i,t_i,\mathbf X_j)\right\|_2^2.
$$

源码对照：`findChessboardCorners`/`findCirclesGrid`/`CharucoDetector::detectBoard` 产生对应；棋盘角点经 `cornerSubPix` 精化；`runCalibration` 调 `calibrateCameraRO`；`computeReprojectionErrors` 调 `projectPoints` 逐视图统计误差；`initUndistortRectifyMap` 与 `remap` 把逆映射预计算后复用。

#### 关键 API

- `findChessboardCorners`、`findCirclesGrid`、`CharucoDetector::detectBoard`：检测标定板观测；
- `cornerSubPix`：在灰度局部梯度上精化角点；
- `calibrateCameraRO`：联合估计 `K`、`distCoeffs`、`rvecs/tvecs`，并可释放部分物点；
- `projectPoints`：按同一成像模型重投影，是误差评估的真值接口；
- `getOptimalNewCameraMatrix`、`initUndistortRectifyMap`、`remap`：控制保留视野并执行去畸变。

#### 处理流程

标定板规格生成 `objectPoints` → 检测并亚像素化 `imagePoints` → 收集多姿态有效视图 → `calibrateCameraRO` → `projectPoints` 计算逐视图误差 → 保存 `K/distCoeffs` → 预计算映射并检查直线是否恢复为直线。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `boardSize` | 内角点列数、行数 | 必须与板一致 | 填错会破坏点序与几何对应，不是“精度稍差” |
| `squareSize` | 相邻角点物理距离 | 实测值 | 只改变平移/尺寸的物理尺度；填错比例会使测距同比例错误 |
| 有效视图数与姿态 | 独立平面单应数量 | 通常 15–30 张 | 数量增加但姿态重复收益很小；应覆盖四角、距离和倾角 |
| `cornerSubPix` 窗口 | 局部精化范围 | 常用 $5$–$11$ 半窗 | 过小抓不住偏差，过大会混入邻角/纹理 |
| `CALIB_FIX_*` | 固定已有先验参数 | 按镜头先验 | 固定更多可降方差，却会把错误先验压进其他参数 |
| `CALIB_RATIONAL_MODEL` | 启用高阶径向模型 | 广角或残差明显时 | 数据不足时容易过拟合，边缘外推尤其不稳 |
| `alpha` | 新内参的裁剪—视野权衡 | `0` 到 `1` | `0` 少黑边但裁视野；`1` 留全视野但黑边增多 |

#### 关联与对比

`camera_calibration.cpp` 把同一流程封装进 `Settings` 并增加 `fisheye` 分支；鱼眼不是“多加几个普通畸变系数”，而是不同投影模型，不能混用命名空间。双目标定应先获得稳定单目内参，再固定内参估相机间外参。

#### 注意事项

- 只拍正对棋盘会使焦距与距离强耦合，RMS 可能好看但参数不可靠；
- 棋盘只占画面中心时无法约束边缘畸变；
- 总 RMS 会掩盖坏帧，应看逐视图误差和残差空间分布；
- 自动曝光、运动模糊、打印板翘曲会产生系统误差；`calibrateCameraRO` 只能缓解板尺寸非理想，不能修复模糊点；
- 缩放图像后必须同步缩放 `K`，畸变系数通常保持不变。

#### 应用场景

测量、AR 叠加、机器人定位、双目外参估计、去畸变和后续所有度量三维任务。

### 7.1.2 `3calibration.cpp` —— 三目共线标定与联合校正
> **源文件**：`samples/cpp/3calibration.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`完整流程`

#### 功能概述

先分别标定三台相机，再以中间相机为公共参考估计两组相对位姿，最后调用 `rectify3Collinear` 生成三路共线校正参数。

#### 核心原理

**30 秒心智模型**：三目不是重新发明标定模型，而是把两个共享中间相机的双目标定统一到同一校正坐标系。针孔、畸变和重投影目标见 7.1.1；相对位姿与极线约束见 7.2.1。共线先验使三路对应点经校正后沿同一行搜索，并让第三相机增加遮挡区和量程信息。

#### 关键 API

`calibrateCamera` 估每路内参；`stereoCalibrate` 估相机对外参；`rectify3Collinear` 联合输出 `R1/R2/R3`、`P1/P2/P3` 与 `Q`；`initUndistortRectifyMap`、`remap` 执行校正。

#### 处理流程

读取三路同步标定图 → 各路检测同序角点 → 三次单目标定 → 对共享相机的两组双目标定 → `rectify3Collinear` → 三路重映射和极线检查。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `CALIB_FIX_INTRINSIC` | 双目阶段固定三路内参 | 单目标定可靠时启用 | 降低自由度；错误内参会偏置外参 |
| 共线误差 | 三相机中心偏离共同基线 | 越小越好 | 偏离增大会留下校正后垂直视差 |
| `alpha` | 校正视野与黑边 | `-1` 或 `[0,1]` | 大保留视野，小裁掉无效边界 |
| 同步误差 | 三路采集时间差 | 静态板可放宽 | 运动板时增大会被误认为几何误差 |

#### 关联与对比

与 `stereo_calib.cpp` 相同地先估相机对，但本例多一个共享参考和 `rectify3Collinear`。任一相机内参失真都会传播到公共校正。

#### 注意事项

三路图像列表必须严格成组、角点序号一致；相机不近似共线时不应强套此模型；应分别检查 1–2 与 2–3 的垂直视差，而非只看拼排图。

#### 应用场景

三目深度、宽量程测距、遮挡补全和工业多相机阵列。

### 7.1.3 `camera_calibration.cpp` —— 配置驱动标定与鱼眼分支
> **源文件**：`samples/cpp/tutorial_code/calib3d/camera_calibration/camera_calibration.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`完整流程`

#### 功能概述

用 `Settings` 从 YAML/XML 读取标定板、输入源和模型选项，组织完整采集—标定—保存—去畸变流程，并提供标准针孔与 `fisheye` 两条分支。

#### 核心原理

**30 秒心智模型**：算法仍是 7.1.1 的重投影误差最小化；该示例的价值是把数据契约、状态机和模型选择显式化。`fisheye` 使用角度域畸变模型，不是普通 `k1...k6` 的简单加长版，因此 `cv::fisheye::*` 与标准 API 不能混用。

#### 关键 API

`FileStorage`/`Settings::read` 读取配置；`findChessboardCorners`、`findCirclesGrid` 检测板；`calibrateCameraRO` 与 `fisheye::calibrate` 分别求解；两套 `initUndistortRectifyMap` 生成映射。

#### 处理流程

读取并校验配置 → 从相机/视频/图像列表取帧 → 检测和精化角点 → 达到有效帧数后按模型标定 → 保存参数与逐视图误差 → 进入去畸变预览状态。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `Input_Delay` | 接受相邻标定帧的时间间隔 | 设备相关 | 大可增加姿态差异但采集慢；小易收重复姿态 |
| `Calibrate_FixAspectRatio` | 固定焦距宽高比 | 有可靠先验时 | 稳定解但错误比例会留下方向性残差 |
| `Calibrate_UseFisheyeModel` | 选择鱼眼模型 | 布尔 | 选错模型会使边缘残差系统性增大 |
| `Show_UndistortedImage` | 标定后预览 | 布尔 | 不影响求解，只影响验收闭环 |

#### 关联与对比

`calibration.cpp` 偏命令行通用器，本例偏配置化教程；两者共享角点和误差逻辑。本例输出可直接供 PnP 与双目流程使用。

#### 注意事项

配置中的板宽高是内角点数；输入分辨率变化后原 `K` 不可原样套用；标准与鱼眼畸变系数维数、标志和映射 API 均不同。

#### 应用场景

可复用标定工具、鱼眼相机、批量数据集标定和参数文件标准化。

---

## 7.2 双目标定、校正与深度

### 7.2.1 `stereo_calib.cpp` —— 双目外参与极线校正
> **源文件**：`samples/cpp/stereo_calib.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`完整流程`

#### 功能概述

从同步左右标定图估计两相机的相对旋转 `R`、平移 `T`、本质矩阵 `E` 和基础矩阵 `F`，再计算使对应极线水平或垂直对齐的校正变换。源码同时展示有标定的 Bouguet 路径和只依赖 `F` 的未标定 Hartley 路径。

#### 核心原理

**30 秒心智模型**：双目标定先回答“两台相机彼此怎么摆”，校正再虚拟旋转两台相机，使同一空间点落在同一图像行；它不创造对应关系，只把二维搜索降成一维。

以左相机为参考，右相机坐标满足 $\mathbf X_R=R\mathbf X_L+T$。平移方向与两条观测光线共面，因此：

$$
\hat{\mathbf x}_R^\top [T]_\times R\hat{\mathbf x}_L=0,\qquad
E=[T]_\times R,\qquad F=K_R^{-T}EK_L^{-1}.
$$

`stereoCalibrate` 在角点重投影约束下估计 `R,T`。`stereoRectify` 输出 `R1,R2,P1,P2,Q`：`R1/R2` 是原相机到校正相机的旋转，`P1/P2` 是校正后的投影矩阵，`Q` 把 $(u,v,d)$ 映射到三维。水平双目常见形式：

$$
P_1=\begin{bmatrix}f&0&c_{x1}'&0\\0&f&c_y'&0\\0&0&1&0\end{bmatrix},\quad
P_2=\begin{bmatrix}f&0&c_{x2}'&-fB\\0&f&c_y'&0\\0&0&1&0\end{bmatrix}.
$$

Hartley 路径的 `stereoRectifyUncalibrated` 仅求两幅图的射影变换 $H_1,H_2$ 令极线平行，适合匹配而不保证度量深度。源码中的水平绿线是最直接的校正检查：同一物体边缘应落在同一条线上。

#### 关键 API

- `initCameraMatrix2D`：用各组物点/像点和图像尺寸直接建立两路内参初值，供 `stereoCalibrate` 起步；
- `stereoCalibrate`：由共同物点和两组像点估相对位姿；
- `stereoRectify`：从标定结果生成度量校正参数和 `Q`；
- `findFundamentalMat`、`stereoRectifyUncalibrated`：未标定射影校正；
- `initUndistortRectifyMap`、`remap`：合并去畸变与校正。

#### 处理流程

同步检测左右板角点 → 用 `initCameraMatrix2D` 建立两路内参初值（或固定既有内参）→ `stereoCalibrate` 求 `R,T,E,F` → `stereoRectify` 求 `R1,R2,P1,P2,Q` → 为两图生成映射 → `remap` → 用水平/垂直参考线检查。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `CALIB_FIX_INTRINSIC` | 固定单目标定的内参 | 推荐稳定内参时启用 | 降低双目优化自由度；内参错误则外参会被迫补偿 |
| `CALIB_SAME_FOCAL_LENGTH` | 假设两相机焦距相同 | 同型号同步变焦镜头 | 约束更稳，但不同镜头时会引入偏差 |
| `alpha` | 校正后有效 ROI 与视野 | `0`、`1`、`-1` | 越大保留越多原图，也产生更多无效黑边 |
| `CALIB_ZERO_DISPARITY` | 对齐校正后主点 | 测深常启用 | 简化视差零点；关闭可扩大共同有效视野 |
| 未标定校正阈值 | 剔除偏离极线的匹配 | 像素级 | 过小丢真匹配，过大让外点污染变换 |

#### 关联与对比

有标定校正保留 `K`、基线和物理单位，可交给 `stereo_match.cpp` 测深；未标定校正只服务于行对齐。`epipolar_lines.cpp` 用 `F` 检查几何，`stereo_calib.cpp` 则进一步把它变成可用的重映射。

#### 注意事项

- 左右图必须同步且板点索引一致，运动场景的时间差会被误解释为外参；
- 双目外参改变后（碰撞、重新对焦、变焦）旧校正立即失效；
- 只看总重投影误差不够，应检查校正后垂直视差；
- 校正插值会引入边界无效区，匹配时应使用 `validPixROI`。

#### 应用场景

双目模组标定、立体测距、机器人避障、校正图生成与多目系统几何统一。

### 7.2.2 `stereo_match.cpp` —— StereoBM/SGBM 视差与三维反投影
> **源文件**：`samples/cpp/stereo_match.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`完整流程`

#### 功能概述

读取左右图及可选标定参数，校正后用 `StereoBM` 或 `StereoSGBM` 计算稠密视差，再借 `Q` 生成三维点。源码同时体现视差定点格式、可视化缩放和点云输出语义。

#### 核心原理

**30 秒心智模型**：校正后，一个左像素只需在右图同一行寻找最相似位置；横向位移就是视差，近点移动大、远点移动小。

水平平行双目中：

$$
d=u_L-u_R,\qquad Z=\frac{fB}{d},\qquad
X=\frac{(u-c_x)Z}{f},\quad Y=\frac{(v-c_y)Z}{f}.
$$

深度误差由视差误差传播：

$$
|\delta Z|\approx \frac{Z^2}{fB}|\delta d|.
$$

因此远处深度会平方级恶化；增大焦距或基线可改善深度分辨率，却会缩小共同视野并加剧遮挡。

BM 在局部窗口最小化匹配代价。SGBM 还沿多方向累积路径代价：

$$
E(D)=\sum_p C(p,D_p)+\sum_{q\in N_p}
\begin{cases}
0&D_p=D_q\\P_1&|D_p-D_q|=1\\P_2&|D_p-D_q|>1
\end{cases}.
$$

源码中 `compute` 输出 `CV_16S` 定点视差，真实像素视差是存储值除以 `StereoMatcher::DISP_SCALE`（16）。传给 `reprojectImageTo3D` 前必须恢复尺度。

#### 关键 API

- `StereoBM::create`、`StereoSGBM::create`：局部与半全局立体匹配；
- `stereoRectify`、`initUndistortRectifyMap`、`remap`：建立行对齐输入；
- `StereoMatcher::compute`：输出固定 4 位小数的视差；
- `reprojectImageTo3D`：用 `Q` 将每个视差像素转为 XYZ。

#### 处理流程

读左右图与标定 → 可选 `--scale` 缩放并同步缩放内参 → 去畸变/校正 → 配置 BM/SGBM → 计算定点视差 → 左右一致性与斑点过滤 → 转真实像素视差 → `--color` 时 `applyColorMap` 用 `COLORMAP_TURBO` 伪彩显示 → `reprojectImageTo3D`。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `scale` | 输入图与内参的等比缩放因子 | `1`（不缩放） | 小于 `1` 下采样（`INTER_AREA`）快但丢细节；大于 `1` 上采样（`INTER_CUBIC`）保细节但慢；缩放后必须把 `K` 同步乘上同一因子 |
| `minDisparity` | 搜索起始视差 | 由校正和最近/最远距离定 | 设错会整体漏掉真实视差范围 |
| `numDisparities` | 搜索宽度，须为 16 倍数 | 按 $fB/Z_{\min}$ | 增大覆盖近物体但耗时、误匹配机会增加 |
| `blockSize` | 支撑窗口奇数边长 | 3–11 | 大则平滑抗噪但吃掉细节；小则保边但弱纹理不稳 |
| `P1/P2` | 小/大视差跳变惩罚 | 常按通道数和窗口面积设置 | 增大更平滑；过大跨越真实深度边界 |
| `uniquenessRatio` | 最佳与次佳代价差要求 | 5–15 | 增大减少误配，也扩大空洞 |
| `disp12MaxDiff` | 左右一致性容差 | 1–2 px | 小更严；大保留更多遮挡附近错误 |
| `speckleWindowSize/range` | 小连通斑点过滤 | 50–200 / 1–2 px | 过强会删除真实小物体 |

#### 关联与对比

`StereoBM` 快但依赖纹理与窗口；`StereoSGBM` 通过路径平滑改善弱纹理。两者都依赖 `stereo_calib.cpp` 的校正质量；`triangulatePoints` 处理稀疏匹配，`reprojectImageTo3D` 处理稠密视差。

#### 注意事项

- 不除以 16 会使深度缩小 16 倍；
- 曝光差、反光、重复纹理和无纹理墙面会破坏亮度一致性；
- 遮挡区在另一视图没有对应点，算法只能判无效，不能“调参恢复”；
- `d≈0` 时深度发散，应掩蔽无效/负视差；
- `Q` 的单位由双目标定的 `squareSize` 和 `T` 决定；
- `COLORMAP_TURBO` 伪彩只用于可视化区分视差层次，不改写视差数值，点云输出仍取真实像素视差。

#### 应用场景

稠密深度、三维点云、体积测量、近距离避障和双目质量诊断。

---

## 7.3 两视图几何与三角化

### 7.3.1 `epipolar_lines.cpp` —— 基础矩阵与对极约束
> **源文件**：`samples/cpp/epipolar_lines.cpp` ｜ **所属模块**：`calib3d/features2d` ｜ **示例类型**：`完整流程`

#### 功能概述

用 SIFT 匹配两幅图，RANSAC 估计基础矩阵，并将每个内点在另一幅图中的对极线画出，以点到线距离检查两视图几何。

#### 核心原理

**30 秒心智模型**：左图一个像素对应相机空间中的一条射线；这条射线与两相机中心张成一个对极平面，它在右图的投影是一条线，所以正确右点不再能任意落在二维平面上。

像素齐次点满足：

$$
\mathbf x_2^\top F\mathbf x_1=0,\qquad
\ell_2=F\mathbf x_1,\quad \ell_1=F^\top\mathbf x_2,\quad \operatorname{rank}(F)=2.
$$

若 $\ell=(a,b,c)^\top$，点线距离为

$$
\operatorname{dist}(\mathbf x,\ell)=\frac{|au+bv+c|}{\sqrt{a^2+b^2}}.
$$

归一化八点法由每对点构造 $A\mathbf f=0$，SVD 取最小奇异向量后强制 `F` 秩为 2。RANSAC 反复从最小样本估模型，用 Sampson/几何误差近似判内点，防止少量错误匹配拖垮全局解。源码的 `findFundamentalMat(..., FM_RANSAC, 1.0, 0.99, 2000, mask)` 正对应此逻辑。

#### 关键 API

- `SIFT::create`、`FlannBasedMatcher::knnMatch`：产生候选匹配；
- Lowe ratio test：比较最近与次近描述子距离；
- `findFundamentalMat`：鲁棒估计 `F` 和内点掩码；
- `computeCorrespondEpilines`：从点和 `F` 生成对极线；源码也展示直接矩阵相乘。

#### 处理流程

提取特征 → KNN 匹配 → ratio 筛选 → `findFundamentalMat` RANSAC → 只保留内点 → `F*x`/`F.t()*x'` 画对极线 → 统计点线残差。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| Lowe ratio | 匹配歧义阈值 | 0.7–0.8 | 增大保留更多但外点增多；减小更纯但覆盖下降 |
| RANSAC 阈值 | 内点的像素误差 | 0.5–3 px | 大容忍噪声也吞外点；小则模型可能无足够内点 |
| `confidence` | 找到无外点样本的目标概率 | 0.99–0.999 | 增大迭代上限和耗时 |
| `maxIters` | 最大随机采样次数 | 数百到数千 | 太小在高外点率下容易失败 |

#### 关联与对比

`F` 作用于像素坐标，不需要内参；已知 `K` 时用 `E=K_2^\top F K_1` 进入度量相机坐标。单应 `H` 对平面或纯旋转可直接映射点，而 `F` 只给线约束。

#### 注意事项

- 点集中在小区域、近共线或场景几乎纯平面时 `F` 条件差；
- 纯旋转时没有有效基线，能估对极关系但不能稳定三角化；
- 对极线画得“差不多”不是定量验收，应看内点比例与 Sampson/点线误差分布；
- `F` 只有尺度意义，不能从元素绝对值直接读物理量。

#### 应用场景

匹配几何验证、未标定立体校正、SfM/视觉里程计初始化和相机运动诊断。

### 7.3.2 `essential_mat_reconstr.cpp` —— 本质矩阵、位姿恢复与三角化
> **源文件**：`samples/cpp/essential_mat_reconstr.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`完整流程`

#### 功能概述

在内参已知时由匹配估计本质矩阵，分解出相对旋转和平移方向，枚举四组位姿并通过正深度约束消歧，再三角化生成稀疏三维点。

#### 核心原理

**30 秒心智模型**：`E` 把像素尺度剥掉，只描述两台归一化相机之间的刚体几何；它能给出旋转和“朝哪个方向平移”，但单对图没有绝对长度尺。

$$
\hat{\mathbf x}_2^\top E\hat{\mathbf x}_1=0,\qquad
E=[t]_\times R.
$$

理想 `E` 的奇异值为 $(\sigma,\sigma,0)$。`decomposeEssentialMat` 给出 $R_1,R_2,t$，候选是 $(R_1,\pm t)$、$(R_2,\pm t)$。正确解必须让三角化点同时位于两相机前方，即两相机坐标中的深度均为正。

对投影矩阵 $P_1,P_2$ 和匹配 $(u_1,v_1),(u_2,v_2)$，DLT 三角化构造：

$$
A=\begin{bmatrix}
u_1P_1^{(3)}-P_1^{(1)}\\
v_1P_1^{(3)}-P_1^{(2)}\\
u_2P_2^{(3)}-P_2^{(1)}\\
v_2P_2^{(3)}-P_2^{(2)}
\end{bmatrix},\qquad A\tilde{\mathbf X}=0.
$$

SVD 最小奇异向量给出齐次 $\tilde{\mathbf X}$，再除以第四分量。两条射线近平行时，最小奇异值方向对噪声极敏感，因此小基线/远点会产生巨大深度不确定性。

#### 关键 API

- `findEssentialMat`：在 `K` 已知时用 RANSAC 五点法估 `E`；
- `decomposeEssentialMat`：输出两个旋转与一个单位平移方向；
- `recoverPose`：封装分解和正深度消歧；该源码显式展开过程；
- `triangulatePoints`：线性齐次三角化。

#### 处理流程

特征匹配 → `findEssentialMat` 和内点掩码 → `decomposeEssentialMat` → 构造四个 `P2` → 对每组 `triangulatePoints` → 齐次归一化并检查双相机正深度 → 选择有效点最多的位姿 → 检查重投影误差。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `cameraMatrix` | 像素到归一化相机坐标的尺度 | 来自标定 | 焦距/主点错误会直接偏置 `E` 与位姿 |
| RANSAC `threshold` | 像素域内点阈值 | 0.5–2 px | 大会引入误匹配，小会丢掉噪声较大的真点 |
| `prob` | RANSAC 置信度 | 0.99–0.999 | 越大越稳但最坏迭代更多 |
| 视差/夹角门限 | 是否接受三角化点 | 依任务 | 提高门限使深度更稳，但远点更少 |

#### 关联与对比

`findFundamentalMat` 不需要 `K`，只能恢复射影关系；`findEssentialMat` 利用 `K` 恢复欧氏旋转和平移方向。双目标定已知物理标定板尺度，可得到带单位的 `T`；单对自然图像的 `E` 只能得到尺度不定的 `t`。

#### 注意事项

- 正深度检查必须在两台相机中都做，仅检查第一相机会选错候选；
- 平面场景、纯旋转、极小基线是 `E`/三角化退化情形；
- 三角化前点、投影矩阵必须在同一坐标约定：都带 `K` 或都为归一化坐标；
- 输出平移通常归一化，不能直接称作“米”；
- 应在 RANSAC 内点上重投影验证，而非仅凭正深度数量。

#### 应用场景

两帧 SfM、视觉里程计初始化、相机相对姿态、稀疏重建与 SLAM 初始化。

---

## 7.4 位姿与平面几何

### 7.4.1 `select3dobj.cpp` —— PnP 位姿与平面反投影
> **源文件**：`samples/cpp/select3dobj.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`完整流程`

#### 功能概述

利用已知棋盘三维点和检测到的二维角点求棋盘相对相机的位姿，再把用户二维交互反投影到棋盘平面，构造三维盒并投影回图像辅助分割。

#### 核心原理

**30 秒心智模型**：标定回答“相机内部怎样成像”，PnP 回答“一个已知三维物体此刻在相机前怎样摆放”。它以 `K` 为已知量，只优化 `R,t`：

$$
\min_{R,t}\sum_i\|\mathbf x_i-\pi(K,d,R,t,\mathbf X_i)\|_2^2.
$$

`SOLVEPNP_ITERATIVE` 以闭式/平面初值开始，用 LM 精化重投影误差；EPnP 用四个虚拟控制点把复杂度降到 $O(n)$；P3P/AP3P 从最小点集产生多个候选，适合置于 RANSAC 中；平面方形标记更适合 IPPE/IPPE_SQUARE。

图像点反投影到已知棋盘平面，本质是相机中心发出的射线与平面相交。去畸变归一化射线 $\mathbf r_c=K^{-1}\mathbf x$，转到世界坐标后：

$$
\mathbf C_w=-R^\top t,\qquad
\mathbf r_w=R^\top\mathbf r_c,\qquad
\mathbf X(\lambda)=\mathbf C_w+\lambda\mathbf r_w,
$$

再由平面方程求 $\lambda$。源码 `image2plane` 以矩阵求逆实现同一关系。

#### 关键 API

- `solvePnP`：从 3D–2D 对应估 `rvec,tvec`（本例用普通版本，非 RANSAC）；
- `Rodrigues`：旋转向量与矩阵互转；
- `projectPoints`：验证位姿并投影三维盒；
- `initUndistortRectifyMap`、`remap`：把整帧去畸变后再 PnP，此后 `distCoeffs` 清零不再重复校正；
- `image2plane`：以矩阵求逆把鼠标像素反投影到棋盘平面，是"像素→射线→平面交"的自实现；
- `grabCut`：利用投影盒生成的先验做前景分割。

#### 处理流程

加载 `K/distCoeffs` → 检测棋盘角点 → `solvePnP` → `Rodrigues`/`projectPoints` 验证 → 鼠标点形成相机射线 → 与棋盘平面求交 → 构造三维盒 → 投影凸包并 `grabCut`。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `flags` | PnP 求解器 | `ITERATIVE/EPNP/P3P/AP3P/IPPE` | 影响最小点数、速度、平面退化与多解处理 |
| `useExtrinsicGuess` | 是否以给定位姿初始化 | 连续视频可启用 | 好初值加速稳定；坏初值可能落入局部极小 |
| RANSAC 重投影阈值 | PnP 内点像素误差 | 2–8 px | 大则容忍检测噪声也接受外点；小则易无解 |
| `iterationsCount` | 随机假设上限 | 100–1000 | 高外点率需更多迭代，代价是延迟 |
| 3D 点布局 | 位姿可观测性 | 尽量覆盖物体 | 点聚集或共线会使姿态不稳；共面需选适配方法 |

#### 关联与对比

`E` 从 2D–2D 对应恢复两相机相对位姿且尺度不定；PnP 从已知 3D–2D 对应恢复绝对尺度位姿。`projectPoints` 是两者共同的最终验收工具。

#### 注意事项

- `rvec,tvec` 表示物体/世界到相机，不是相机在世界中的位姿；相机中心是 $-R^\top t$；
- 输入点顺序错一位会得到“数值收敛但物理错误”的位姿；
- 平面 PnP 存在镜像/多解，需正深度、先验或时序连续性消歧；
- 去畸变图上做 PnP 时应传零畸变；原图则传真实畸变，不能重复校正。

#### 应用场景

AR 坐标轴、棋盘/标记定位、机器人抓取、三维盒交互、相机位姿跟踪。

### 7.4.2 `main_registration.cpp` —— 交互建立三维模型点与描述子
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/main_registration.cpp` ｜ **所属模块**：`calib3d/features2d` ｜ **示例类型**：`多文件工程`

#### 功能概述

注册阶段加载三维网格和参考图，接收用户选择的 2D–3D 对应，求初始位姿，再把参考图特征反投影到网格，生成检测阶段使用的模型。

#### 核心原理

**30 秒心智模型**：先用少量人工可靠对应确定相机—物体关系，再把大量二维特征沿相机射线落到网格表面，将“图像描述子”绑定到“物体三维点”。PnP 目标见 7.4.1；射线—三角形相交由 `ModelRegistration` 执行。

#### 关键 API

`solvePnP` 求初始位姿；`ORB/SIFT/AKAZE` 提取特征；`PnPProblem::backproject2DPoint` 反投影；`Model::save` 保存三维点和描述子。

#### 处理流程

加载图像与 PLY → 人工建立对应 → `solvePnP` → 提取参考特征 → 射线与网格求交 → 保留落在表面的三维点 → 保存模型。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| 人工对应数/分布 | 初始 PnP 约束 | 至少 4，覆盖物体 | 多且分散更稳；聚集或近共线会退化 |
| 特征类型 | 模型描述子 | ORB/SIFT/AKAZE | 影响尺度鲁棒性、速度和 matcher 类型 |
| 网格相交容差 | 接受射线命中 | 与模型尺度相关 | 大会绑定错误表面，小会漏掉边缘特征 |

#### 关联与对比

它产出 `main_detection.cpp` 消费的模型；与 `select3dobj.cpp` 的平面反投影相比，本例对任意三角网格做射线相交。

#### 注意事项

人工点索引必须与网格顶点一致；内参或模型单位错误会污染全部三维特征；自遮挡区域的特征不应错误绑定到背面。

#### 应用场景

无纹理 CAD/网格对象注册、AR 物体初始化和离线检测模型制作。

### 7.4.3 `main_detection.cpp` —— 特征匹配、PnP RANSAC 与位姿平滑
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/main_detection.cpp` ｜ **所属模块**：`calib3d/features2d/video` ｜ **示例类型**：`多文件工程`

#### 功能概述

加载注册模型，对每帧匹配场景描述子与模型描述子，以 3D–2D 对应调用 `solvePnPRansac`，再用 Kalman 滤波稳定姿态并投影网格。

#### 核心原理

**30 秒心智模型**：描述子回答“哪个像素像模型上的哪个点”，PnP RANSAC 判断“这些对应能否由同一个刚体位姿解释”，时序滤波再抑制逐帧噪声。重投影内点而非原始匹配数决定姿态可信度。

#### 关键 API

`RobustMatcher::fastRobustMatch` 产生对应；`solvePnPRansac` 估位姿和内点；`KalmanFilter::predict/correct` 平滑状态；`projectPoints` 投影模型。

#### 处理流程

加载三维模型描述子 → 每帧提特征并匹配 → 组装 3D–2D 对应 → PnP RANSAC → 按内点数决定是否校正滤波器 → 投影网格与坐标轴。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `iterationsCount` | PnP RANSAC 上限 | 源码量级数百 | 大抗高外点率但慢 |
| `reprojectionError` | 内点像素阈值 | 数像素 | 大易吞外点，小易无足够内点 |
| `minInliersKalman` | 接受测量的内点下限 | 源码约 30 | 大更稳但易只预测；小会把坏姿态送入滤波 |
| ratio 阈值 | 描述子歧义过滤 | 约 0.7 | 大召回高但外点多 |

#### 关联与对比

输入来自 `main_registration.cpp`；核心 PnP 由 `PnPProblem.cpp` 封装，匹配由 `RobustMatcher.cpp` 封装。

#### 注意事项

Kalman 平滑不能修复系统性错配；长时间丢失后应重初始化而非无限预测；欧拉角跨 $\pm\pi$ 时直接滤波可能跳变。

#### 应用场景

实时物体姿态、AR 覆盖、机械零件跟踪和机器人视觉定位。

### 7.4.4 `PnPProblem.cpp` —— PnP 求解与投影封装
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/PnPProblem.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`多文件工程`

#### 功能概述

封装相机内参、`solvePnP/solvePnPRansac`、旋转表示、投影矩阵更新以及三维点投影，为注册和检测主程序提供统一位姿接口。

#### 核心原理

**30 秒心智模型**：该类维护同一世界到相机约定，避免 `rvec/tvec`、`R` 和 $P=K[R|t]$ 在不同调用点各自解释。PnP 数学与求解器取舍见 7.4.1。

#### 关键 API

`solvePnP`、`solvePnPRansac` 求解；`Rodrigues` 转换旋转；`set_P_matrix` 组成投影矩阵；`backproject3DPoint` 做齐次投影。

#### 处理流程

保存 `K/distCoeffs` → 接收对应点 → 选择普通或 RANSAC PnP → 更新 `R,t,P` → 投影三维点供显示和误差检查。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| PnP `method` | 求解器类型 | ITERATIVE/EPNP/P3P | 影响速度、最小点数和多解 |
| RANSAC 阈值/置信度 | 内点与停止规则 | 像素级/约 0.99 | 更严减少坏点，也可能无解 |
| `useExtrinsicGuess` | 使用已有姿态初值 | 连续帧可用 | 好初值快，坏初值可能局部收敛 |

#### 关联与对比

`main_detection.cpp` 负责流程决策，本文件负责几何状态；`projectPoints` 可作为其手写投影的官方一致性对照。

#### 注意事项

相机到世界与世界到相机不可混用；更新 `R,t` 后必须同步更新 `P`；畸变图和已去畸变图的 `distCoeffs` 语义不同。

#### 应用场景

可复用 PnP 组件、AR 投影、位姿单元测试和重投影诊断。

### 7.4.5 `RobustMatcher.cpp` —— 比率检验与对称匹配
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/RobustMatcher.cpp` ｜ **所属模块**：`features2d` ｜ **示例类型**：`多文件工程`

#### 功能概述

封装特征检测、描述和 KNN 匹配，并以 Lowe 比率检验及双向对称检验清理候选，为 PnP 提供较纯的对应。

#### 核心原理

**30 秒心智模型**：最近邻距离小并不等于唯一；最佳距离相对次佳明显更小才说明描述子有区分度，双向互选再排除单向歧义。几何外点最终仍交给 PnP RANSAC。

#### 关键 API

`Feature2D::detectAndCompute` 生成关键点/描述子；`DescriptorMatcher::knnMatch` 取前两邻居；`ratioTest`、`symmetryTest` 清理匹配。

#### 处理流程

两侧提特征 → 双向 KNN(`k=2`) → ratio 过滤 → 交叉检查 → 输出匹配索引。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| ratio | 最佳/次佳距离上限 | 0.6–0.8 | 大保留更多歧义点，小更纯但少 |
| 特征数 | 最大关键点数量 | 图像复杂度相关 | 大提高覆盖也增加匹配成本 |
| matcher 范数 | Hamming 或 L2 | 随描述子类型 | 选错会使距离失去意义 |

#### 关联与对比

与 `epipolar_lines.cpp` 的特征前端同类，但本例下游是 3D–2D PnP，不是 2D–2D `F`。

#### 注意事项

二进制描述子必须用 Hamming 类距离；对称检验提高精度但降低召回；重复纹理仍需几何 RANSAC。

#### 应用场景

物体识别前端、图像配准、PnP/SfM 对应清洗。

### 7.4.6 `ModelRegistration.cpp` —— 像素射线与网格求交
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/ModelRegistration.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`多文件工程`

#### 功能概述

把参考图二维特征反投影成世界射线，与 `Mesh` 中三角面求交，生成带描述子的三维模型点。

#### 核心原理

**30 秒心智模型**：像素只定义一条射线，网格表面提供缺失的深度。将射线 $\mathbf C+\lambda\mathbf d$ 与每个候选三角形求最近正向交点，命中点就是特征的三维位置。

#### 关键 API

使用 `PnPProblem` 提供相机中心/射线；调用 `Mesh` 的三角数据；内部射线—三角形相交和模型点写入连接 2D 与 3D。

#### 处理流程

像素去畸变 → 由当前姿态形成世界射线 → 遍历/筛选三角面 → 求最近有效交点 → 保存 XYZ 与对应描述子。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| 近零容差 | 平行与退化判断 | 小正数 | 大会漏近掠射命中，小会放大数值噪声 |
| 最近交点规则 | 自遮挡处理 | 最小正 $\lambda$ | 不使用最近点会把特征绑到背面 |
| 模型尺度 | 射线和三角坐标单位 | 必须一致 | 不一致会使交点距离和阈值失真 |

#### 关联与对比

`main_registration.cpp` 驱动本组件；平面版本可退化为 `select3dobj.cpp` 的射线—平面相交。

#### 注意事项

网格法向、三角索引或坐标系错误会产生系统性背面命中；轮廓附近射线易受标注和内参误差影响。

#### 应用场景

网格纹理注册、2D 特征三维化、AR 模型锚定。

### 7.4.7 `Mesh.cpp` —— PLY 网格读取与三角拓扑
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/Mesh.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`多文件工程`

#### 功能概述

读取 PLY 顶点与面索引，向注册、投影和可视化代码提供三角网格数据。

#### 核心原理

**30 秒心智模型**：本文件不估计视觉几何，而是保存“顶点坐标 + 三角形索引”的表面离散化；索引拓扑决定射线相交和投影线框是否正确。

#### 关键 API

文件流解析 PLY；`Point3f`/索引容器保存顶点与三角面；访问器向 `ModelRegistration` 和显示逻辑提供数据。

#### 处理流程

解析头部 → 读取顶点数量与坐标 → 读取面及顶点索引 → 校验范围 → 提供三角列表。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| 网格密度 | 三角面数量 | 模型相关 | 高相交更精细但注册更慢；低会丢几何细节 |
| 坐标单位 | 顶点物理尺度 | 与标定/PnP 一致 | 比例错会使平移和投影尺度错误 |
| 面索引基准 | 顶点编号规则 | 依文件 | 解析错会越界或形成错误三角形 |

#### 关联与对比

`Model.cpp` 保存稀疏三维特征模型，本文件保存完整表面网格，两者用途不同。

#### 注意事项

示例解析器支持的是其预期 PLY 子集；非三角面、二进制 PLY 或额外属性需确认源码支持；索引越界必须拒绝。

#### 应用场景

物体表面注册、线框投影和射线相交的数据载体。

### 7.4.8 `Model.cpp` —— 三维特征模型序列化
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/Model.cpp` ｜ **所属模块**：`core/features2d` ｜ **示例类型**：`多文件工程`

#### 功能概述

管理注册得到的三维点、关键点和描述子，并通过 OpenCV 持久化格式供检测阶段加载。

#### 核心原理

**30 秒心智模型**：模型是一张跨帧查表：描述子用于找到对应，三维坐标用于 PnP。两组数据的行/索引必须一一对应。

#### 关键 API

`FileStorage`、`FileNode` 读写矩阵和点集；容器访问器保持三维点与描述子行的索引契约。

#### 处理流程

注册阶段追加三维点/描述子 → 校验数量与类型 → 写 YAML/XML → 检测阶段读回 → 向 matcher 和 PnP 提供同索引数据。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| 模型点数 | 可匹配三维特征数量 | 覆盖可见表面 | 多提高召回但增内存/匹配时间 |
| 描述子类型 | `CV_8U` 或 `CV_32F` 等 | 随特征算法 | 类型变化必须同步更换 matcher |
| 序列化精度 | 三维坐标保存精度 | 浮点 | 过低会增加重投影误差 |

#### 关联与对比

由 `main_registration.cpp` 写、`main_detection.cpp` 读；`Mesh.cpp` 是完整几何，不可拿描述子模型替代。

#### 注意事项

点数与描述子行数不等必须报错；模型文件中的特征类型必须与运行时 detector/matcher 一致；字段缺失不能默认为空成功。

#### 应用场景

离线对象模型缓存、可重复检测和注册结果交换。

### 7.4.9 `CsvReader.cpp` —— 位姿 CSV 输入适配
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/CsvReader.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`多文件工程`

#### 功能概述

读取教程工程使用的 CSV 数值记录，为离线位姿或评估数据提供输入。

#### 核心原理

**30 秒心智模型**：这是数据边界而非视觉算法；关键是列顺序、数值单位和旋转表示必须与消费者约定一致。

#### 关键 API

标准文件流和字符串解析；工程内数据结构接收时间、平移或旋转字段。

#### 处理流程

打开文件 → 逐行分列 → 校验列数/数值 → 转换为工程类型 → 向评估或回放逻辑返回。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| 分隔符 | CSV 列边界 | 逗号 | 与文件不符会整行解析失败 |
| 列顺序 | 位姿各分量语义 | 工程固定 | 错序会得到数值合法但物理错误的位姿 |
| 角度/长度单位 | 度或弧度、米或模型单位 | 必须明确 | 单位错会产生比例或旋转灾难 |

#### 关联与对比

与 `CsvWriter.cpp` 构成对称数据契约，不参与 PnP 求解。

#### 注意事项

应拒绝空行、非数值和列数错误；不要依赖地区小数格式；读取的旋转约定要与 `Utils.cpp` 一致。

#### 应用场景

位姿回放、离线评估和跨工具数据交换。

### 7.4.10 `CsvWriter.cpp` —— 位姿 CSV 输出适配
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/CsvWriter.cpp` ｜ **所属模块**：`calib3d` ｜ **示例类型**：`多文件工程`

#### 功能概述

把检测或评估得到的位姿数值按固定列顺序写入 CSV。

#### 核心原理

**30 秒心智模型**：输出格式本身就是接口；稳定列顺序、精度和单位比“能写出文本”更重要。

#### 关键 API

标准输出文件流、格式化精度和逐行写入。

#### 处理流程

创建输出 → 写入约定字段 → 每帧格式化位姿 → 检查流状态 → 正常关闭刷新。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| 浮点精度 | 小数有效位 | 足以保留亚像素影响 | 高文件更大；低会量化位姿 |
| 写入频率 | 每帧或抽样 | 任务相关 | 高保留动态但 IO 更多 |
| 列/单位 | 数据契约 | 与 Reader 一致 | 不一致会破坏回放 |

#### 关联与对比

与 `CsvReader.cpp` 必须互为逆语义；与 `Model.cpp` 的 YAML 模型持久化用途不同。

#### 注意事项

检查文件打开和写入失败；程序异常退出前未刷新可能丢尾部；时间戳与图像帧必须对应。

#### 应用场景

轨迹记录、算法对比和位姿可视化数据导出。

### 7.4.11 `Utils.cpp` —— 旋转表示与教程辅助函数
> **源文件**：`samples/cpp/tutorial_code/calib3d/real_time_pose_estimation/src/Utils.cpp` ｜ **所属模块**：`calib3d/core` ｜ **示例类型**：`多文件工程`

#### 功能概述

集中实现教程工程使用的旋转、矩阵和显示辅助转换，连接 PnP 输出、Kalman 状态与可视化。

#### 核心原理

**30 秒心智模型**：旋转向量、矩阵、欧拉角都描述同一姿态但各有奇异性和约定；辅助函数必须固定轴顺序、角度单位和世界/相机方向。

#### 关键 API

`Rodrigues` 在旋转向量和矩阵间转换；矩阵运算提取/构造欧拉角；辅助绘制和数值转换供主程序调用。

#### 处理流程

接收一种旋转表示 → 按固定轴序转换 → 必要时归一化/检查正交性 → 返回滤波或显示所需表示。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| 欧拉轴序 | XYZ/ZYX 等 | 由源码固定 | 变更会得到完全不同角度 |
| 角度单位 | 弧度或度 | 计算常用弧度 | 混用会放大约 57.3 倍 |
| 奇异阈值 | 万向节锁判断 | 小正数 | 大过早进入特殊分支，小会数值不稳 |

#### 关联与对比

`PnPProblem.cpp` 维护几何矩阵，本文件负责表示转换；Kalman 对欧拉角滤波的限制见 `main_detection.cpp`。

#### 注意事项

欧拉角不是全局无奇异参数；转换往返应以旋转矩阵误差而非角度逐项相等验收；左右手系不可混用。

#### 应用场景

姿态显示、状态滤波接口和几何调试。

---

## 7.5 平面单应分解

### 7.5.1 平面单应的运动分解（原理串联）

> 源码逐文件详解见[第 3 章 §3.2.6](./ch03_features.md#326-homographydecompose_homographycpp)。本节只从 `calib3d` 几何链路补充其物理含义，不重复登记同一个官方示例。

#### 功能概述

由同一平面在两个相机姿态下的单应矩阵和内参，分解出若干组相对旋转、平移方向与平面法向，并与已知姿态比较。

#### 核心原理

**30 秒心智模型**：平面上所有点的深度都受同一个平面方程约束，所以整个两视图映射可压进一个 `3×3` 矩阵；但这个矩阵把相机运动和平面法向混合在一起，分解天然有多解。

若第一相机坐标中的平面满足 $\mathbf n^\top\mathbf X=d$，两视图单应为：

$$
H=K_2\left(R+\frac{t\mathbf n^\top}{d}\right)K_1^{-1}.
$$

归一化单应 $\bar H=K_2^{-1}HK_1$ 可分解为候选 $(R_i,t_i,\mathbf n_i)$。只能恢复 $t/d$，所以未知平面距离时仍无绝对平移尺度。`decomposeHomographyMat` 返回的解通常成符号/镜像对，需要可见点正深度、平面朝向或先验位姿筛选。

#### 关键 API

- `findHomography`：由平面点对应估计 `H`；
- `decomposeHomographyMat`：由 `H,K` 输出候选 `R,t,n`；
- `filterHomographyDecompByVisibleRefpoints`：按可见参考点过滤物理解；
- `Rodrigues`、`projectPoints`：与真实/估计姿态比较。

#### 处理流程

检测平面角点 → `findHomography` → 由标定参数构造归一化单应 → `decomposeHomographyMat` → 逐候选检查 `det(R)=1`、正深度和平面法向 → 与 PnP/真实位姿对照。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `ransacReprojThreshold` | 单应内点阈值 | 1–5 px | 大会把非平面/错配吞入模型；小则内点不足 |
| `cameraMatrix` | 去除像素尺度 | 精确标定值 | 错误会使分解旋转不再正交、平移方向偏差 |
| 点覆盖范围 | 单应条件数 | 覆盖整个平面 | 点集中会放大噪声和外推误差 |
| 可见性过滤 | 候选物理解筛选 | 应启用 | 不筛选会把数学解误当真实相机运动 |

#### 关联与对比

单应适合平面或纯旋转；基础矩阵适合一般三维场景。PnP 已知平面点的物理坐标，可直接给带尺度位姿；单应分解只从两幅平面影像出发，平移与平面距离耦合。

#### 注意事项

- 非平面点、明显视差或滚动快门会让一个 `H` 无法解释全图；
- 纯旋转时平移项消失，平面法向不可辨；
- `H` 仅差尺度等价，不能直接解释单个矩阵元素；
- 分解返回“多个有效解”是问题本性，不是 API 故障。

#### 应用场景

平面 AR、文档/墙面跟踪、视觉伺服、相机运动初值与全景拼接几何诊断。

---

## 7.6 全景拼接

### 7.6.1 `stitching.cpp` —— `Stitcher` 高级流水线
> **源文件**：`samples/cpp/stitching.cpp` ｜ **所属模块**：`stitching` ｜ **示例类型**：`完整流程`

#### 功能概述

用 `Stitcher::create` 和 `stitch` 封装完成多图配准与融合，并以状态码报告图像不足、单应估计失败或相机参数优化失败。它适合默认参数能处理的旋转全景或扫描件。

#### 核心原理

**30 秒心智模型**：`Stitcher` 不是简单地把两张图做一次 `warpPerspective`，而是先建立“哪些图互相重叠”的匹配图，再联合优化所有相机，最后处理亮度、接缝和多尺度融合。

`PANORAMA` 假设相机近似绕光心旋转，使用透视/球面相机模型；`SCANS` 使用仿射模型，更适合平面扫描和弱透视。高级接口内部阶段与下一节 detailed 示例一致。

#### 关键 API

- `Stitcher::create(PANORAMA/SCANS)`：选择运动模型和默认组件；
- `Stitcher::stitch`：执行估计与合成；
- `Stitcher::Status`：区分数据不足、几何失败和优化失败。

#### 处理流程

读取多图 → 创建模式 → 特征与两两匹配 → 相机估计与全局优化 → 投影 → 曝光/接缝/融合 → 输出全景及状态。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `mode` | 相机旋转模型或扫描仿射模型 | `PANORAMA/SCANS` | 选错模型会让几何优化失败或局部拉伸 |
| `registrationResol` | 注册分辨率 | 约 0.6 MP | 大更稳更慢；小会丢小纹理 |
| `seamEstimationResol` | 接缝估计分辨率 | 约 0.1 MP | 大可改善细接缝，耗时和内存增加 |
| `panoConfidenceThresh` | 保留匹配图边的置信门限 | 数据相关 | 高会断图，低会让错误边污染全局 |

#### 关联与对比

高级接口适合先验证数据是否可拼；需要定位失败阶段、控制投影、BA、曝光或融合时应转到 `stitching_detailed.cpp`。

#### 注意事项

- 近景平移造成的视差不能被单个全局相机模型完全消除；
- 重叠太少、重复纹理、运动物体会破坏匹配图；
- 返回状态必须检查，空 `pano` 不是图像写出问题；
- `PANORAMA` 与 `SCANS` 是模型选择，不是质量档位。

#### 应用场景

旋转全景、文档扫描拼接、快速验证图像组可拼性和默认生产管线原型。

### 7.6.2 `stitching_detailed.cpp` —— 匹配图、BA、曝光补偿与多频段融合
> **源文件**：`samples/cpp/stitching_detailed.cpp` ｜ **所属模块**：`stitching/detail` ｜ **示例类型**：`完整流程`

#### 功能概述

逐阶段暴露完整拼接系统：多尺度特征、两两匹配、最大连通分量、相机估计、光束平差、波形校正、投影、曝光补偿、接缝搜索和融合。它是理解 `Stitcher` 失败原因和参数因果的主源码。

#### 核心原理

**30 秒心智模型**：拼接是一组相互耦合的全局估计问题。匹配决定图结构，BA 让所有重叠同时一致，投影决定几何外观，曝光补偿解决低频亮度差，接缝选择避开冲突，多频段融合隐藏不同尺度的边界。

1. **匹配图与相机初值**：每张图是节点，可靠两两匹配是边。`leaveBiggestComponent` 删除不属于最大连通分量的图，避免无约束相机进入优化。旋转全景中，匹配可近似满足

$$
\mathbf x_j\sim K_jR_jR_i^\top K_i^{-1}\mathbf x_i.
$$

近平面扫描则使用单应/仿射估计。

2. **光束平差（BA）**：不是逐对修补，而是联合优化所有相机参数，使所有匹配射线或重投影一致：

$$
\min_{\{\theta_i\}}\sum_{(i,j)}\sum_k
\rho\!\left(\left\|\mathbf x_{jk}-\pi(\theta_j,\Pi^{-1}(\theta_i,\mathbf x_{ik}))\right\|^2\right).
$$

`BundleAdjusterRay` 最小化射线方向误差，适合旋转全景；`BundleAdjusterReproj` 最小化像素重投影误差；`ba_refine_mask` 决定焦距、主点、宽高比等哪些量可动。错误匹配会被全局传播，因此 BA 前的数据清洗比“多迭代”重要。

3. **投影与波形校正**：`waveCorrect` 调整相机旋转，使全景地平线不呈波浪；`RotationWarper` 将各图投到球面、柱面或平面。球面适合宽视场旋转全景，平面适合窄视场，柱面在水平长条全景中折中。

4. **曝光补偿**：重叠区若有 $I_i\approx g_iI_j$，全局增益补偿求每图 `g_i`；块增益把图划分成网格，允许空间缓变：

$$
\min_{\{g_i\}}\sum_{(i,j)}\sum_{p\in\Omega_{ij}}
w_p(g_iI_i(p)-g_jI_j(p))^2+\lambda\sum_i(g_i-1)^2.
$$

它修低频亮度/色彩差，不修几何错位。

5. **接缝与融合**：GraphCut seam finder 在重叠区寻找颜色/梯度差小的路径。Feather 以距离边界的权重线性混合。MultiBand 将图分成拉普拉斯金字塔 $L_i^l$，掩码分成高斯金字塔 $G_i^l$：

$$
L_{\text{blend}}^l=
\frac{\sum_iG_i^lL_i^l}{\sum_iG_i^l+\varepsilon},
\qquad I_{\text{out}}=\operatorname{collapse}(\{L_{\text{blend}}^l\}).
$$

低频在宽区域缓慢过渡以消亮带，高频只在窄区域混合以保细节。融合能隐藏小残差，不能修复大视差或错误单应。

源码对照：`computeImageFeatures` → `BestOf2NearestMatcher` → `leaveBiggestComponent` → `HomographyBasedEstimator/AffineBasedEstimator` → `BundleAdjuster*` → `waveCorrect` → `RotationWarper` → `ExposureCompensator::feed/apply` → `SeamFinder::find` → `Blender::feed/blend`。

#### 关键 API

- `computeImageFeatures`、`BestOf2NearestMatcher`：特征和匹配图；
- `leaveBiggestComponent`：保留有共同约束的图像集合；
- `HomographyBasedEstimator`、`AffineBasedEstimator`：相机/变换初值；
- `BundleAdjusterRay/Reproj/AffinePartial`：全局参数精化；
- `waveCorrect`、`RotationWarper`：姿态整形与投影；
- `ExposureCompensator`、`GraphCutSeamFinder`：光度校正与接缝；
- `FeatherBlender`、`MultiBandBlender`：最终融合。

#### 处理流程

`work_megapix` 尺度提特征 → 两两匹配并建图 → 置信度筛边和最大连通分量 → 相机初值 → BA → 波形校正 → `seam_megapix` 尺度投影、曝光估计和接缝 → `compose_megapix` 尺度重新投影 → 应用补偿与接缝掩码 → blender 累积和重建。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
|---|---|---|---|
| `work_megapix` | 注册工作分辨率 | 0.3–1 MP | 大保留更多特征但慢；小会丢纹理和小重叠 |
| `match_conf` | 描述子匹配严格度 | 特征类型相关 | 高更纯但图可能断开；低会引入错误边 |
| `conf_thresh` | 图边/相机置信门限 | 约 1 | 高会删弱重叠图；低让错误匹配进入 BA |
| `ba` | BA 代价模型 | `ray/reproj/affine/no` | 应与相机运动模型一致，选错会不收敛或形变 |
| `ba_refine_mask` | 可精化的内参维度 | 五字符掩码 | 放开更多可降残差，也增大漂移和退化风险 |
| `warp` | 目标投影面 | `spherical/cylindrical/plane/...` | 决定直线形态、视场能力和边缘拉伸 |
| `expos_comp` | 曝光模型 | `gain/gain_blocks/channels...` | 模型复杂可修局部差异，但可能放大噪声/色偏 |
| `seam` | 接缝代价 | `gc_colorgrad` 等 | 梯度项更避开结构边，代价更高 |
| `blend` | 融合方法 | `multiband/feather/no` | multiband 质量高内存大；feather 快但易亮带 |
| `blend_strength` | 融合宽度的相对控制 | 约 5 | 大使过渡宽、可能重影；小使接缝明显 |

#### 关联与对比

`PANORAMA` 对应旋转/透视相机链，`SCANS` 对应仿射链。曝光补偿、接缝和融合分别解决低频光度、路径选择和跨尺度过渡，职责不同，不能互相替代。单应分解解释两图平面运动，BA 则把多图所有相机放进一个全局优化。

#### 注意事项

- 先查看匹配图：若拓扑错误，后续任何 BA/融合参数都救不了；
- `--save_graph <file>` 可把匹配图导出为 DOT 文本，节点标签 `Nm` 为匹配数、`Ni` 为内点数、`C` 为置信度，是定位断图、弱边和错误匹配的最快手段；
- 相机发生平移且场景有明显深度层次时会出现不可消除视差，应改变拍摄方式或做局部网格变形；
- 移动物体可能被接缝切开或形成鬼影；
- 三个 megapix 尺度语义不同，修改后必须正确缩放焦距和投影参数；
- `blend_strength` 不是固定 band 数，源码依据输出 ROI 计算实际融合宽度/频带数；
- 曝光补偿应在接缝/融合前估计并在合成尺度应用，顺序错误会留下亮带。

#### 应用场景

高质量全景、航拍/显微大图、壁画与文档数字化、拼接失败诊断和定制化多相机合成。

---

## 7.7 本章小结

本章的因果链可压缩为：

```text
针孔与畸变模型
  → 平面多姿态标定得到 K、d
  → 双目/两视图得到 R、T、E、F
  → 校正把对应搜索压到同一行
  → 视差或三角化恢复深度
  → PnP 用已知 3D 尺度恢复位姿
  → 单应描述平面/纯旋转映射
  → 拼接以匹配图和 BA 做全局几何，再处理光度、接缝与融合
```

验收时不要只看“输出图像像不像”：标定看逐视图重投影残差与覆盖，双目看垂直视差，`F/E` 看内点和几何残差，三角化看夹角/正深度/重投影，PnP 看点布局和位姿约定，拼接先看匹配图与 BA，再看曝光、接缝和融合。
