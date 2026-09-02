# 视频目标跟踪（videoTracking）

本节讲解视频场景下的三种经典运动目标技术：**稀疏光流**（Lucas-Kanade 金字塔）、**CamShift 自适应跟踪**、**背景减除**（MOG2）。对应官方示例 [optical_flow.cpp](../../mingw-build/samples/cpp/tutorial_code/video/optical_flow/optical_flow.cpp)、[optical_flow_dense.cpp](../../mingw-build/samples/cpp/tutorial_code/video/optical_flow/optical_flow_dense.cpp)、[camshift.cpp](../../mingw-build/samples/cpp/tutorial_code/video/meanshift/camshift.cpp)、[meanshift.cpp](../../mingw-build/samples/cpp/tutorial_code/video/meanshift/meanshift.cpp)、[bg_sub.cpp](../../mingw-build/samples/cpp/tutorial_code/video/bg_sub.cpp)。

本目录源码：[optical_flow.cpp](optical_flow.cpp)（LK 稀疏光流）、[optical_flow_dense.cpp](optical_flow_dense.cpp)（Farneback 稠密光流）、[meanshift.cpp](meanshift.cpp)（MeanShift）、[camshift.cpp](camshift.cpp)（CamShift）、[bg_subtraction.cpp](bg_subtraction.cpp)（MOG2 背景减除）。

## 1. 章节文件索引

| 文件 | 主题 |
|------|------|
| [optical_flow.cpp](optical_flow.cpp) | Lucas-Kanade 金字塔稀疏光流 |
| [optical_flow_dense.cpp](optical_flow_dense.cpp) | Farneback 稠密光流 + HSV 可视化 |
| [meanshift.cpp](meanshift.cpp) | MeanShift 直方图反投影跟踪（窗口固定） |
| [camshift.cpp](camshift.cpp) | CamShift 直方图反投影跟踪（窗口自适应） |
| [bg_subtraction.cpp](bg_subtraction.cpp) | MOG2 背景减除提取前景 |

## 2. 光流基础

**光流**是像素点在图像平面上的运动矢量场。亮度恒定假设：同一目标点的灰度在帧间不变：

$$
I(x, y, t) = I(x + dx, y + dy, t + dt)
$$

泰勒展开并忽略高阶项，得到**光流约束方程**：

$$
I_x u + I_y v + I_t = 0
$$

其中 $u, v$ 是待求的光流矢量，$I_x, I_y, I_t$ 是图像在空间和时间上的偏导。一个方程两个未知数，欠定，需要额外的平滑约束。

### 2.1 Lucas-Kanade（稀疏）

LK 假设**小邻域内光流恒定**，用窗口内多个像素的约束方程联立最小二乘求解：

$$
\begin{bmatrix}
I_{x1} & I_{y1} \\
I_{x2} & I_{y2} \\
\vdots & \vdots
\end{bmatrix}
\begin{bmatrix} u \\ v \end{bmatrix}
= -
\begin{bmatrix}
I_{t1} \\ I_{t2} \\ \vdots
\end{bmatrix}
$$

OpenCV 用 `calcOpticalFlowPyrLK` 实现**金字塔多尺度**版本：先在大尺度粗算、再逐层细化，可处理大位移。

### 2.2 Farneback（稠密）

`calcOpticalFlowFarneback` 对**每个像素**都求光流，输出稠密矢量场，适合光流可视化与运动分析，代价是计算量大。来自 [optical_flow_dense.cpp](optical_flow_dense.cpp)：

```cpp
calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
// flow: CV_32FC2，每像素 (dx, dy)

// HSV 编码可视化：H=方向角度，S=1，V=速度大小
cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
angle *= ((1.f / 360.f) * (180.f / 255.f));
// ... merge 成 HSV 后 cvtColor 转 BGR 显示
```

| 参数 | 含义 |
|------|------|
| `pyr_scale` | 金字塔缩放系数（0.5 即逐层减半） |
| `levels` | 金字塔层数（3 层可捕捉较大运动） |
| `winsize` | 平均窗口大小，越大光流越平滑 |
| `iterations` | 每层迭代次数 |
| `poly_n / poly_sigma` | 多项式展开邻域大小与高斯标准差 |

## 3. 稀疏光流代码解读

来自 [optical_flow.cpp](optical_flow.cpp)：

```cpp
// 首帧用 Shi-Tomasi 角点作为跟踪点
goodFeaturesToTrack(old_gray, p0, 100, 0.3, 7, Mat(), 7, false, 0.04);

// 逐帧：p0 是上一帧点，p1 是当前帧对应点
TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, 10, 0.03);
calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err,
                     Size(15, 15), 2, criteria);
```

| 参数 | 含义 |
|------|------|
| `winSize` | 搜索窗口大小（15×15），越大可跟踪越快运动但更模糊 |
| `maxLevel` | 金字塔层数，2~3 可处理大位移 |
| `status` | 每点跟踪成败标志（1=成功），失败点必须剔除 |
| `criteria` | 迭代终止条件（最大迭代 10 次或精度 0.03） |

**关键实践**：只跟踪 `status[i]` 为真的点；跟踪失败的点要丢弃，否则会累积错误。

## 4. CamShift 目标跟踪

### 4.1 从 MeanShift 到 CamShift

- **MeanShift**：把目标颜色直方图反投影回当前帧得到概率图，然后用均值漂移迭代寻找窗口质心，窗口**大小固定**。来自 [meanshift.cpp](meanshift.cpp)：

```cpp
calcBackProject(&hsv, 1, channels, roi_hist, dst, range);  // 概率图
meanShift(dst, track_window, term_crit);                    // 向质心迭代移动窗口
```

- **CamShift**（Continuously Adaptive MeanShift）：每次迭代后根据零阶矩**自适应调整窗口大小与方向**，输出带角度的旋转矩形，能跟随目标尺寸变化。

### 4.2 核心流程

```
首帧框选目标 → 计算目标 H-S 直方图（目标模型）
   ↓ 每帧
当前帧转 HSV → 提取 H 通道 → calcBackProject 反投影 → 概率图
   ↓
medianBlur 去噪 → CamShift 迭代 → 输出旋转矩形（位置/大小/方向）
```

代码来自 [camshift.cpp](camshift.cpp)：

```cpp
// 初始化目标模型：框选 ROI 的 H 通道直方图
calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &ranges);
normalize(hist, hist, 0, 255, NORM_MINMAX);

// 每帧跟踪：反投影 + CamShift 更新
calcBackProject(&hue, 1, 0, hist, backproj, &ranges);
medianBlur(backproj, backproj, 5);
trackBox = CamShift(backproj, trackBox,
                    TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1));
```

| 参数 | 含义 |
|------|------|
| `hsize` | 直方图 bin 数（16），越大模型越精细 |
| `inRange` 阈值 | 过滤低饱和度/低亮度像素，避免背景干扰 |
| `TermCriteria` | 迭代次数 10 + 收敛精度 1 |

## 5. 背景减除

背景减除适合**固定摄像头**下的运动目标提取。核心思想：用前 N 帧为每个像素建立**混合高斯模型（GMM）**，当前像素与背景模型差异超过阈值即判为前景。

```cpp
Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2(500, 16, true);
// 500: 历史帧数; 16: 方差阈值; true: 检测阴影
pMOG2->apply(frame, fgMask);   // 输出前景二值掩码
```

来自 [bg_subtraction.cpp](bg_subtraction.cpp)：

```cpp
morphologyEx(fgMask, fgMask, MORPH_OPEN, kernel);   // 开运算去孤立噪点
frame.copyTo(fgImg, fgMask);                        // 掩码抠出前景彩色图
```

| 参数 | 含义 | 调大/调小 |
|------|------|-----------|
| `history` | 背景建模用历史帧数 | 越大背景更新越慢 |
| `varThreshold` | 像素与模型差异阈值 | 越大前景越少（更保守） |
| `detectShadows` | 是否检测阴影 | 开则阴影归为灰色，便于消除阴影干扰 |

## 6. 三技术选型对比

| 技术 | 原理 | 适用场景 | 缺点 |
|------|------|----------|------|
| LK 光流 | 局部亮度恒定 + 邻域恒定 | 跟踪特征点/角点 | 只跟踪稀疏点 |
| Farneback | 全局稠密光流估计 | 运动分析、光流可视化 | 慢 |
| CamShift | 颜色直方图反投影 | 颜色特征明显的目标 | 颜色相近时易跟丢 |
| MOG2 | 逐像素混合高斯建模 | 固定相机前景提取 | 相机抖动即失效 |

## 7. 典型应用场景

- **智能监控**：MOG2 提取运动目标 → 轮廓分析 → 越界/逗留告警。
- **自动驾驶**：LK 光流估计 ego-motion，辅助目标跟踪。
- **体感交互**：CamShift 跟踪手部/头部区域。
- **视频压缩**：稠密光流估计帧间运动用于运动补偿。

## 8. 相关官方示例

- [optical_flow.cpp](../../mingw-build/samples/cpp/tutorial_code/video/optical_flow/optical_flow.cpp)：LK 稀疏光流官方演示
- [optical_flow_dense.cpp](../../mingw-build/samples/cpp/tutorial_code/video/optical_flow/optical_flow_dense.cpp)：Farneback 稠密光流
- [camshift.cpp](../../mingw-build/samples/cpp/tutorial_code/video/meanshift/camshift.cpp)：CamShift 官方演示
- [meanshift.cpp](../../mingw-build/samples/cpp/tutorial_code/video/meanshift/meanshift.cpp)：MeanShift 官方演示
- [bg_sub.cpp](../../mingw-build/samples/cpp/tutorial_code/video/bg_sub.cpp)：背景减除官方演示
- [bgfg_segm.cpp](../../mingw-build/samples/cpp/bgfg_segm.cpp)：KNN 与 MOG2 两种背景建模
- [kalman.cpp](../../mingw-build/samples/cpp/kalman.cpp)：卡尔曼滤波跟踪
