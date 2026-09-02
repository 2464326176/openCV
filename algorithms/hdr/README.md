# hdr — HDR & 曝光融合

## 算法链

```
   N 帧 (不同曝光)
         │
         ▼
┌──────────────────────────────────────────────────┐
   │ CalibrateDebevec         │ → 输入 8U + times →输出响应曲线 CRF  │
   └──────────────────────────────────────────────────┘
                        │
         ▼
┌──────────────────────────────────────────────────┐
   │ MergeDebevec (CRF, times)  │ → HDR float32 (linear radiance)      │
   └──────────────────────────────────────────────────┘
                        │
         ▼
┌──────────────────────────────────────────────────┐
   │ TonemapDrago (γ=1.0)      │ → LDR 8U                           │
   └──────────────────────────────────────────────────┘
                        ▼              PNG 输出

   另一条无 CRF / 无 times 的分支
┌──────────────────────────────────────────────────┐
   │ MergeMertens (exposure)   │ → fusion 8U (与上面对比              │
   └──────────────────────────────────────────────────┘
```

## 关键概念

### HDR (High Dynamic Range)

普通 8-bit 相机感光后非线性的"sRGB"编码丢失了真实亮度比例。要恢复线性辐亮度
(radiance)，需要 **相机响应曲线 (CRF)** `g`，对每帧做 `log radiance = g(I) − log Δt`，
然后多曝光加权融合：

```
L(p) = Σ_k w(I_k(p)) · (g(I_k(p)) − log Δt_k) / Σ_k w(I_k(p))
```

权重 `w(I)` 通常用三角权函数，中间灰度权重大，过曝/欠曝权重接近 0。

### Tone Mapping

HDR 通常 > 1.0，显示需要压缩回 [0,1]。Drago 算法基于人眼对数响应：

```
L_disp = (L_hdr / (1 + L_hdr))^(1/γ)
```

配合自适应对数 base，把暗部对比保住。

### Exposure Fusion (Mertens 2007)

跳过 CRF 和 times，直接对每帧计算"质量权重"（对比度、饱和度、良好曝光度），
然后做 **拉普拉斯金字塔加权融合**，输出已经是 LDR。在工程上特别实用：

- 不需要 RAW 与曝光元数据
- 对小幅配准残差不敏感
- 输出色彩饱和度往往比 MergeDebevec + ToneMap 更讨喜

## 数据来源

`data/nv21/ev/`（基准 + 两档减曝）：

| 文件 | ev | et | iso |
|------|----|----|-----|
| `..._159_00_..._ev_0_..._et_134354_..._base_1.NV21` | 0  | 134354 | 109 |
| `..._159_01_..._ev_-4_..._et_24428_..._base_0.NV21` | -4 | 24428 | 150 |
| `..._159_02_..._ev_-8_..._et_12214_..._base_0.NV21` | -8 | 12214 | 100 |

`data/nv21/hdr_*/`（同组基准连拍 + 输出参考）：

- 3 帧输入同 ev/-4/-8
- `*_output_*_merge_3.NV21` 是厂商 ISP 输出的 ground truth，可与我们的
  MergeDebevec + Drago / Mertens 输出做视觉对比。

## 运行

```powershell
cd out\algorithms
.\hdr.exe
# 指定自定义目录与 ground truth:
.\hdr.exe D:\...\data\nv21\ev D:\...\data\nv21\hdr_...\output_..._merge_3.NV21
```

输出 `out/algorithms/hdr_compare.png`，含：Input#0、Input#1、Debevec+Drago、
Mertens、ground truth `merge_3`。

## 扩展建议

- 加 Reinhard (`createTonemapReinhard`) / Mantiuk (`createTonemapMantiuk`) 对比；
- 在 `MergeRobertson` / `MergeMertens` 上做对比表；
- 加白平衡 (`xphoto::applyChannelGains`) 模块在 Mertens 之前；
- 用 PSNR/SSIM 评估 LDR 输出与厂商参考图的差距 (注意：raw ISP 后处理会带
  锐化 / 降噪 / gamma，差异天然较大，仅供参考)。

## 已知限制

OpenCV 4.13 的 `MergeDebevec::process` 在 BGR 3 通道输入下，对 response 做了
`reshape(1, 256)`，把 `256x1x3` 变成 `256x3x1`，导致断言
`log_response.cols == 1 && log_response.channels() == channels` 失败。
本 demo 现以 `Mertens` 曝光融合作为主输出，Debevec 链路在 try/catch 中
软失败，不影响整体运行。如需 Debevec 结果，可改为对 BGR 各通道分别调用
`MergeDebevec`（单通道 response 为 `256x1x1`），再 merge 回 BGR。
