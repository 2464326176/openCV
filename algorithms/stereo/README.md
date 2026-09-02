# stereo —— 立体匹配（视差估计）对比

## 1. 功能概述

对一对**已校正的双目图像**跑 StereoBM 与 StereoSGBM 的多组参数配置，输出视差伪彩图（JET）
+ 无参考质量统计。

> **前提：输入必须是经过立体校正的图对**（同一行对应同一物理点，极线水平对齐）。
> 未校正的图对直接跑会得到一堆垃圾视差。校正流程见 `learn/L4_detect_calib/12_camera_calib.cpp`。

## 2. 算法清单

| 算法 | 全称 | 代价计算 | 聚合方式 | 特点 |
|------|------|---------|---------|------|
| **StereoBM** | Block Matching | SAD（绝对差和）窗口 | 固定窗口 | **最快**，弱纹理/倾斜面区域大面积失效，视差图有块状伪影 |
| **StereoSGBM** | Semi-Global Block Matching | SAD / Census | **8 方向扫描线动态规划** | 准得多，速度慢些，弱纹理区明显更好，边缘更锐 |

> 半全局（Semi-Global）的含义：不做全局 2D 优化（NP 难），而是沿 8 个/16 个一维方向
> 分别做动态规划再累加代价。**用一维近似二维，性价比极高**，是工业界事实标准。

## 3. 运行

```powershell
cd build_algo_ALL\algorithms

.\algo_stereo.exe                                              # 默认 aloeL / aloeR
.\algo_stereo.exe ..\..\data\aloeL.jpg ..\..\data\aloeR.jpg     # 显式指定
```

构建：

```powershell
.\build.ps1 -Target algorithms -Module stereo
```

## 4. 输入 / 输出

**输入**

| 参数 | 位置 | 默认 | 说明 |
|------|------|------|------|
| `argv[1]` | 左图 | `../../data/aloeL.jpg` | `IMREAD_GRAYSCALE` 读入 |
| `argv[2]` | 右图 | `../../data/aloeR.jpg` | 尺寸不同会自动 resize 到左图尺寸 |

- 读取失败时合成一对**已知水平视差**的圆盘图（中心偏移 10 px），保证流程可跑通，
  且能凭肉眼核验视差是否正确。

**输出**

| 文件 | 内容 |
|------|------|
| `out/algorithms/stereo_compare.png` | 8 格 2 列：left / right / BM bs=5,11,21 / SGBM bs=3,11，视差以 `COLORMAP_JET` 伪彩显示 |

**stdout**

```
method                   valid/mean_disp/smoothness
StereoBM(bs=5)   valid=88.2% mean_disp= 21.34 std= 8.91 smoothness=0.512
SGBM(bs=3)       valid=94.7% mean_disp= 22.10 std= 9.02 smoothness=0.318
```

## 5. 参数表

| 参数 | 位置 | 默认值 | 影响 |
|------|------|--------|------|
| `numDisparities` | `constexpr int numDisp = 64` | `64` | **必须能被 16 整除**。视差搜索范围，设小了远处测不到，设大了慢且噪声多 |
| `minDisparity` | `constexpr int minDisp = 0` | `0` | 最小视差，通常 0 |
| BM `blockSize` | `StereoBM::create(numDisp, bs)`，`bs ∈ {5,11,21}` | 扫 3 档 | **必须奇数**。↑ 更稳但边缘糊、细节丢 |
| BM `Disp12MaxDiff` | `bm->setDisp12MaxDiff(1)` | `1` | 左右一致性检查阈值，`-1` 表示关闭。开着能滤掉遮挡区误匹配 |
| SGBM `P1` | `8 * 3 * bs * bs` | 随 bs 变 | 相邻像素视差差 1 的平滑惩罚。**太小 → 噪声多；太大 → 过度平滑** |
| SGBM `P2` | `32 * 3 * bs * bs` | 随 bs 变 | 视差差 > 1 的惩罚，**必须 > P1**。控制边缘锐利度 |
| SGBM `disp12MaxDiff` | 构造参数第 7 个 = `1` | `1` | 左右一致性检查 |
| SGBM `preFilterCap` | 构造参数第 8 个 = `63` | `63` | 预处理截断值 |
| SGBM `uniquenessRatio` | 构造参数第 9 个 = `10` | `10` | 唯一性比率，↑ 剔除更多歧义匹配（视差图更干净但空洞更多） |
| SGBM `speckleWindowSize` | 第 10 个 = `100` | `100` | 散斑滤波窗口，0 = 关闭 |
| SGBM `speckleRange` | 第 11 个 = `32` | `32` | 散斑判定阈值，只与 `speckleWindowSize` 配合生效 |
| SGBM `mode` | `MODE_SGBM` | `MODE_SGBM` | 另有 `MODE_HH`（全 DP，更慢更准）、`MODE_SGBM_3WAY` |

**P1 / P2 调参口诀**（实践中最有效的两个旋钮）：

```
视差图麻点/噪声多        → P1 调大（如 8→16 倍系数）
视差图边缘糊成一片         → P2 调小，或减小 blockSize
物体边界出现"前景膨胀"     → P2 相对 P1 调小
```

## 6. 依赖数据

| 数据 | 路径 | 说明 |
|------|------|------|
| 默认图对 | `data/aloeL.jpg` + `data/aloeR.jpg` | **`data/` 为只读依赖，禁止修改**。OpenCV 经典已校正双目样本 |
| 备选 | `data/left01.jpg` + `data/right01.jpg` | 标定板双目序列（同名 `0x` 构成对母板序列） |

## 7. 结果怎么读

数据集通常**没有 GT 视差**，所以用三个无参考统计量：

| 指标 | 定义 | 判读 |
|------|------|------|
| **valid** | `disp > 0` 的像素占比 | **越高越好**，低说明大量区域匹配失败（遮挡/弱纹理/视差超范围） |
| **mean_disp** | 有效视差的均值（已除以 scale=16） | 与场景深度分布对应。近处物体多 → 均值大 |
| **std** | 有效视差标准差 | 反映场景深度层次。平坦墙面 → std 小 |
| **smoothness** | 视差图 Sobel 梯度幅值的均值 | **越小越平滑**。但**不是越小越好**——过小说明过度平滑，边缘被抹平 |

> 无效区域（`disp <= 0`）在伪彩图里被置为**中灰（128）**，一眼就能看出哪些区域没匹配上。

**伪彩图（JET）读法**：蓝→青→绿→黄→红 代表视差由小到大，
对应**距离由远到近**（视差与深度成反比：`depth = f * baseline / disparity`）。

**典型对比结论**：

- **BM 的 bs 越小**：细节越丰富但麻点越多；**bs 越大**：越平滑但块状伪影（"台阶"）越明显。
- **SGBM 的 valid 通常显著高于 BM**，尤其在弱纹理区——半全局聚合把邻域约束带进来了。
- **SGBM 的 bs=3** 是最锐利的，配合较大的 P1/P2 往往能得到最佳视觉结果。
- 如果 valid 普遍低于 70%，先怀疑 `numDisparities=64` 不够（拉近景时视差会超范围）。

**排错**：

- 视差图左右颠倒/全是灰 → 图对顺序反了（左图必须在 `argv[1]`）。
- 大面积灰（invalid）→ 图对**未做立体校正**，或 `numDisparities` 太小。
- 视差图有规则条纹 → 输入有周期性纹理，考虑 `MODE_HH` 或调整 `uniquenessRatio`。

## 8. 扩展 TODO

- [ ] 加 **左右一致性检查（LR check）后处理**，用 `ximgproc::getDisparityVis` 做遮挡填充（需 contrib）
- [ ] 加 **WLS 滤波**（`ximgproc::DisparityWLSFilter`，需 contrib）——能显著提升 SGBM 视觉效果
- [ ] 加 **GT 视差评估**：引入 Middlebury 数据集，算 bad pixel rate / RMS error
- [ ] 加 **视差转深度**：接入相机内参，输出 3D 点云（`reprojectImageTo3D`）
- [ ] 加 **CUDA 版本**（`cuda::StereoBM` / `StereoBeliefPropagation`）性能对照
- [ ] 加 **参数网格扫描**：自动搜 P1/P2/blockSize 组合，按 smoothness + valid 打分排序
