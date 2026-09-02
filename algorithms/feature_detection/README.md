# feature_detection —— 特征点检测与匹配对比

## 1. 功能概述

两件事合在一个 demo 里：

1. **检测对比**：在同一张图上跑 6 种检测器，可视化关键点分布 + 打印关键点数量；
2. **匹配验证**：用 ORB 描述符对两张视角变化图做匹配，经 Lowe 比例测试 + RANSAC 单应性
   剔除离群点，输出**内点率**并画出单应性四边形。

检测器全部来自 OpenCV **主库**（`features2d` / `imgproc`），**不依赖 opencv_contrib**，
所以 SIFT / SURF 不在本模块内。

## 2. 算法清单

### 2.1 检测器

| 检测器 | 类别 | 描述符 | 旋转不变 | 尺度不变 | 速度 | 特点 |
|--------|------|--------|---------|---------|------|------|
| **Harris** | 角点（二阶矩） | 无 | 否 | 否 | 快 | 经典，响应值极小需归一化；无描述符，只能定位 |
| **Shi-Tomasi** | 角点（`goodFeaturesToTrack`） | 无 | 否 | 否 | 快 | Harris 的改进版，实践中比 Harris 更稳定 |
| **FAST** | 角点（像素比较） | 无 | 否 | 否 | **最快** | 只做检测不做描述，适合实时跟踪前端 |
| **ORB** | 二进制（FAST+BRIEF 改良） | 256 bit 二进制 | **是** | 部分 | 快 | **综合首选**，免费 + 快 + 带方向 |
| **BRISK** | 二进制 | 512 bit 二进制 | 是 | **是** | 中 | 尺度空间用金字塔，尺度变化大时优于 ORB |
| **AKAZE** | 二进制（非线性尺度空间） | 486 bit 二进制 | 是 | **是** | 中 | 非线性扩散构造尺度空间，**边缘保持最好**，细节丰富图上优于 ORB |

### 2.2 匹配链路

```
ORB detectAndCompute(1000)
        ↓
BFMatcher(NORM_HAMMING).knnMatch(k=2)
        ↓
Lowe 比例测试：d1 < 0.75 * d2      ← 剔除歧义匹配
        ↓
findHomography(RANSAC, 3.0)        ← 剔除几何不一致的离群点
        ↓
内点率 = inliers / good
```

## 3. 运行

```powershell
cd build_algo_ALL\algorithms

.\algo_feature_detection.exe                                                   # 默认 graf1 / graf3
.\algo_feature_detection.exe ..\..\data\graf1.png ..\..\data\graf3.png         # 显式指定图对
.\algo_feature_detection.exe ..\..\data\box.png ..\..\data\box_in_scene.png    # 换一组
```

构建：

```powershell
.\build.ps1 -Target algorithms -Module feature_detection
```

## 4. 输入 / 输出

**输入**

| 参数 | 位置 | 默认 | 说明 |
|------|------|------|------|
| `argv[1]` | 图 1 | `../../data/graf1.png` | 参考图 |
| `argv[2]` | 图 2 | `../../data/graf3.png` | 待匹配图 |

- 两图尺寸不一致时自动把图 2 resize 到图 1 的尺寸（便于拼接可视化）。
- 图 1 读取失败 → 合成随机图；图 2 读取失败 → 直接复用图 1（此时匹配无意义，仅跑通流程）。

**输出**

| 文件 | 内容 |
|------|------|
| `out/algorithms/feature_detection_compare.png` | 8 格 3 列：img1 / img2 / Shi-Tomasi / Harris / FAST / ORB / BRISK / AKAZE，`DRAW_RICH_KEYPOINTS` 画出方向圆与尺度圆 |
| `out/algorithms/feature_matching.png` | 2 格：`inliers_only`（仅 RANSAC 内点）+ `all_good + H-quad`（全部 good 匹配 + 黄色单应性四边形） |

**stdout**

```
detector                  keypoints
Shi-Tomasi                     400
Harris                        1203
FAST                          1832
ORB(500)                       500
BRISK                         2741
AKAZE                         3315
[feature_detection] ORB match: good=184 inliers=171 ratio=92.9%
```

## 5. 参数表

| 参数 | 代码位置 | 默认 | 影响 |
|------|---------|------|------|
| Shi-Tomasi `maxCorners` | `goodFeaturesToTrack(g1, corners, 400, 0.01, 10)` | `400` | 上限，实际返回 ≤ 400 |
| Shi-Tomasi `qualityLevel` | 同上 | `0.01` | 保留最强响应的 1% 分位以上，↑ 更少更稳 |
| Shi-Tomasi `minDistance` | 同上 | `10` | 角点间最小欧氏距离，非极大抑制 |
| Harris `blockSize` / `ksize` / `k` | `cornerHarris(g1, harris, 2, 3, 0.04)` | `2 / 3 / 0.04` | `k` 是经验常数（0.04~0.06） |
| Harris 筛选阈值 | `harris.at<float>(y,x) > 0.01 * maxv` | `1%` 最大响应 | 阈值 ↑ → 点更少更显著 |
| FAST `threshold` / NMS | `FastFeatureDetector::create(20, true)` | `20` / 开 | 阈值 ↑ → 点少；NMS 必开否则成片 |
| ORB `nfeatures` | `ORB::create(500)`（检测对比） | `500` | 检测展示用 500，匹配环节用 `1000` |
| Lowe 比例 | `m[0].distance < 0.75 * m[1].distance` | `0.75` | **最关键的匹配参数**。↓ 更严格（匹配少但准），↑ 更宽松 |
| RANSAC 重投影阈值 | `findHomography(pts1, pts2, RANSAC, 3.0, inliers)` | `3.0` px | 允许的最大重投影误差，↑ 内点多但可能混入错配 |

> **匹配环节用的是 ORB::create(1000)**，与检测展示环节的 500 不同——匹配需要更多候选点
> 才能在 RANSAC 后剩下足够内点。

## 6. 依赖数据

| 数据 | 路径 | 说明 |
|------|------|------|
| 默认图对 | `data/graf1.png` + `data/graf3.png` | **`data/` 为只读依赖，禁止修改**。经典视角变化样本（Oxford graf 序列） |
| 备选图对 | `data/box.png` + `data/box_in_scene.png` | 目标在复杂场景中的定位，更难 |

## 7. 结果怎么读

### 7.1 检测器对比

看两件事：**点的数量**和**点的分布是否落在有意义的结构上**。

- **Harris 点数通常最多**（本实现用 1% 最大响应的硬阈值，在纹理区会成片）。它**没有描述符**，
  不能直接用于匹配，只适合做跟踪的特征点来源。
- **FAST 最快但无方向无尺度**，只在帧间运动很小时可用。
- **ORB/BRISK/AKAZE 的关键点圆圈带半径**——半径代表检测尺度。AKAZE 的尺度分布最丰富，
  在纹理图上通常点最多、质量最好。
- 判断好坏的直观标准：**点是否落在角点/纹理显著处，而不是均匀铺满整张图**。

### 7.2 匹配质量

| 指标 | 含义 | 健康区间 |
|------|------|---------|
| `good` | 通过 Lowe 比例测试的匹配数 | 几十到几百，取决于纹理 |
| `inliers` | RANSAC 判定符合单应性的匹配数 | — |
| `ratio` | **内点率 = inliers / good** | **> 60% 算好**，> 90% 非常好，< 30% 说明匹配基本失败 |

**内点率低的排查顺序**：

1. 两图是不是真的存在**单一平面单应关系**？非平面场景/剧烈 3D 旋转下，单应模型本身就错了，
   内点率低是模型问题不是算法问题——换 `findEssentialMat` / `recoverPose`。
2. Lowe 比例太松？把 `0.75` 降到 `0.6` 试试。
3. 图对纹理太少？FAST/ORB 在弱纹理区提不出稳定点。

**单应性四边形**：`all_good + H-quad` 里的**黄色四边形**是把图 1 的四个角用 H 投影到图 2
坐标系的结果。如果四边形形状合理（接近矩形、位置对得上目标），说明单应估计成功。
代码里对 `!std::isfinite` 做了保护——退化单应会跳过绘制而不是画出垃圾多边形。

## 8. 扩展 TODO

- [ ] 加 **FLANN + KMeans 树** 匹配器，与 BFMatcher 做性能对比（大规模特征库必备）
- [ ] 加 **SIFT / SURF**（需 contrib 版本 OpenCV，参见 `README.md §7 FAQ` 里 contrib 编译坑）
- [ ] 加 **BEBLID / VGG** 等学习型描述符，与 ORB 二进制描述符对比精度
- [ ] 加 **匹配耗时 + 检测耗时** 的性能表，形成「质量 vs 速度」二维选型图
- [ ] 加 **重复率（repeatability）** 指标：用已知 H 的图对，量化各检测器的几何稳定性
- [ ] 支持 **批量图对扫描**，一次跑完 `data/` 下所有同名图对并汇总内点率
