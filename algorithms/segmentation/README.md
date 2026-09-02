# segmentation —— 图像分割方法横向对比

## 1. 功能概述

在同一张图上跑完 **10 种分割方法**，横跨四个流派（阈值 / 颜色聚类 / 能量最小化 / 区域生长），
输出 3 列标签网格图 + 区域数统计。

分割是"把像素划分成有语义的组"，没有万能算法。本模块的价值在于把各流派摆在一起，
让你看清**每种方法的假设是什么、会在哪里失效**。

## 2. 算法清单

| 方法 | 流派 | 核心假设 | 输出形态 | 计算开销 |
|------|------|---------|---------|---------|
| **Otsu** | 全局阈值 | 直方图双峰，类间方差最大 | 二值 | 极低 |
| **Adaptive-mean** | 局部阈值 | 局部邻域均值可代表局部背景 | 二值 | 低 |
| **Adaptive-gauss** | 局部阈值 | 同上，邻域按高斯加权 | 二值 | 低 |
| **KMeans k=3/5/8** | 颜色聚类 | 颜色在特征空间成簇 | 量化图 | 中 |
| **MeanShift** | 密度聚类 | 颜色-空间联合空间中向密度峰收敛 | 平滑+分割 | **高** |
| **GrabCut** | 能量最小化 | 前景/背景各服从一个 GMM，用图割求最小割 | 二值前景 mask | 中 |
| **Watershed** | 区域生长 | 把梯度幅值当地形，从标记处"注水" | 彩色标签图 + 红色分水岭线 | 中 |
| **ConnectedComponents** | 连通性 | 前景像素的连通关系 | 伪彩标签图 | 极低 |

## 3. 运行

```powershell
cd build_algo_ALL\algorithms

.\algo_segmentation.exe                            # 默认 data/images/lena.jpg
.\algo_segmentation.exe ..\..\data\images\fruits.jpg
```

构建：

```powershell
.\build.ps1 -Target algorithms -Module segmentation
```

## 4. 输入 / 输出

**输入**

| 参数 | 位置 | 默认 | 说明 |
|------|------|------|------|
| `argv[1]` | 输入图片 | `../../data/images/lena.jpg` | 以 `IMREAD_COLOR` 读入 |

- 长边 > 720 px 自动缩放到 720（MeanShift 是 O(N·迭代) 级开销，大图极慢）。
- 读入失败时合成 400×400 随机图兜底。

**输出**

| 文件 | 内容 |
|------|------|
| `out/algorithms/segmentation_compare.png` | 11 格 3 列网格，从 input 到各方法结果 |

**stdout**

```
[segmentation] Otsu foreground CC count = 237
```

## 5. 参数表

| 方法 | 参数 | 代码默认值 | 调参方向 |
|------|------|-----------|---------|
| Otsu | 阈值 | 自动 | 无法调；直方图非双峰时直接失效 |
| Adaptive | `blockSize` | `51` | **必须奇数**。太小 → 把纹理当目标；太大 → 退化成全局阈值 |
| Adaptive | `C` | `3` | 从局部均值里减去的常数，调它来控制"多亮算前景" |
| KMeans | `k` | `3 / 5 / 8` | k↑ 细节更丰富但可能过分割 |
| KMeans | 终止条件 | `EPS+COUNT, 30, 0.6` | 迭代 30 次或中心位移 < 0.6 停 |
| KMeans | `attempts` / flags | `3` / `KMEANS_PP_CENTERS` | PP 初始化比随机稳，attempts↑ 更稳更慢 |
| MeanShift | `sp`（空间窗） | `20` | 空间半径，↑ 区域更合并 |
| MeanShift | `sr`（颜色窗） | `30` | 颜色半径，↑ 颜色被合并得越狠 |
| GrabCut | `rect` | 图像中央 `0.12~0.88` 区域 | 框要**完整包住目标**且留一圈背景 |
| GrabCut | `iterCount` | `5` | 迭代次数，5 通常已收敛 |
| Watershed | 距离变换阈值 | `0.4 * norm(dist, NORM_INF)` | 系数 ↑ → 前景标记更少更大块，↓ → 标记更碎 |
| CC | 邻域 | 8 邻域（`connectedComponentsWithStats`） | 4 邻域会更碎 |

## 6. 依赖数据

| 数据 | 路径 | 说明 |
|------|------|------|
| 默认测试图 | `data/images/lena.jpg` | **`data/` 为只读依赖，禁止修改** |
| 推荐对照 | `data/images/fruits.jpg`（颜色聚类效果显著） | 颜色成簇明显，KMeans/MeanShift 差异最大 |

## 7. 结果怎么读

**先认清一个事实**：分割没有金标准，本模块给的是**区域数**这类结构统计，不是精度分。

| 观察点 | 怎么看 |
|--------|--------|
| Otsu vs Adaptive | 光照不均时 Otsu 会大面积失败，Adaptive 能救回来——这是选型的分水岭 |
| KMeans k=3/5/8 | k 越大越接近原图。判断"够了"的标准是再增大 k 视觉上没新信息 |
| MeanShift | 输出是**平滑后的图**而非硬分割，常用于分割前的预处理去纹理 |
| GrabCut | 看边缘是否贴合目标轮廓。框没包全 → 缺一块；框贴太紧 → 吞背景 |
| Watershed | 红色线是分水岭边界。**红线密密麻麻 = 过分割**，说明前景标记太碎，需调大距离变换阈值系数或先做开运算 |
| CC | `CC4map(n=...)` 的 n 是连通域数。n 巨大说明二值图噪声多，应先形态学开运算 |

**选型速判**：

```
光照均匀 + 目标与背景灰度分明   → Otsu（最快）
光照不均 / 文档扫描             → Adaptive（高斯加权更好）
目标靠颜色区分、形状无所谓       → KMeans
需要抠出单个物体做后续处理       → GrabCut（要能给出包围框）
多个粘连物体要分开（如细胞计数） → Watershed（必须做好标记，否则过分割）
```

**经典陷阱：Watershed 过分割。** 分水岭对噪声极敏感，直接对梯度图做必然过分割。
本模块用「Otsu 反二值 → 距离变换 → 阈值出 sure-fg → 连通域做标记 → 未知区置 0」这套
标准流程来抑制。如果你的数据上红线还是太多，优先调 `0.4` 这个系数。

## 8. 扩展 TODO

- [ ] 加 **SLIC 超像素**（`ximgproc::createSuperpixelSLIC`，需 contrib）作为分割预处理
- [ ] 加 **语义分割 DNN**（换 ONNX 模型，见 `models/README.md` 的新增流程）
- [ ] 加 **分割定量评估**：准备 GT mask，算 IoU / Dice / Pixel Accuracy
- [ ] 把 GrabCut 升级为**交互式**（鼠标画前景/背景笔刷，见 `learn/L4_detect_calib/07_grabcut.cpp`）
- [ ] 加 **分水岭标记质量对比**（不做标记 / 距离变换标记 / 形态学重建标记 三种）
