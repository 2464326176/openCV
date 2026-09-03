# color_transfer — 色彩迁移

让 source 图获得 reference 图的色彩风格。对应相机调优的**多摄/多场景色彩一致性**：
把一台机器的风格搬到另一台上，或统一不同光照下的出图风格。

## 算法清单

| 方法 | 核心思想 | 特点 |
|------|---------|------|
| `ReinhardLAB` | CIELAB 均值/方差匹配（Reinhard 2001） | 经典，整体风格迁移最均衡 |
| `MeanStdRGB` | BGR 逐通道均值/方差匹配 | 更激进，可能偏色 |
| `HistMatch` | 逐通道 CDF 直方图匹配（精确 1D 直方图对齐） | 全局色调最贴近参考 |
| `LumaOnlyLAB` | 仅 LAB 的 L 通道迁移，保留自身彩色 | 多摄调音常用：对齐亮度、保留本机色彩 |

所有方法支持 **strength ∈ [0,1]**（第 3 个命令行参数）与源图线性混合。

## 指标

- **maxDmean / maxDstd**：输出与 reference 的 BGR 逐通道 |均值| / |标准差| 最大差 ——
  越小越"像参考"。LumaOnlyLAB 的色度差大是设计使然（保留本机彩色）。
- **sat / entropy**：无参考色彩指标，检查迁移是否过度（饱和度爆炸/细节丢失）。

## 运行

```powershell
.\build.ps1 -Target algorithms -Module color_transfer
cd build\algorithms
.\color_transfer.exe                                  # 默认 VCG3 -> VCG5 风格
.\color_transfer.exe ..\..\data\images\4.jpg ..\..\data\images\lena.jpg 0.7
```

输出 `out/algorithms/color_transfer_compare.png`（source / reference / 4 方法 3×2 网格
+ stdout 指标表）。

## 结果怎么读

1. 要"整体氛围像参考" → ReinhardLAB；要"色调直方图像参考" → HistMatch。
2. 多摄一致性调优 → LumaOnlyLAB，只对齐亮度风格，避免本机色彩被带偏。
3. mean/std 匹配对 std 比值做了 [0.5, 2.0] 截断，防止参考图低对比时输出爆炸。
