# demosaic — 去马赛克算法对比

从参考 RGB 图合成 **RGGB Bayer CFA** 原始帧（可选加散粒噪声），再跑 4 种去马赛克
算法与原图对比。对应相机 ISP 的 **Demosaic 环节**：核心矛盾是插值精度 vs
拉链伪影（zipper）/ 伪彩色（false color）。

## 算法清单

| 方法 | 核心思想 | 速度 | 典型表现 |
|------|---------|:----:|---------|
| `Bilinear` | cvtColor 双线性插值 | 极快 | 边缘有拉链伪影 |
| `Malvar(2004)` | Malvar-He-Cutler 5×5 线性滤波（本模块实现） | 快 | PSNR 通常最高 |
| `VNG` | 变梯度数插值 | 中 | 边缘保真好，细节区略糊 |
| `EdgeAware` | OpenCV 边缘感知插值 | 慢 | 伪彩最少 |

## 指标

- **PSNR / SSIM vs 原图**：整体插值精度。
- **MAE-B/G/R**：分通道误差 —— G 通道样本占一半，MAE 通常最小。
- **3× 放大细节条**：中心 180×180 裁块最近邻放大，直接看拉链/伪彩差异。

## 运行

```powershell
.\build.ps1 -Target algorithms -Module demosaic
cd build\algorithms
.\demosaic.exe                                  # 默认 VCG5.jpg, 无噪声
.\demosaic.exe ..\..\data\images\VCG5.jpg 8     # 加 sigma=8 噪声（RAW 更真实）
```

输出 `out/algorithms/demosaic_compare.png`（全图网格）与
`demosaic_detail.png`（细节放大条）+ stdout 指标表。

## 结果怎么读

1. Malvar 以近零成本稳定优于 Bilinear（线性滤波的梯度校正收益）；VNG/EA 在
   强边缘图上更强，但依图像内容而定。
2. 细节条里 Bilinear 的斜边会出现明暗相间的"拉链"，Malvar 明显改善。
3. 加噪声后再跑：线性方法（Bilinear/Malvar）会把噪声插值扩散，EA/VNG 相对稳。
