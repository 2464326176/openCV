# common — 公共工具库

`demo_algorithms_common` 静态库为所有算法模块提供共用的基础设施。把 IO / 指标 / 配准 / 单帧降噪
集中放在一起，避免每个 demo 各写一份，同时让算法可被多模块复用（例如 NLM 同时被
`denoise_single` 和 `denoise_multi` 调用做对照）。

## 文件

| 文件 | 主要 API | 用途 |
|------|---------|------|
| `nv21_io.hpp/cpp` | `readNv21Auto / readNv21Y / loadNv21Dir / writeNv21` | NV21 (YUV420SP) 原始流读写；从文件名解析宽高与 et/ev；批量加载目录 |
| `algo_utils.hpp/cpp` | `psnr / ssim / hstackWithLabels / imshowFit / alignECC / alignToRef` | 评估指标、可视化、ECC 配准、目录工具 |
| `single_denoise.hpp/cpp` | `denoiseGaussian / Median / Bilateral / Guided / NLM / NLMGray` | 5 类常用单帧降噪，公共算法库 |

## NV21 布局与解析约定

NV21 内存布局：

```
Y  plane : width × height 字节
VU plane : width × (height/2) 字节  (V 与 U 交错, 每两个 Y 共享一对 VU)
总字节 = width × height × 3 / 2
```

公共库读取策略：

1. **从文件名解析宽高** — `parseNv21SizeFromName()` 用正则风格匹配
   `_NNNNxNNNN_` / `_NNNNXNNNN_`，宽高必须为偶数。
2. **校验字节数** — 实际工程数据有时会带尾部 metadata 或 stride padding，
   只要文件 ≥ `width*height*3/2` 就读，多余字节忽略。
3. **紧凑布局** — 默认 `stride_y == width`，一次性把整个 YUV420 缓冲读到
   `(h + h/2, w, CV_8U)` 单 Mat，调用 `cv::cvtColor(buf, bgr, COLOR_YUV2BGR_NV21)`
   一步转 BGR。
4. **stride padding 布局** — 若调用方传入 `stride_y > width`，Y/UV 两平面分别
   按行紧凑拷贝后再调用上述 cvtColor。

工程数据示例（已在 `data/nv21/` 实测）：

```
4032 × 3000   18,144,000 字节   = 4032*3000*3/2  紧凑布局 
3264 × 2448   11,985,408 字节   vs 11,983,104    末尾多 2,304 字节 → 按紧凑读，
                                                  多余字节忽略
```

## 评估指标

- **PSNR**：`MSE = mean( (a-b)^2 )`；`PSNR = 10·log10(255²/MSE)`。完全相同返回
  `1000 dB`，避免 `log(0)`。
- **SSIM**：单通道简化版，11×11 高斯窗口 (σ=1.5)，按 OpenCV photo module 思路
  计算 μ/σ/协方差；多通道取通道均值。

## 配准

`alignECC(ref, src, motionType=MOTION_AFFINE)`：

- 转 32F 灰度后调用 `cv::findTransformECC`，`TermCriteria(COUNT+EPS, 50, 1e-6)`。
- 支持 `MOTION_TRANSLATION` / `MOTION_AFFINE` / `MOTION_HOMOGRAPHY`。
- `alignToRef` 在 `alignECC` 成功后直接返回 warp 后的对齐图，失败时回退到原图。

## 可视化

`hstackWithLabels(imgs, labels, labelHeight=30)`：把多张同尺寸 BGR/灰度图
水平拼接，每张顶部带 30px 标题栏。不同尺寸的图需先 resize 对齐。

`imshowFit(win, img, maxEdge=1024, delay=0)`：长边超过 `maxEdge` 时按比例缩放
（INTER_AREA），避免大图直接 imshow 把屏幕占满。
