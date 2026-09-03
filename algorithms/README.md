# algorithms — 常用图像算法 Demo 集

> **20 个独立模块 + 1 个公共静态库**。每个模块是一个独立可执行 `<module>.exe`，
> 输出带标签的 PNG 对比图到 `out/algorithms/`，并把量化指标打到 stdout。
>
> 本子项目面向**算法开发实战**：吃项目里的真机数据（`data/nv21/`、`data/images/`），
> 做端到端的算法演示与横向对比。

**与 [learn/](../learn) 的分工**

| | learn | algorithms |
|---|---|---|
| 组织方式 | 按 OpenCV 教程章节，"知识点逐个击破" | 按**相机 ISP / 视觉算法**组织端到端 demo |
| 单文件规模 | 40~80 行，每题一个知识点 | 100~900 行，一次跑完一个算法族 |
| 目标 | 学会 API | 选对算法、调对参数 |
| 共享 | 都用 `common/opencv_utils.h` | 额外共享 `demo_algorithms_common` 静态库 |

---

## 1. 模块速览总表（20 个）

| # | 模块 | 算法数 | 主要输入 | 输出 PNG | 有无 GT 参考 |
|:-:|------|:------:|---------|---------|:-----------:|
| 1 | [`denoise_single`](denoise_single/README.md) | 11 种降噪 × 4 类噪声 | `data/images/` | `denoise_single.png` | ✅ 干净原图 |
| 2 | [`denoise_multi`](denoise_multi/README.md) | 3 种配准 × 4 种融合 | `data/nv21/nr/` | `denoise_multi.png` | ✅ 单帧 out |
| 3 | [`hdr`](hdr/README.md) | 2 CRF × 2 Merge × 7 Tonemap + Mertens | `data/nv21/ev/`、`hdr_*/` | `hdr_compare.png` | ✅ `merge_3.NV21` |
| 4 | [`night_scene`](night_scene/README.md) | 10+ 增强算法 | `data/nv21/ev/` ev=-8 | `night_scene.png` | ❌ 无参考 IQA |
| 5 | [`beauty`](beauty/README.md) | 频率分离磨皮 + 五官保留 | `data/images/` 人像 | `beauty.png` | ❌ 主观 |
| 6 | [`watermark`](watermark/README.md) | 可见 3 类 + DFT 非盲 + DCT 盲 + 30+ 攻击 | `data/images/` | 4 张总览图 | ✅ bit 恢复率 |
| 7 | [`edge_detection`](edge_detection/README.md) | 8 种边缘算子 | `data/images/lena.jpg` | `edge_detection_compare.png` | ❌ 无参考 |
| 8 | [`morphology`](morphology/README.md) | 8 种形态学 + SE/核对比 | `data/images/lena.jpg` | `morphology_compare.png` | ❌ 无参考 |
| 9 | [`segmentation`](segmentation/README.md) | 10 种分割方法 | `data/images/` | `segmentation_compare.png` | ❌ 无参考 |
| 10 | [`feature_detection`](feature_detection/README.md) | 6 检测器 + ORB 匹配 | `data/graf1/3.png` | 2 张（检测 + 匹配） | ✅ 单应内点率 |
| 11 | [`stereo`](stereo/README.md) | BM × 3 + SGBM × 2 | `data/aloeL/R.jpg` | `stereo_compare.png` | ❌ 无参考 |
| 12 | [`deblur`](deblur/README.md) | 逆滤波 / Wiener × 3 / RL / USM | `data/images/`（自合成模糊） | `deblur_compare.png` | ✅ 干净原图 |
| 13 | [`template_matching`](template_matching/README.md) | 6 种 TM + 多尺度 | `data/images/` + 模板 | `template_matching_compare.png` | ❌ 无参考 |
| 14 | [`inpaint`](inpaint/README.md) | Telea / NS × r=3/8 | `data/images/`（自合成损伤） | `inpaint_compare.png` | ✅ 干净原图 |
| 15 | [`hough_transform`](hough_transform/README.md) | 直线 + 圆检测 | `data/sudoku.png`、`smarties.png` | 2 张（线 + 圆） | ❌ 无参考 |
| 16 | [`frequency_domain`](frequency_domain/README.md) | 谱分析 / 低通 / 高通 / 陷波 | `data/images/`（自叠周期噪声） | `frequency_domain_compare.png` | ✅ 干净原图 |
| 17 | [`optical_flow`](optical_flow/README.md) | LK 稀疏 + Farneback 稠密 | `data/vtest.avi` | `optical_flow_compare.png` | ✅ 合成运动时 |
| 18 | [`sharpen`](sharpen/README.md) | 5 类锐化（USM/引导/掩码抑制）× 9 变体 | `data/images/`（自合成退化） | `sharpen_compare.png` | ✅ 清晰原图 |
| 19 | [`demosaic`](demosaic/README.md) | Bilinear/Malvar/VNG/EA | `data/images/`（自合成 CFA） | 2 张（全图 + 细节） | ✅ 原图 |
| 20 | [`color_transfer`](color_transfer/README.md) | LAB 均值方差 / RGB / 直方图 / 仅亮度 | `data/images/` 图对 | `color_transfer_compare.png` | ❌ 风格指标 |

> **有 GT 的模块**（✅）能给出 PSNR/SSIM 这类硬指标；**无 GT 的模块**用边缘密度、
> 内点率、平滑度等**无参考统计量**来横向比较。判读方法见各模块 README 的"结果怎么读"。

---

## 2. 目录结构

```
algorithms/
├── common/                      公共库 (静态库 demo_algorithms_common)
│   ├── nv21_io.{hpp,cpp}        NV21/NV12/I420 原始流读写、批量加载、元数据解析
│   │                            (WWWWxHHHH / et / iso / ev / base)
│   ├── algo_utils.{hpp,cpp}     PSNR/SSIM/MAE/MSE/MS-SSIM/LOE/NIQE + 色彩统计/增强/ECC配准/可视化
│   └── single_denoise.{hpp,cpp} 11 种单帧降噪算法 + 4 种噪声合成
├── hdr/                         ├─ hdr_pipeline.{hpp,cpp}  CRF 估计 → Merge → 7 种 Tonemap + Mertens
│                                └─ hdr.cpp                 多帧曝光序列 + 4 种配准 + 参数扫描
├── denoise_single/              单帧降噪 11 算法 × 4 类噪声 + 参数扫描
├── denoise_multi/               多帧降噪: ECC/仿射/单应性配准 + 4 种聚合 + 对齐残差热图
├── night_scene/                 夜景增强 10+ 算法 + 无参考 IQA (亮度/熵/LOE/边缘保留)
├── beauty/                      美颜: 三融合皮肤 mask + 频率分离磨皮 + 五官保留 + USM
├── watermark/                   水印: 可见 3 类 + DFT 非盲 + DCT QIM 盲提取 + 30+ 攻击
├── edge_detection/              边缘检测 8 算子 + 边缘密度/连续性指标
├── morphology/                  形态学全套 + 结构元素与核大小对比 + 连通域统计
├── segmentation/                分割: Otsu/自适应/KMeans/MeanShift/GrabCut/Watershed/CC
├── feature_detection/           特征检测 6 种 + ORB 匹配 + RANSAC 单应性验证
├── stereo/                      立体匹配 StereoBM/SGBM + JET 伪彩 + 视差统计
├── deblur/                      去模糊: 合成 PSF + 逆滤波/Wiener/RL/USM + PSNR 评估
├── template_matching/           模板匹配 6 种 TM 方法 + 多尺度定位
├── inpaint/                     图像修复: Telea/NS + 合成损伤 + mask 区 MAE
├── hough_transform/             霍夫变换: HoughLinesP 直线 + HoughCircles 圆
├── frequency_domain/            频域: DFT 谱 + 低通/高通/陷波去周期噪声
├── optical_flow/                光流: LK 稀疏 + Farneback 稠密 (HSV 速度场)
├── sharpen/                     锐化: USM/引导滤波/光晕抑制 + Tenengrad + 参数扫描
├── demosaic/                    去马赛克: 合成 RGGB CFA + Bilinear/Malvar/VNG/EA
├── color_transfer/              色彩迁移: Reinhard LAB/直方图匹配/仅亮度迁移
└── README.md                    本文件（构建统一由根 CMakeLists.txt 自动发现，无需子 CMakeLists）
```

---

## 3. 公共库接口总览（`demo_algorithms_common`）

### 3.1 评估指标（自动处理 BGR8UC3 / GRAY8UC1）

| 函数 | 含义 | 范围 | 越高越好? |
|------|------|------|:---------:|
| `psnr(a, b)` | 峰值信噪比 | [0, +∞) dB | ✅ |
| `ssim(a, b)` | 均值 SSIM（11×11 高斯核） | (0, 1] | ✅ |
| `msSsim(a, b)` | 多尺度 SSIM（5 尺度） | (0, 1] | ✅ |
| `mae(a, b)` | 平均绝对误差 | [0, 255] | ❌ |
| `mse(a, b)` | 均方误差 | [0, 255²] | ❌ |
| `loe(a, b)` | 光照顺序误差（Naturalness） | [0, W×H] | ❌ |
| `niqe(a)` | 无参考质量 NIQE（仅灰度） | 越小越好 | ❌ |
| `brightnessScore(img)` | 平均亮度 / 亮度标准差 | — | 诊断用 |
| `entropyScore(img)` | 灰度信息熵（bit） | 越大信息越丰富 | ✅ |
| `edgePreservationScore(orig, enh)` | 边缘保留度（梯度相关系数） | (0, 1] | ✅ |

### 3.2 色彩统计 & 增强工具

```cpp
// 色彩统计
cv::Scalar meanSaturation(const cv::Mat& bgr);                // 平均饱和度
cv::Scalar globalMeanStdBGR(const cv::Mat& bgr);              // BGR 各通道均值/标准差
ColorStats computeColorStats(const cv::Mat& bgr);             // 全部色彩统计汇总

// 图像增强
cv::Mat gammaCorrect(const cv::Mat& src, double gamma = 1.0); // Gamma 校正
cv::Mat applyCLAHE(const cv::Mat& src, double clip = 2.0,
                   int grid = 8);                             // LAB 上的 CLAHE
cv::Mat retinexSSR(const cv::Mat& src, int sigma);            // 单尺度 Retinex
cv::Mat retinexMSR(const cv::Mat& src,
                   const std::vector<int>& sigmas);           // 多尺度 Retinex
cv::Mat retinexMSRCR(const cv::Mat& src,
                     const std::vector<int>& sigmas,
                     double G, double b, double alpha,
                     double beta);                            // 带色彩恢复的 MSR
cv::Mat darkChannelDehaze(const cv::Mat& src,
                          int patchSize = 15, double omega = 0.95,
                          double t0 = 0.1, int guidedR = 40,
                          double guidedEps = 1e-3);            // 暗通道先验去雾
cv::Mat ACE(const cv::Mat& src, int C = 3, int k0 = 1);       // 自动色彩均衡
```

### 3.3 YUV420 IO（NV21 / NV12 / I420 自动识别）

```cpp
YuvFormat detectYuvFormat(const std::string& path, int w, int h);

cv::Mat readNv21Auto(const std::string& path);                // 文件名含 WWWWxHHHH 时自动解尺寸
cv::Mat readNv21(const std::string& path, int w, int h, YuvFormat fmt = FMT_NV21);
cv::Mat readI420(const std::string& path, int w, int h);

bool   parseNv21SizeFromName(const std::string& name, int& w, int& h);
double parseExposureTimeFromName(const std::string& name);    // us
int    parseIsoFromName(const std::string& name);
double parseEvValueFromName(const std::string& name);
int    parseBaseIndexFromName(const std::string& name);

std::vector<FrameMeta> loadNv21DirAsMeta(const std::string& dir);
```

### 3.4 单帧降噪算法库（11 种）

| 算法 | 说明 | 典型参数 |
|------|------|---------|
| `gaussianBlur(src, k, s)` | 高斯滤波 | k=3/5, s=1.0 |
| `medianBlur(src, k)` | 中值滤波 | k=3/5 |
| `bilateralFilter(src, d, sC, sS)` | 双边滤波 | d=9 |
| `nonLocalMeansDenoise(src, h, tW, sW)` | 非局部均值 NLM | h=3~10 |
| `guidedFilter(guide, src, r, eps)` | 引导滤波 | r=8 |
| `adaptiveBilateralFilter(src, k, sM, sC, sS)` | 自适应双边（结合局部方差） | k=5 |
| `wienerFilter(src, k, noiseVar)` | Wiener 滤波 | k=5 |
| `anisotropicDiffusion(src, iter, kappa, lambda)` | Perona-Malik 各向异性扩散 | iter=20 |
| `laplacianSoftThreshold(src, k, t)` | 拉普拉斯软阈值降噪 | t=3.0 |
| `waveletShrinkDenoise(src, level, t)` | 小波收缩（Haar 近似） | level=3 |
| `bm3dDenoise(src, sigma, tW, sW, beta)` | 简化 BM3D（grouping + 维纳协同滤波） | sigma=15 |

### 3.5 配准 / 可视化

```cpp
// 配准：ECC 梯度迭代 / Affine / Homography / Euclidean
cv::Mat alignToRef(const cv::Mat& ref, const cv::Mat& src,
                   int motion = cv::MOTION_EUCLIDEAN, int iter = 5000,
                   double eps = 1e-6);
cv::Mat alignByFeatures(const cv::Mat& ref, const cv::Mat& src,
                        const std::string& method = "homography");

// 可视化：带标签网格拼图 / 自适应尺寸 imshow / 目录创建 / 日志
cv::Mat gridWithLabels(const std::vector<cv::Mat>& imgs,
                       const std::vector<std::string>& labels,
                       int cols = 3, int labelHeight = 30);
cv::Mat hstackWithLabels(const std::vector<cv::Mat>& imgs,
                         const std::vector<std::string>& labels,
                         int labelHeight = 30);
void   imshowFit(const std::string& win, const cv::Mat& img,
                 int maxW = 1600, int maxH = 900);
```

---

> 各模块算法细节、参数表、典型结果见 §1 链接的模块 README；根目录
> [README.md §4 路径 C](../README.md#路径-c实战派--algorithms) 给出同样的 ISP / 通用两套分组速查。

## 5. 数据约定

复用项目根目录的 `data/`（**本子项目只读，绝不修改其内容**）：

| 数据 | 用途 | 被哪些模块读 |
|------|------|------------|
| `data/images/` | 通用 jpg/png 测试图（lena 等） | 1, 5, 6, 7, 8, 9, 12, 13, 14, 16, 17(兜底), 18, 19, 20 |
| `data/nv21/ev/` | 3 帧 ev=-8/-4/0 NV21 (4032×3000) | 3, 4 |
| `data/nv21/hdr_*/` | 3 帧 ev 输入 + GT `merge_3.NV21` | 3 |
| `data/nv21/nr/` | YNRCNR 单帧降噪 in/out 对 (3264×2448) | 2 |
| `data/graf1.png` / `graf3.png` | 视角变化图对 | 10 |
| `data/aloeL.jpg` / `aloeR.jpg` | 已校正双目图对 | 11 |
| `data/sudoku.png` / `smarties.png` | 直线 / 圆检测样本 | 15 |
| `data/vtest.avi` | 监控视频（光流） | 17 |
| `data/lena_tmpl.jpg` | lena 的子区域（模板） | 13 |

> **NV21 命名约定**：`..._WWWWxHHHH_..._et_XXX_iso_YYY_ev_Z_base_B.NV21`
> 公共库用 `parseNv21SizeFromName()` 自动解宽高，`parseExposureTimeFromName()` /
> `parseEvValueFromName()` 解 et/ev。**按此约定命名的新数据丢进 `data/nv21/` 即可，
> 不用改一行 C++ 代码。**

**无数据时的兜底策略**：所有模块在 `imread` 失败时都会**合成随机图/合成运动/合成双目对**
继续跑通，保证任何环境下 demo 都能执行。所以"跑不出图"一定另有原因，先查路径。

---

## 6. 编译 & 运行

需要根目录已配置好的 OpenCV 静态库（`mingw-build/x64/mingw/staticlib/`）。

### 6.1 推荐：用根目录的 build.ps1

```powershell
# 全量构建 20 个模块
.\build.ps1 -Target algorithms -Module ALL

# 只编 3 个
.\build.ps1 -Target algorithms -Module "hdr;denoise_single;beauty"

# Debug（断点调试）
.\build.ps1 -Target algorithms -Module ALL -Config Debug
```

### 6.2 等价的原生 CMake

>  **不要把构建目录设成 `out/`**。`out/` 是**算法输出目录**，且 `build.ps1 -Action clean -Mode out` 会清它。
> 用 `build/algorithms` 这类独立目录名。

```powershell
cmake -B build/algorithms -G "MinGW Makefiles" `
      -DBUILD_ALGORITHMS=ON `
      -DALGO_MODULE=ALL `
      -DBUILD_MAIN=OFF `
      -DBUILD_LEARN=OFF `
      -DCMAKE_BUILD_TYPE=Release
cmake --build build/algorithms -j
```

`ALGO_MODULE` 取值：`ALL`，或分号分隔的模块名（见 `CMakeLists.txt` 顶部注释）。

### 6.3 运行全部 20 个

可执行都落在 `build/algorithms/`，CWD 必须切到该目录（输入路径 `../../data/...`、输出 `../out/algorithms/` 相对它解析）。
完整清单与一键全跑脚本见 [根 README §3.1](../README.md#31-最快上手)。

### 6.4 常见运行问题

| 现象 | 原因 | 解决 |
|------|------|------|
| 找不到 `data` 图片 | CWD 不是 exe 所在目录 | `cd` 到 exe 目录再运行 |
| 输出目录报错 | `../out/algorithms` 不存在 | 代码里有 `ensureDir()`，正常会自动建；若失败手动 `mkdir` |
| `hdr.exe` 断言崩溃 | OpenCV 4.13 的 `CalibrateCRF` BGR reshape 已知 bug | 改用 **Mertens 曝光融合**路径（不需要 CRF） |
| 视频类模块无输出 | `data/vtest.avi` 打不开 | 会走合成运动兜底分支，看 stdout 的 log 提示 |

---

## 7. 后续可扩展方向

**短期（低风险增量）**

- [ ] `denoise_single`：接 `cv::BM3D`（contrib xphoto）替换简化版
- [ ] `deblur`：加噪声注入 + 盲去模糊（模糊核估计）
- [ ] `stereo`：加 WLS 滤波与左右一致性检查（contrib ximgproc）
- [ ] `segmentation`：加 SLIC 超像素与 IoU 定量评估
- [x] `common`：加 `ALGO_HEADLESS=1` 环境变量跳过 `imshow`（CI 友好）

**中期（向 DNN 迁移）**

- [ ] `night_scene`：加 EnlightenGAN / Zero-DCE 等端侧 DNN 方案做对照
- [ ] `beauty`：用 **YuNet landmark**（`models/face_detection_yunet_2023mar.onnx`）替代 Haar 级联做精准五官保留
- [ ] `denoise_single`：UNet / Restormer 风格 DNN 降噪作为传统算法的对照基线
- [ ] `watermark`：SSIM 感知约束的 alpha 自适应 + DNN-based HiDDeN 对抗水印

**长期（ISP 小引擎）**

- [ ] 串成端到端小 ISP：NV21 → 多帧降噪 → HDR Merge → 夜景增强 → 美颜 → 水印 → JPEG
- [ ] 加 `benchmarks/`：固定数据集 + 固定参数的 PSNR/SSIM/timing 回归表
- [ ] 接入 CI：每次 push 跑 benchmarks 并输出 diff

---

**相关文档**

- 根目录导航：[README.md](../README.md)
- 公共工具 API：[common/README.md](common/README.md)
- 模型清单：[models/README.md](../models/README.md)
- 输出目录说明：[out/README.md](../out/README.md)
