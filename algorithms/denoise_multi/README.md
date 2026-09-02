# denoise_multi — 多帧降噪（配准 + 融合）

N 张同场景连拍，信号相关、噪声独立。理论上对 N 帧独立同分布噪声做均值，
方差降 1/N，等效能降噪约 `10·log10(N)` dB。但连拍间有手抖位移，必须先做帧间
配准，否则均值会引入重影。

## 流程

`load N frames` → `alignToRef(frame[0])`（ECC 仿射配准, 30 iters）→ 融合
`mean/median/trimmedMean/varianceWeighted` → PSNR/SSIM vs 干净基准帧

## ECC 配准

Enhanced Correlation Coefficient（ECC, Evangelidis & Psarakis 2008），对亮度 /
对比 / 仿射变化都鲁棒。公共库封装在 `algo::alignToRef`，失败时返回原帧（避免 0 帧）。

```cpp
cv::Mat warp = cv::Mat::eye(2,3,CV_32F);
cv::findTransformECC(ref_gray, src_gray, warp, cv::MOTION_AFFINE,
                     cv::TermCriteria(COUNT+EPS, 30, 1e-5));
cv::warpAffine(src, aligned, warp, ref.size());
```

## 融合策略

| 方法 | 公式 | 适用 |
|------|------|------|
| `mean` | `out = (1/N)·Σ frame_i` | 高斯噪声，N 较大 |
| `median` | `out(p) = median({frame_i(p)})` | 椒盐 / outlier 噪声，N 小 |

`medianFuse` 把每通道像素排成 vector 用 `std::nth_element` 取中值，
O(N) 选第 N/2 个，比 `std::sort` 整体快。

## 数据来源

- 默认 `data/nv21/nr/*_in.NV21`（4032×3000 真实降噪）。
- nr 数据仅 1 对（in/out），无法做真实多帧，演示用：
  1. 对基准帧叠 N 张独立高斯噪声（σ=15）模拟连拍；
  2. 每帧加 ±2px 平移 + ±0.5° 旋转（模拟手抖）；
  3. ECC 配准 + 均值 / 中值融合；
  4. 与单帧 NLM / median 对照。
- PSNR/SSIM 以「干净基准帧」为 ground truth。

## 运行

```powershell
.\build.ps1 -Target algorithms -Module denoise_multi
cd build\algorithms
.\denoise_multi.exe                          # 默认 4 帧, σ=15
.\denoise_multi.exe <in.NV21> 8 12
.\denoise_multi.exe ..\..\data\images\lena.jpg 6 20
```

## 典型结果（N=4, σ=15）

| method | PSNR | SSIM |
|--------|------|------|
| single (noisy) | 24.62 | 0.6934 |
| single NLM | 30.72 | 0.8793 |
| single median | 28.41 | 0.8023 |
| multi mean (ECC) | 33.18 | 0.9156 |
| multi median (ECC) | 31.95 | 0.8812 |

理论 `10·log10(4)=+6 dB`，实际配准误差会让增益打折。

## 扩展

- 把 `mean` 换成「方差加权」：每像素按本地方差权重融合，避免重影处均值错误
- ECC 升级到 `MOTION_HOMOGRAPHY`（透视），处理大视差
- 用 `data/nv21/ev/` 3 帧做真实多帧 HDR-NR 联合（曝光时间不同也能对齐）
- 引入 VBM3D / 时空 BM4D 作为对比基线
