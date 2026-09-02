# morphology —— 形态学处理全景对比

## 1. 功能概述

对一张图的 **Otsu 二值 mask** 跑完形态学全套操作，并附带三组对照实验：
**结构元素形状对比**（矩形/椭圆/十字）、**核大小对比**（5/9）、**hit-or-miss 角点检测**。
输出 4 列网格图 + 每格的前景像素占比与连通域数量统计。

形态学的本质是**用结构元素（SE）去探测图像的几何形状**，属于非线性、基于集合论的处理。
它不关心像素值大小，只关心"前景在哪"——所以输入必须是二值 mask。

## 2. 算法清单

| 操作 | OpenCV 枚举 | 集合定义 | 效果 |
|------|-----------|---------|------|
| **腐蚀** erode | `cv::erode` | `A ⊖ B` | 前景收缩，吃掉细小毛刺和孤立点 |
| **膨胀** dilate | `cv::dilate` | `A ⊕ B` | 前景扩张，填补小孔洞和断缝 |
| **开运算** open | `MORPH_OPEN` | `(A ⊖ B) ⊕ B` | **去白噪点**，先蚀后胀，主体形状基本不变 |
| **闭运算** close | `MORPH_CLOSE` | `(A ⊕ B) ⊖ B` | **填黑洞**，先胀后蚀，缝隙被焊上 |
| **形态学梯度** | `MORPH_GRADIENT` | `dilate − erode` | 提取物体**外轮廓**（一圈边） |
| **顶帽** tophat | `MORPH_TOPHAT` | `src − open` | 提取**亮于背景**的细小结构（如光照不均下的亮文字） |
| **黑帽** blackhat | `MORPH_BLACKHAT` | `close − src` | 提取**暗于背景**的细小结构（如暗划痕） |
| **hit-or-miss** | `MORPH_HITMISS` | fg 核命中前景 ∧ bg 核命中背景 | 模板匹配式形状定位（角点、端点） |

> **记忆口诀**：开运算"去白"（先蚀把小白点蚀没，再胀回来），闭运算"补黑"（先胀把黑洞胀没，再蚀回来）。

## 3. 运行

```powershell
cd build\algorithms

.\morphology.exe                                    # 默认 lena.jpg + Otsu 自动阈值
.\morphology.exe ..\..\data\images\VCG5.jpg 128     # 手动指定二值化阈值
```

构建：

```powershell
.\build.ps1 -Target algorithms -Module morphology
```

## 4. 输入 / 输出

**输入**

| 参数 | 位置 | 默认 | 说明 |
|------|------|------|------|
| `argv[1]` | 输入图片 | `../../data/images/lena.jpg` | 内部以 `IMREAD_GRAYSCALE` 读入 |
| `argv[2]` | 二值化阈值 | Otsu 自动 | 给了就用固定阈值，否则走 `THRESH_OTSU` |

- 长边 > 700 px 自动缩放到 700。
- 读入失败时合成随机图兜底。

**输出**

| 文件 | 内容 |
|------|------|
| `out/algorithms/morphology_compare.png` | 14 格，4 列网格：灰度原图 / mask / 基础 2 / 组合 5 / SE 形状 3 / 核大小 2 / hit-or-miss 2 |

**stdout 指标表**

```
[morphology] threshold = 117.0
panel                       fg_pixels components
binary(mask)                  45.12%         312
erode(3x3)                    40.03%         298
...
```

## 5. 参数表

| 参数 | 代码位置 | 默认 | 影响 |
|------|---------|------|------|
| 二值化方式 | `cv::threshold(..., THRESH_BINARY \| THRESH_OTSU)` | Otsu | 前景/背景的划分决定后面一切，mask 错了形态学救不回来 |
| 结构元素形状 | `getStructuringElement(MORPH_RECT/ELLIPSE/CROSS, ...)` | `MORPH_RECT` | 见下方对比 |
| 核大小 `ksize` | `morph(bin, op, 3)` | `3` | 核越大，腐蚀/膨胀越狠；**必须奇数** |
| 闭运算核 | `morph(bin, MORPH_CLOSE, {5,9})` | `5` / `9` | 填洞能力随核增大 |
| hit-or-miss 核 | `getStructuringElement(RECT, Size(2,2))` | `2×2` | 本 demo 用单核近似演示（严格版需 fg/bg 双核） |

**三种 SE 形状怎么选**

| 形状 | 特点 | 适用 |
|------|------|------|
| `MORPH_RECT` | 最"狠"，方形完整覆盖 | 去规则噪点、默认选择 |
| `MORPH_ELLIPSE` | 圆润，不产生直角伪影 | 处理圆形/不规则物体，视觉最自然 |
| `MORPH_CROSS` | 只沿水平垂直方向作用 | 保留细长结构、处理十字纹理 |

## 6. 依赖数据

| 数据 | 路径 | 说明 |
|------|------|------|
| 默认测试图 | `data/images/lena.jpg` | **`data/` 为只读依赖，禁止修改** |

> 想看形态学最戏剧性的效果，建议换成**文本/印章/裂缝**类图片——lena 的连续色调
> 二值化后 mask 比较碎，开闭运算的对比反而不如文档图明显。

## 7. 结果怎么读

三个统计量的联合判读法：

| 指标 | 定义 | 判读 |
|------|------|------|
| **fg_pixels** | 前景像素占比（`>= 8` 视为前景） | 腐蚀后应**下降**，膨胀后应**上升**，开闭后应**接近原值** |
| **components** | 8 邻域连通域个数 | 开运算**大幅减少**（孤立噪点被清除）；闭运算也会减少（缝隙被焊合，多个域连成一片） |
| 视觉 | — | 形态学梯度应看到一圈细轮廓；顶帽应只剩亮细节；黑帽应只剩暗细节 |

**典型趋势（拿 mask 当基线）**：

```
erode   → fg↓  components↑（大块被切碎，或小块消失）
dilate  → fg↑  components↓（邻近块粘连）
open    → fg≈  components↓↓（最关键：噪声被清掉而主体面积几乎不变）
close   → fg≈  components↓（孔洞填掉、断口接上）
gradient→ fg 很小（只剩一圈轮廓）
tophat  → fg 很小（只剩亮细节）
```

**排错经验**：

- 开运算后主体也消失了 → 核太大，或 mask 前景本身就很细。换 `MORPH_CROSS` 或减小 `ksize`。
- 闭运算填不上洞 → 洞比核还大。核大小必须 **大于** 待填补的缝隙宽度。
- 顶帽/黑帽一片黑 → 说明图像光照均匀，没有比背景局部更亮/更暗的结构，属正常。

## 8. 扩展 TODO

- [ ] 实现**严格版 hit-or-miss**（fg 核 + bg 核双核），做真正的角点/端点定位
- [ ] 加 **骨架提取**（`ximgproc::thinning`，需 contrib）与 **细化** 对照
- [ ] 加 **形态学重建**（测地重建，先标记再膨胀收敛）——做孔洞填充的工业标准做法
- [ ] 加 **顶帽做光照校正** 的实战场景（不均匀光照下的文档二值化）
- [ ] 加 **灰度形态学**（`MORPH_*` 直接作用于灰度图，不必先二值化）
