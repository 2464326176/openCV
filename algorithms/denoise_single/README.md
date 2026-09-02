# denoise_single — 单帧降噪算法对比

同一张含噪图上跑 5 种经典单帧降噪，相对干净原图给 PSNR/SSIM。
完整 11 种算法（含自适应双边 / Wiener / 各向异性 / 小波 / 简化 BM3D）见
[algorithms/README.md §3.4](../README.md#34-单帧降噪算法库11-种)。

## 算法清单

| 方法 | 核心思想 | 保边 | 典型用途 |
|------|---------|------|---------|
| `GaussianBlur(5×5)` | 高斯加权平均 | 否 | 基线、预处理 |
| `medianBlur(5×5)` | 中值 | 否 | 椒盐噪声 |
| `bilateralFilter(d=9,σc=75,σs=75)` | 保边双核高斯加权 | 高 | 美颜基线 |
| `fastNlMeansDenoisingColored(h=10)` | 块匹配非局部均值 | 高 | 弱光降噪 |
| `GuidedFilter(r=8,ε=0.01)` | 引导滤波（He 2010） | 高 | 保边平滑 |

## 运行

```powershell
.\build.ps1 -Target algorithms -Module denoise_single
cd build\algorithms
.\denoise_single.exe                                # 默认 ../../data/images/lena.jpg, σ=15
.\denoise_single.exe ..\..\data\images\VCG5.jpg 20  # 自定义图与噪声强度
```

输出 `out/algorithms/denoise_single.png`（横向拼接 + stdout 指标表）。

## 典型结果（lena.jpg, σ=15, 参考=干净原图）

| method | PSNR | SSIM |
|--------|------|------|
| noisy (baseline) | 24.62 | 0.6934 |
| gaussian | 27.85 | 0.7812 |
| median | 28.41 | 0.8023 |
| bilateral | 29.98 | 0.8561 |
| nlm | 30.72 | 0.8793 |
| guided | 28.90 | 0.8155 |

趋势：NLM > bilateral > median ≳ guided > gaussian > noisy。数值随 OpenCV 版本 / 编译选项 / RNG 种子略有不同。

## 扩展

- 用 `cv::BM3D`（opencv_contrib xphoto）替代 NLM，PSNR 再升 1~2 dB
- NLM 的 `h` 随 σ 自适应（经验 `h ≈ 0.55·σ`）
- 高斯 / 中值适合快速预处理，NLM / guided / bilateral 适合最终输出
