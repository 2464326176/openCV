# sharpen — 锐化算法对比

模拟"软对焦采集"(轻度高斯模糊 + 少量噪声)，以清晰原图为 GT，横向对比 5 类锐化算法。
对应相机 ISP 的 **Sharpen 环节**：核心矛盾是锐度提升 vs 噪声放大 / 光晕（halo）。

## 算法清单

| 方法 | 核心思想 | 光晕 | 噪声放大 |
|------|---------|:----:|:--------:|
| `Laplacian k={0.5,1.0}` | 加性拉普拉斯增强 | 中 | 高 |
| `USM sigma={1,2} × amount={0.8,1.5}` | 反锐化掩模（经典） | 明显 | 高 |
| `Guided r=8 eps=0.01 a={1,2}` | 引导滤波基底的边缘感知 USM | 轻微 | 中 |
| `MaskedUSM s=1.0 a=2.0` | 梯度掩码抑制平坦区增强 | 轻微 | 低 |
| `HighBoostBilateral a=1.5` | 双边基底 + 残差提升 | 轻微 | 低 |

## 指标

- **PSNR / SSIM vs GT**：过度锐化（光晕）会让两项同时下降 —— 用来找最优强度。
- **Tenengrad**：Sobel 梯度平方均值，锐度参考（GT 是自然上限）。
- **USM 参数扫描表**：sigma × amount 的 PSNR 网格，直接定位最优参数。

## 运行

```powershell
.\build.ps1 -Target algorithms -Module sharpen
cd build\algorithms
.\sharpen.exe                                   # 默认 lena.jpg, blur=1.2, noise=3
.\sharpen.exe ..\..\data\images\VCG5.jpg 1.5 5  # 自定义图 / 模糊 / 噪声
```

输出 `out/algorithms/sharpen_compare.png`（GT → 退化输入 → 各算法结果网格 + stdout 指标表）。

## 结果怎么读

1. 先看 USM 扫描表找 PSNR 峰值 —— 峰值左侧欠锐、右侧光晕。
2. `MaskedUSM` / `HighBoostBilateral` 在有噪声时 PSNR 通常更稳（平坦区不放大噪声）。
3. Tenengrad 高于 GT 太多 ⇒ 视觉上会出现描边感，需下调 amount。
