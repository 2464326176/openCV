# OpenCV C++ 学习与实战项目（MinGW + CMake）

​	Windows + MinGW 工具链的 **OpenCV 4.13 学习 + 实战** 双形态仓库：从零基础逐题练习（118 个 .cpp）→ 官方 233 demo 中文原理详解→ **17 个端到端算法 Demo**（6 个 ISP 主流水线 + 11 个通用视觉算法）。

***

## 0. 项目亮点

| 特性                   | 说明                                                                                                 |
| -------------------- | -------------------------------------------------------------------------------------------------- |
| **静态链接零依赖**          | `mingw-build/x64/mingw/staticlib/` 自带预编译 OpenCV，exe 发布不需要一堆 DLL                                    |
| **四套学习体系**           | `learn/` 练习树 + `docs/` 原理详解 + `notes/` 毛星云笔记 + `algorithms/` 实战 Demo                               |
| **真机 NV21 数据**       | `data/nv21/` 含 4032×3000 曝光序列 / GT HDR 输出 / 降噪 in-out 对，直接喂给 ISP 算法                                |
| **17 个算法 Demo**      | 6 个 ISP 主流水线（单/多帧降噪、HDR、夜景、美颜、水印）+ 11 个通用视觉算法（边缘/形态/分割/特征/立体/去模糊/模板匹配/修复/霍夫/频域/光流） |
| **评估工具链**            | PSNR / SSIM / MS-SSIM / MAE / LOE / NIQE 近似 / EME / 熵 / 色彩饱和度 内置                                   |
| **2 个 ONNX 模型**      | YuNet 人脸检测 + SFace 人脸识别（OpenCV DNN / FaceDetectorYN API 原生调用）                                      |
| **一键 PowerShell 构建** | [build.ps1](build.ps1) / [clean.ps1](clean.ps1)，一行命令编所有子项目                                         |

### 0.1 项目定位速查（技术参数表）

| 维度         | 现状                                                  |
| ---------- | --------------------------------------------------- |
| 语言         | C++（OpenCV 4.13，MinGW 静态链接）                         |
| 工具链        | MinGW 14.2.0 (x86\_64-posix-seh-ucrt) + CMake 3.24+ |
| OpenCV 静态库 | `mingw-build/x64/mingw/staticlib/`                  |
| 测试数据       | `data/` (jpg/png/avi + `data/nv21/` 真机 NV21 raw)    |
| DNN 模型     | `models/` (YuNet 人脸检测、SFace 识别, ONNX)               |
| 项目类型       | **学习 + 实战** 双形态：理论笔记 + 可运行 demo + 端到端算法集            |

***

## 1. 环境与安装（Windows · MinGW64 + CMake + OpenCV 源码编译）

### 1.1 MinGW-w64 下载

**下载地址**：<https://sourceforge.net/projects/mingw-w64/files/>

在 `MinGW-W64 GCC-8.1.0` 列表下有：

| 变体                 | 说明                                         |
| ------------------ | ------------------------------------------ |
| x86\_64-posix-seh  | 64 位 + POSIX 线程 + 新 SEH 异常（性能好，但不支持 32 位）  |
| x86\_64-posix-sjlj | 64 位 + POSIX 线程 + 旧 SJLJ（稳定性好、**兼容 32 位**） |
| i686-win32-dwarf   | 32 位版本                                     |

👉 推荐：**`x86_64-posix-sjlj`** — 与 `mingw-build/` 中预编译产物工具链一致。

术语小抄：

- **POSIX** vs Win32：POSIX 线程模型让 `std::thread` / OpenMP 跨平台一致；Win32 线程仅 Windows 原生。

- **SEH** 是结构化异常处理，性能更高但仅 64 位；**SJLJ**（SetJump/LongJump）古老但稳定、32/64 通用。

### 1.2 CMake 安装

下载地址：<https://cmake.org/download/>，选 Windows x64 Installer 并勾选「Add CMake to PATH」。

### 1.3 OpenCV 源码 + 自编教程

- 源码：<https://github.com/opencv/opencv>（需要 SURF/SIFT/xfeatures2d 时必选 `opencv_contrib`）

- **MinGW-w64 + CMake GUI 自编图文教程**：<https://blog.huihut.com/2018/07/31/CompiledOpenCVWithMinGW64/>

自编产出就是本仓库的 `mingw-build/` 目录——直接复用即可，无需再编一次。

***

## 2. 仓库结构（6 套体系并存）

```
openCV/
├── 📖 learn/                      「分层逐题练习」L0~L5 共 6 层 100+ 独立 .cpp (每个有 main)
├── 📚 docs/                       「官方 233 demo 原理详解」8 章 + principles 骨架 + samples_flow
├── 📝 notes/                      「毛星云体例学习笔记」8 章 16+ 子目录（image_process 子项目可独立编译）
├── 🔬 algorithms/                 「算法实战 Demo 集」17 个模块 + common 静态库
├── 🛠️ common/                     跨子项目共享工具 (opencv_utils.{h,cpp})
├── 🗂️ data/                       测试数据（NV21 真机 + 通用 JPG/PNG）**只读不修改**
├── 🧠 models/                     ONNX 模型 (YuNet 检测 / SFace 识别)
├── 🏗️ mingw-build/                OpenCV 编译产物（头文件 + 静态库 + 官方 sample）**只读不修改**
├── 📤 out/                        algorithms 运行时输出 PNG 对比图（可清理）
│
├── 🔧 build.ps1                   PowerShell 一键构建 (main/learn/algorithms/notes/all)
├── 🧹 clean.ps1                   清理脚本 (build/cmake/out/all + DryRun)
├── CMakeLists.txt                 根 CMake (4 个 BUILD_* 开关 + LEARN_LAYER + ALGO_MODULE)
├── README.md                      本文件（项目总导航）
└── OUTLINE.md                     跳转说明（已合并入本文件，保留避免书签断链）
```

### 2.1 目录角色速查表（点击跳转对应 README）

| 目录                                  | 角色                      | 形态                        | 入口                                                             |
| ----------------------------------- | ----------------------- | ------------------------- | -------------------------------------------------------------- |
| [learn/](learn/README.md)           | **教程式逐题练习**             | L0\~L5 共 6 层，100+ 独立 .cpp | [README](learn/README.md)                                      |
| [docs/](docs/README.md)             | **原理 + 官方 233 demo 详解** | 8 章 + 原理篇 + 流程篇 markdown  | [README](docs/README.md) / [principles.md](docs/principles.md) |
| [notes/](notes/README.md)           | **毛星云体例学习笔记**           | 按主题 8 章 16+ 子目录           | [README](notes/README.md)                                      |
| [algorithms/](algorithms/README.md) | **算法实战 demo 集**         | 17 个模块（6 ISP + 11 通用）     | [README](algorithms/README.md)                                 |
| [common/](common/opencv_utils.h)    | 公共工具                    | 单文件 .h/.cpp               | [opencv\_utils.h](common/opencv_utils.h)                       |
| [models/](models/README.md)         | ONNX 模型                 | 只读（新增时写入 README 清单）       | [README](models/README.md)                                     |
| [out/](out/README.md)               | 算法 demo 输出 PNG          | build artifacts，可清理       | [README](out/README.md)                                        |
| `data/` / `mingw-build/`            | 只读依赖                    | **永不修改**                  | —                                                              |

### 2.2 四大子项目（怎么选？）

| 子项目               | 适合谁                                  | 入口                                                                          |
| ----------------- | ------------------------------------ | --------------------------------------------------------------------------- |
| **`learn/`**      | 要"边学边写"，40\~80 行最小可运行逐题练习            | [learn/README.md](learn/README.md)                                          |
| **`docs/`**       | 要"啃原理 + 调参"，233 官方 demo 逐章讲解 + 参数表   | [docs/README.md](docs/README.md) + [docs/principles.md](docs/principles.md) |
| **`notes/`**      | 习惯《毛星云·OpenCV3编程入门》章节组织方式（可独立构建 exe） | [notes/README.md](notes/README.md)                                          |
| **`algorithms/`** | 直接上生产级算法，跑 NV21 真机数据，看 PSNR/SSIM 打分（17 个模块）  | [algorithms/README.md](algorithms/README.md)                                |

### 2.3 数据资产详解（data/ 完整导航）

```
data/
├── images/         *.jpg / *.png  通用图（lena / VCG / baboon / fruits / messi5 …）
├── nv21/           真机 NV21 raw（ISP 算法主粮）
│   ├── ev/                 3 帧 ev=-8/-4/0 NV21 (4032×3000)   → HDR / 夜景
│   ├── hdr_<编号>/         同组曝光序列 + ground truth merge_3.NV21  → HDR GT 对比
│   └── nr/                 YNRCNR 单帧降噪 in/out (3264×2448)  → 降噪 in-out 对
├── dnn/            类标签文件 (coco/yolo/imagenet/…)  → DNN demo
├── aruco/          Charuco 标定数据                     → ArUco demo
└── *.jpg / *.png   OpenCV 官方示例原图
```

**NV21 文件命名约定**：`..._WWWWxHHHH_..._ev_Z_iso_YYY_et_XXX_base_B.NV21`

- 宽高自动从文件名解析（`algo::parseNv21SizeFromName`）；

- `et` / `EV` / `iso` / `base` 元数据均从文件名解析（`algo::parseExposureTimeFromName` 等）。

- 所以你只要把真实手机 dump 的 NV21 按这个约定命名丢进 `data/nv21/`，algorithms 的 ISP 类模块
  全部能自动读，不用改一行 C++ 代码。

***

## 3. 一分钟构建

环境前提：MinGW、CMake **已在 PATH**；根 `CMakeLists.txt` 已内建
`OpenCV_DIR=${CMAKE_SOURCE_DIR}/mingw-build/x64/mingw/staticlib`，**不需要**你自己设环境变量。

### 3.1 最快的「我想先看到东西」

```powershell
# 在项目根目录打开 PowerShell
.\build.ps1 -Target all        # 同时编 legacy main + 全部 learn + 全部 algorithms
```

构建完成后分别运行：

```powershell
# 1) algorithms：17 个算法 Demo，输出 PNG 到 out/algorithms/
#    需要先用 -Module ALL 构建（见 3.2）
cd build_algo_ALL\algorithms

# ISP 主流水线 6 个
.\algo_denoise_single.exe
.\algo_denoise_multi.exe
.\algo_hdr.exe
.\algo_night_scene.exe
.\algo_beauty.exe
.\algo_watermark.exe

# 通用视觉算法 11 个
.\algo_edge_detection.exe
.\algo_morphology.exe
.\algo_segmentation.exe
.\algo_feature_detection.exe
.\algo_stereo.exe
.\algo_deblur.exe
.\algo_template_matching.exe
.\algo_inpaint.exe
.\algo_hough_transform.exe
.\algo_frequency_domain.exe
.\algo_optical_flow.exe

# 或者一把全跑
Get-ChildItem .\algo_*.exe | ForEach-Object { Write-Host "=== $($_.Name) ===" -ForegroundColor Cyan; & $_.FullName }

# 2) learn：先跑 L0 入门几个
cd ..\..\build_learn_ALL\learn
.\learn_07_opencv_version.exe
.\learn_01_hello_imread.exe

# 3) notes：毛星云体例 legacy 练习
.\build.ps1 -Target notes
cd build_notes\notes\image_process\note_imgproc\
.\note_imgproc_image_read_image.exe
```

### 3.2 脚本的典型用法（[build.ps1](build.ps1) 全参数）

```powershell
# 构建 Target: main(默认) / learn / algorithms / notes / all
.\build.ps1 -Target learn -Layer L2                      # learn 的 L2 层
.\build.ps1 -Target algorithms -Module "hdr;denoise_single;beauty"   # 只编 3 个模块
.\build.ps1 -Target notes                                # 单独编 notes 子项目
.\build.ps1 -Config Debug                                # 编 Debug（断点调试）
.\build.ps1 -Target all -NoBuild                         # 只生成 Makefile 不编译
```

### 3.3 等价的原生 CMake 命令（不想用脚本）

```powershell
# A. legacy openCv exe
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# B. learn 单层练习（LEARN_LAYER ∈ L0..L5 / ALL）
cmake -B build_learn -G "MinGW Makefiles" -DBUILD_LEARN=ON -DLEARN_LAYER=L2 -DBUILD_MAIN=OFF
cmake --build build_learn -j

# C. algorithms（ALGO_MODULE = ALL / 分号分隔列表）
cmake -B build_algo -G "MinGW Makefiles" -DBUILD_ALGORITHMS=ON -DALGO_MODULE=ALL -DBUILD_MAIN=OFF
cmake --build build_algo -j

# D. notes/image_process legacy（notes/image_process 可 standalone 子构建）
cmake -B build_notes -G "MinGW Makefiles" -DBUILD_NOTES=ON -DBUILD_MAIN=OFF
cmake --build build_notes -j
```

### 3.4 编译开关矩阵（4 个 BUILD\_ 开关 + 2 个模块选择器）

| CMake 选项           | 默认                   | 含义                                                 |
| ------------------ | -------------------- | -------------------------------------------------- |
| `BUILD_MAIN`       | `ON`                 | 构建 legacy `openCv` 可执行（VideoCapture + drawText 示例） |
| `BUILD_LEARN`      | `OFF`                | 启用 learn 子项目                                       |
| `LEARN_LAYER`      | `ALL`                | learn 编译哪一层：`ALL` / `L0`\~`L5`                     |
| `BUILD_ALGORITHMS` | `OFF`                | 启用 algorithms 子项目                                  |
| `ALGO_MODULE`      | `hdr;denoise_single` | algorithms 编译哪些模块：`ALL` 或分号分隔列表                    |
| `BUILD_NOTES`      | `OFF`                | 启用 notes/image\_process legacy 子项目（**本项目新增开关**）    |

### 3.5 清理

```powershell
.\clean.ps1 build        # 清 build*/ / CMake 缓存（默认模式）
.\clean.ps1 cmake        # 只清散落在各子目录的 CMake 临时文件（保留 build 文件夹）
.\clean.ps1 out          # 清 out/algorithms/*.png（保留 out/README.md）
.\clean.ps1 all          # 上述三样一次性清干净
.\clean.ps1 all -DryRun  # 先 DryRun 打印会删什么，不落盘（推荐第一次先跑这个）
```

> 🔒 **硬保证**：clean.ps1 内含保护逻辑，`data/` 与 `mingw-build/` 无论什么参数
> 都**绝不会被触碰**。

***

## 4. 四条学习路径（按你的目标选其一）

每条路径都给两个版本：**速览表（5 秒定位）** + **详解（对应子目录/文件）**。

### 路径 A：零基础 1 周入门 → `learn/`（逐题练习，每题独立 main）

| 阶段                   | 主干练习（80% 必做，按编号对应 `learn/L<X>/NN_xxx.cpp`）                                                                                                                                           |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Day 1 入门 8 题         | L0: `07_opencv_version` → `01_hello_imread` → `04_drawing_primitives` → `02_named_window_trackbar` → `03_mouse_roi` → `06_split_merge` → `08_videowriter` → `05_videocapture_camera` |
| Day 2\~3 Core 5 题    | L1: `01_mat_create_type` → `02_pixel_scan` → `04_mask_convolution` → `07_dft_spectrum` → `03_lut_color_reduce`                                                                       |
| Day 4\~5 Imgproc 8 题 | L2: `01_smoothing` → `08_threshold` → `13_canny` → `10_sobel` → `14_hough_lines` → `26_calc_hist` → `30_match_template` → `32_phase_correlate`                                       |
| 按需 Features/Video    | L3: `02_features2d` → `05_optical_flow` → `09_bg_sub` → `10_camshift` → `17_mean_shift`                                                                                              |
| 按需 Detect/Calib      | L4: `01_yunet_face` → `05_calib_pnp` → `07_find_chess` → `12_grabcut`                                                                                                                |

### 路径 B：原理深挖 → `docs/`（233 官方 demo 逐章详解）

1. [docs/principles.md](docs/principles.md) §1\~6：像素表示 → 线性/非线性滤波 → 形态学 → 边缘检测（Sobel/Canny）→ 阈值化 → 几何变换
2. [ch01\_core.md](docs/ch01_core.md) + [ch02\_imgproc.md](docs/ch02_imgproc.md) 分章详解
3. principles §7\~12：直方图 + 特征描述子 + 单应匹配 + 轮廓 + 光流 + 背景建模 → 配 [ch03\_features.md](docs/ch03_features.md)
4. principles §13\~16：ML + 目标检测 + 摄影学（HDR/Tonemap/Inpaint）+ 三维（立体视觉）+ GUI/GPU 加速 → 选 [ch04\~ch08](docs/ch04_video.md) 对应章
5. 端到端流程：[samples\_flow.md](docs/samples_flow.md)（"数据输入 → 预处理 → 处理 → 后处理 → 输出"典型 5 条流水线）

### 路径 C：实战派 → `algorithms/`（17 个模块，NV21 真机数据 + PSNR/SSIM 打分）

> 完整总表见 [algorithms/README.md §1 模块速览总表](algorithms/README.md#1-模块速览总表17-个)。

#### C-1 ISP 主流水线（吃真机 NV21，有 GT 可量化打分）

| 算法                                                             | 入口                                                                | 推荐数据                                |
| -------------------------------------------------------------- | ----------------------------------------------------------------- | ----------------------------------- |
| 单帧降噪 11 种 × 4 类噪声 × 参数扫描                                       | [algorithms/denoise\_single](algorithms/denoise_single/README.md) | `data/images/`                      |
| 多帧降噪（ECC/Affine 配准 + 4 种融合）                                    | [algorithms/denoise\_multi](algorithms/denoise_multi/README.md)   | `data/nv21/nr/`                     |
| HDR：2 种 CRF × 2 种 Merge × 7 种 Tonemap + Mertens                | [algorithms/hdr](algorithms/hdr/README.md)                        | `data/nv21/ev/`, `data/nv21/hdr_*/` |
| 夜景增强 9 种：Gamma/CLAHE/SSR/MSR/MSRCR/DCP去雾/ACE/LIME/RetinexTV    | [algorithms/night\_scene](algorithms/night_scene/README.md)       | `data/nv21/ev/` ev=-8               |
| 美颜：频率分离磨皮 + YCrCb CLAHE 提亮 + USM 锐化                            | [algorithms/beauty](algorithms/beauty/README.md)                  | `data/images/` 人像图                  |
| 水印：可见 3 类 + DFT 非盲 + DCT QIM 盲提取 + 30+ 攻击                      | [algorithms/watermark](algorithms/watermark/README.md)            | `data/images/` + 自选 Logo/二维码        |

#### C-2 通用视觉算法（吃通用图片，算法族横向对比）

| 算法                                          | 入口                                                                        | 推荐数据                                  |
| ------------------------------------------- | ------------------------------------------------------------------------- | ------------------------------------- |
| 边缘检测 8 算子：Sobel/Scharr/Prewitt/Laplacian/LoG/DoG/Canny(+Otsu) | [algorithms/edge\_detection](algorithms/edge_detection/README.md)         | `data/images/lena.jpg`                |
| 形态学全套 + SE 形状 / 核大小对比                        | [algorithms/morphology](algorithms/morphology/README.md)                  | `data/images/lena.jpg`                |
| 分割 10 方法：Otsu/自适应/KMeans/MeanShift/GrabCut/Watershed/CC | [algorithms/segmentation](algorithms/segmentation/README.md)              | `data/images/`、`fruits.jpg`           |
| 特征检测 6 种 + ORB 匹配 + RANSAC 内点率                | [algorithms/feature\_detection](algorithms/feature_detection/README.md)   | `data/graf1.png` + `graf3.png`        |
| 立体匹配 StereoBM/SGBM 视差估计                      | [algorithms/stereo](algorithms/stereo/README.md)                          | `data/aloeL.jpg` + `aloeR.jpg`        |
| 去模糊：合成 PSF + 逆滤波/Wiener/RL/USM                | [algorithms/deblur](algorithms/deblur/README.md)                          | `data/images/`（自合成模糊）                |
| 模板匹配 6 种 TM 方法 + 多尺度                          | [algorithms/template\_matching](algorithms/template_matching/README.md)   | `data/images/` + `lena_tmpl.jpg`      |
| 图像修复 Telea/NS + 合成损伤 + mask MAE              | [algorithms/inpaint](algorithms/inpaint/README.md)                        | `data/images/`（自合成划痕）                |
| 霍夫变换：直线 HoughLinesP + 圆 HoughCircles           | [algorithms/hough\_transform](algorithms/hough_transform/README.md)       | `data/sudoku.png`、`smarties.png`      |
| 频域：DFT 谱 + 低通/高通/陷波去周期噪声                      | [algorithms/frequency\_domain](algorithms/frequency_domain/README.md)     | `data/images/`（自叠周期噪声）               |
| 光流：LK 稀疏 + Farneback 稠密                      | [algorithms/optical\_flow](algorithms/optical_flow/README.md)             | `data/vtest.avi`                      |

### 路径 D：笔记式系统学 → `notes/`（毛星云体例）

1. 基础：`image_process/mat` + `image_process/image`
2. 增强：`image_process/morphology` + `image_transformation`
3. 分割：`image_segmentation` + `histogram_match`
4. 特征：`harris_detect` + `features2d`
5. 实战：`face_detect` + `image_process/image_algo`

> tips：`notes/image_process/` 已经做成可独立编译的子项目（含共享静态库 + 17 个 exe），
> 用 `.\build.ps1 -Target notes` 一把编完。

***

### 4.1 知识体系全景图（4 层递进）

```
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ 1. 理论骨架 (docs/principles.md §1~16)                                                │
│    像素 → 滤波 → 形态 → 边缘 → 阈值 → 几何 → 直方图 → 特征 → 匹配 →                  │
│    轮廓 → 光流 → 背景 → ML → 检测 → 摄影 → 三维 → GUI/GPU                              │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        ↓
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ 2. 分章详解 (docs/ch01~ch08)                                                          │
│    每章统一 8 节：概述 / 原理 / API / 流程 / 参数表 / 同类对比 / 注意事项 / 应用场景   │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        ↓
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ 3. 可运行练习 (×3 体系并列)                                                           │
│    learn/ L0~L5  ⇄  notes/ 主题笔记  ⇄  algorithms/ 17 个模块                        │
│    (每个 .cpp 独立 main, 共享 common 工具, 复用同一 data/ 资产)                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        ↓
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ 4. 官方源码对照 (mingw-build/samples/cpp, 233 个 .cpp)                                │
│    learn/README 列出每个练习对应的官方 demo；docs 附录 B 给出 233 demo→章节归属映射    │
└───────────────────────────────────────────────────────────────────────────────────────┘
```

### 4.2 6 周推荐组合："动手 + 原理"双轨制

| 周 | learn 动手主线              | docs 原理主线                     | algorithms 实战配套                                                       |
| - | ----------------------- | ----------------------------- | --------------------------------------------------------------------- |
| 1 | L0 入门 8 题 + L1 Core 5 题 | principles §1\~6（像素/滤波/形态/边缘） | —                                                                     |
| 2 | L2 Imgproc 8 题          | ch02\_imgproc 全节              | denoise\_single：Gauss→Bilateral→NLM→Guided→Wiener→各向异性扩散              |
| 3 | L3 Features+Video       | ch03 特征 + ch04 视频光流           | denoise\_multi：ECC 配准 + 加权平均 / 中值融合                                   |
| 4 | L4 §HDR 相关              | ch06 Photo §HDR/Tonemap       | hdr：Debevec + Drugo CRF → Drago/Reinhard/Mantiuk Tonemap → Mertens 对比 |
| 5 | L4 §Inpaint + GrabCut   | ch06 Photo §Inpaint + 分割      | beauty 频率分离磨皮 + watermark（DFT 非盲 → DCT QIM 盲）                         |
| 6 | L5 ML/GAPI/GPU 加速       | ch05 ML + ch08 GAPI/GPU       | night\_scene（MSRCR 必做 + DCP 去雾 + LIME + RetinexTV）                    |

***

## 5. 图像处理基础知识整理（legacy 骨架快速索引）

> 系统化版本请参考 [docs/principles.md](docs/principles.md) 与各 chXX.md。
> 这里保留项目最初版本积累的 imgproc 章节"一页速查"，做日常扫盲和面试复盘。

### 5.1 图像滤波（Image Filtering）

**目的**：抽出对象特征（图像识别前预处理） / 消除数字化噪音。

| 类别          | 典型算法                          |
| ----------- | ----------------------------- |
| 方框滤波 / 均值滤波 | 最简单低通，去高频噪声但糊边缘               |
| **高斯滤波**    | 最常用平滑，`G(x) ∝ exp(−x²/(2σ²))` |
| **中值滤波**    | 椒盐噪声杀手，排序取中值，非线性              |
| **双边滤波**    | 保边去噪（值域核 + 空间核联合加权），美颜磨皮的基础   |

形态学滤波另一大类：**腐蚀 / 膨胀 / 开运算（先蚀后胀去白噪点）/ 闭运算（先胀后蚀去黑洞）/ 形态学梯度 / 顶帽 / 黑帽 / 漫水填充 / 尺寸缩放 / 阈值化**。

### 5.2 图像变换（Image Transform）

**边缘检测通用三步走**：**滤波 → 增强（梯度幅值）→ 检测（阈值化）**。一步都不能少。

#### Canny 算子 3 步

1. **滤波**：通常 `GaussianBlur(img, Size(5,5), 1.4)`，Canny 对噪声**极度敏感**。
2. **增强**：Sobel x/y 两个方向算梯度幅值 `G = √(Gx²+Gy²)` 和方向角。
3. **检测**：双阈值高低比 `2:1 ~ 3:1`，弱边缘点**必须连到强边缘**才保留（滞后阈值）。

#### Sobel 算子

离散微分 + 高斯平滑的结合。3×3 两个核：

```
Gx = [ -1  0  +1 ]       Gy = [ -1  -2  -1 ]
     [ -2  0  +2 ]            [  0   0   0 ]
     [ -1  0  +1 ]            [ +1  +2  +1 ]
```

实际工程 `|Gx| + |Gy|`（L1 范数近似）就够用了，省 sqrt。

*同类家族对比：Scharr（更准的 3×3 导数近似，对 k=3 时比 Sobel 数学上更优）、Laplacian（二阶导数，过零交叉更敏感，但对噪声加倍放大——必须**先降噪再用**）*

#### 其他常用变换一览

- **霍夫变换**：直线 (`HoughLines` / `HoughLinesP`) / 圆 (`HoughCircles`)

- **重映射** **`remap`**：自定义 (x,y) → (map\_x, map\_y) 形变，鱼眼校正、图像拼接基础

- **仿射** **`warpAffine`**：3 点定 2×3 矩阵，旋转+平移+缩放（平行关系保持）

- **透视** **`warpPerspective`**：4 点定 3×3 单应矩阵，发票/试卷/文档矫正必背

- **直方图均衡化 / CLAHE**：`equalizeHist` 全局易过曝天空/面部高光 → **CLAHE（clipLimit + tileGridSize）** 分块自适应基本吊打原版

***

## 6. 公共工具（避免每个 demo 重复造轮子）

### 6.1 [common/opencv\_utils.h](common/opencv_utils.h)（learn 与 legacy 用）

| API                                                           | 用途                                                    |
| ------------------------------------------------------------- | ----------------------------------------------------- |
| `getImagePath(name)` / `getModelPath(name)` / `getDataRoot()` | 资源相对路径自动回退（找 CWD → ../ → ../../），不管从哪个工作目录 exe 都能找到   |
| `makeSyntheticTestImage(W,H)`                                 | 缺 data/ 图时自动合成一张（彩色渐变 + 白框 + 文字），保证 demo 100% 可跑      |
| `dbgMatInfo` / `dbgStats` / `dbgPixel(x,y)`                   | 一键 Mat 诊断：type+size / min-max-mean / 指定坐标像素值          |
| `dbgShow` / `dbgShowMany` / `dbgSave(tag, dir)`               | 可视化 + 自动加时间戳 PNG 保存                                   |
| `dbgTime(label)` / `dbgTimeEnd(label)`                        | 打印代码块耗时 ms，性能瓶颈排查                                     |
| `logInfo(fmt, …)` / `logWarn` / `logErr`                      | `[echo][HH:MM:SS.mmm]` 时间戳日志，3 档级别                    |
| **`nv21_to_bgr(W,H,path)`**                                   | NV21 raw → BGR 一步转 + 大图自动缩放到 1600 px 展示（**上一轮刚补全实现**） |

### 6.2 [algorithms/common/](algorithms/common/README.md)（算法子项目公用）

| 头文件                       | 内容提要                                                                                                                                                                                                                        |
| ------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **nv21\_io.hpp**          | `readNv21Auto`（文件名自动解 W×H）/ `readNv21Y`（仅Y灰度）/ `loadNv21Dir`（按 ev 降序整组）/ `writeNv21` / 元数据解析（`parseExposureTimeFromName` 等）                                                                                                 |
| **algo\_utils.hpp**       | IQA 全家桶：`psnr` / `ssim` / `msSsim` / `mae` / `mse` / `loe` / `niqeApprox` / `eme` / `entropy` / `colorSaturation`；拼图 `hstackWithLabels` / `imshowFit`；配准 `alignECC` / `alignToRef`；分块迭代 `processTiled`（防止 4000×3000 大图 OOM） |
| **single\_denoise.hpp**   | 11 种单帧降噪：高斯 / 中值 / 双边 / NLM / 引导滤波 / 自适应双边 / Wiener / 各向异性扩散 / 拉普拉斯软阈值 / 小波收缩 / 简化 BM3D                                                                                                                                     |
| **hdr/hdr\_pipeline.hpp** | 2 种 CRF（Debevec / Drugo）× 2 种 Merge × 7 种 Tonemap（Drago / Reinhard / Mantiuk / Durand / Linear / LogLuv / CLAHE 近似） + Mertens 曝光融合                                                                                          |

### 6.3 [models/README.md](models/README.md)（DNN 模型清单）

YuNet 2023mar 人脸检测 + SFace 2021dec 人脸识别两张 ONNX 的：

- 文件清单（Hash 校验 + 来源链接）

- 调用速查代码片段（`FaceDetectorYN::create` → `alignCrop` → `SFace::create` → `feature` → `cosine match`）

- 新增模型 Checklist

- License 说明

***

## 7. 常见坑 & FAQ（10 条踩过的坑）

| 现象                              | 根因                                         | 解决                                                                     |
| ------------------------------- | ------------------------------------------ | ---------------------------------------------------------------------- |
| 改 ROI 后原图也变了                    | Mat 是「头 + 数据」两体，ROI 浅拷贝共享指针                | 独立副本用 `clone()` 或 `copyTo()`，别写 `Mat dst = src(Rect(x,y,w,h));` 就改 dst |
| 窗口一闪而过                          | HighGUI 没有驱动事件循环                           | GUI 代码末尾必须 `waitKey(>0)`，且确保 `cv::Mat` 窗口对象别先析构                        |
| Sobel / Scharr 结果全花屏            | 负值被 `saturate_cast<uchar>` 截成 0            | 中间用 `CV_16S` / `CV_32F`，最后一步才 `convertScaleAbs`                        |
| Canny 边缘碎成片                     | 没先降噪                                       | 先 `GaussianBlur(Size(5,5), 1.4)`，再 `Canny(thrLow, thrHigh=2×thrLow)`   |
| `calcHist` 显示为空 / 形状不对          | ranges 上界是 exclusive                       | 用 `[0,256)` 而不是 `[0,255]`                                              |
| HSV `inRange` 颜色分割完全不工作         | 你写了 H∈\[0,360)                             | OpenCV 8U HSV 里 **H 的范围是 0\~179**；S/V 才是 0\~255                        |
| `cornerHarris` 响应图看不见           | R 值数值极小（浮点 0.x 级别）                         | `normalize` 到 \[0,255] 再 `imshow`，或加 `threshold` 后用彩色标记                |
| SURF / SIFT 编译报 "has no member" | 没编 opencv\_contrib                         | 自编译 OpenCV 时必须设置 `OPENCV_EXTRA_MODULES_PATH` 指向 contrib/modules        |
| Debevec HDR 在 OpenCV 4.13 崩断言   | `cv::CalibrateCRF` 的 BGR 通道 reshape 已知 bug | 绕路：拆 BGR 为单通道逐通道跑再 Merge，或直接用 **Mertens 曝光融合**（不需要 CRF，实测稳）            |
| `algo_*.exe` 运行找不到 data 图片      | CWD 不是 exe 所在目录                            | `cd` 到 exe 目录再启动；或直接用 common 里的 `getImagePath()` 自带回退                  |

***

## 8. 后续路线图（短 / 中 / 长三期）

### 短期（低风险增量，1\~2 天可完成一项）

- [ ] algorithms 补齐：完整 BM3D / VBM3D（需 opencv\_contrib xphoto）、LIME 夜景补实现、Skin Mask 美颜

- [ ] watermark：DCT 域水印 + 几何攻击鲁棒性自动测试套件（旋转/缩放/JPEG/裁剪/噪声）

- [ ] beauty 接入 `models/face_detection_yunet_2023mar.onnx`：升级到"人脸框内磨皮 + 皮肤保护"

- [ ] HDR 加 Reinhard / Mantiuk / Durand tonemap 的对比图 + 参数扫矩阵

- [ ] `algorithms/common` 加 `--headless` 环境变量跳过 imshow（CI/服务器友好）

### 中期（向 DNN 迁移，一周级）

- [ ] learn/L5 已有 `digits_dnn.cpp` 框架 → 扩展为 ONNX Runtime 风格 UNet 降噪 / 美颜

- [ ] 人脸识别小流水线：YuNet 检测 → `alignCrop` → SFace 提 128 维特征 → cosine 相似度

- [ ] 引入 G-API 流水线对照（参考 [tutorial\_code/gapi/face\_beautification.cpp](mingw-build/samples/cpp/tutorial_code/gapi/face_beautification/face_beautification.cpp)）

### 长期（ISP 小引擎，月级）

- [ ] algorithms 升级为"端到端小 ISP"：NV21 输入 → 多帧降噪 → HDR Merge → 夜景增强 → 美颜 → 水印 → JPEG 输出

- [ ] 加 benchmarks 目录：固定数据集 + 固定参数的 PSNR/SSIM/LOE/timing 表，支持回归对比

- [ ] 接入 CI（GitHub Actions / Gitea Actions）：每次 push 跑 benchmarks 并输出 diff

***

## 9. 参考资源

| 类别            | 链接 / 位置                                                                                     |
| ------------- | ------------------------------------------------------------------------------------------- |
| 官方源码（本仓库自带）   | [mingw-build/samples/cpp](mingw-build/samples/cpp) — **233 个** .cpp                         |
| 官方教程（本仓库自带）   | [mingw-build/samples/cpp/tutorial\_code](mingw-build/samples/cpp/tutorial_code) — **136 个** |
| OpenCV 在线文档   | <https://docs.opencv.org/4.x/>                                                              |
| 中文参考书         | 《OpenCV3编程入门》毛星云 编著 · 电子工业出版社（仓库根 PDF，如存在）                                                  |
| 自编 MinGW 编译教程 | <https://blog.huihut.com/2018/07/31/CompiledOpenCVWithMinGW64/>                             |
| 本项目内部导航       | 就是本 README 😎（原 OUTLINE.md 已并入）                                                             |

***

## 10. 维护硬约束 & 命名规范（所有贡献者必读）

### 10.1 维护约定（底线）

- ❌ **`data/`** **和** **`mingw-build/`** **是只读依赖**，永远不修改其中的任何文件（包括手动改名/增删）。

- ✅ 新增算法 demo 放到 `algorithms/<模块名>/`，每个模块**至少**有：独立子目录 + `main.cpp` + `README.md`。

- ✅ 新增逐题练习放到 `learn/`，严格遵循 L0\~L5 分层与"主干 80%"主线约定（见 [learn/README.md](learn/README.md)）。

- ✅ 原理文档统一在 `docs/` 维护，遵循 [docs/README.md 附录 A 结构章程](docs/README.md#附录-a-结构章程doc-charter)。

- ✅ 笔记式整理放 `notes/`，按主题分子目录，每个子目录配 README。

- ✅ 新增 ONNX 模型放 `models/`，**必须同步写入** [models/README.md](models/README.md) 的清单表（含来源 / 用途 / License）。

### 10.2 命名规范（Naming Conventions，章程级）

> 本节是**仓库内所有目录与文件命名的唯一规范**。新增/重命名必须遵守；遇到旧写法不
> 符合的，改齐。

#### 10.2.1 总则

| 元素                         | 规则                                                | 正确示例                                                                       |
| -------------------------- | ------------------------------------------------- | -------------------------------------------------------------------------- |
| **文件夹**                    | `snake_case`（全小写 + 下划线）                           | `image_process/` `harris_detect/` `face_detect/`                           |
| **C++ 源文件** (.cpp/.c)      | `snake_case`                                      | `corner_harris.cpp` `single_frame_process.cpp`                             |
| **C++ 头文件** (.h/.hpp)      | `snake_case`                                      | `opencv_utils.h` `nv21_io.hpp`                                             |
| **Markdown 文档** (.md)      | `snake_case`，**固定名例外**：`README.md` / `OUTLINE.md` | `ch01_core.md` `samples_flow.md`                                           |
| **PowerShell / Python 脚本** | `snake_case`                                      | `build.ps1` `clean.ps1` `normalize_docs.py`                                |
| **数据/模型文件**                | 保留原始/上游命名                                         | `face_detection_yunet_2023mar.onnx`                                        |
| **CMake target / 选项**      | 全大写 + 下划线；布尔开关前缀 `BUILD_`                         | `BUILD_LEARN` `BUILD_ALGORITHMS` `ALGO_MODULE` `LEARN_LAYER` `BUILD_NOTES` |

#### 10.2.2 例外与保留

- **保护目录**：`data/`、`mingw-build/`、`.idea/`、`.git/` 内的任何文件**不允许修改**（前两者是数据/工具链硬约束，后两者是 IDE/VCS 元数据）。

- **特殊固定名**：`README.md`（每个目录入口文档）、`OUTLINE.md`（保留作跳转，避免书签断链）、`CMakeLists.txt`（CMake 固定文件名）。

- **legacy 兼容名**：根 `main.cpp`（legacy `openCv` 可执行源文件）、`openCv` target 名——出于历史兼容保留，不强制改名。

#### 10.2.3 顶层目录角色表

| 目录             | 角色            | 备注                                                  |
| -------------- | ------------- | --------------------------------------------------- |
| `learn/`       | 分层练习 (L0\~L5) | 每层子目录, 每个 .cpp 独立 main                              |
| `docs/`        | 原理与官方示例详解     | 8 章 markdown + principles + samples\_flow + scripts |
| `notes/`       | 毛星云体例学习笔记     | 按主题分子目录, 与 learn 互补, image\_process 可独立 CMake       |
| `algorithms/`  | 端到端算法 demo 集  | 17 个模块（6 ISP + 11 通用）+ 公共 `common/` 静态库              |
| `common/`      | 跨子项目共享工具      | `opencv_utils.{cpp,h}` 供 learn / legacy 用           |
| `models/`      | ONNX 模型       | 只读依赖（允许新增，但必须同步 models/README）                      |
| `data/`        | 测试数据          | 只读依赖                                                |
| `mingw-build/` | OpenCV 编译产物   | 只读依赖                                                |
| `out/`         | 算法 demo 输出    | build artifacts，除 README 外可随时清空                     |

#### 10.2.4 PR / 提交前命名改写检查清单

- [ ] 新建文件夹/文件是否 `snake_case`？

- [ ] 是否避免了大小写混写 (`camelCase`) 或 Pascal 帽 (`PascalCase`)？

- [ ] Windows 上单步大小写重命名是否走**中转名两步**（`Foo` → `_tmp_foo` → `foo`），避免 NTFS 大小写不敏感冲突？

- [ ] 新增 CMake 布尔开关是否按 `BUILD_<SUBPROJECT>` 模式命名？模块选择器用 `<MODULE>_MODULE`（单数）/ `<PROJECT>_LAYER`？

- [ ] 同一概念在文档 / 源码 / CMake 选项三处是否用同一词？（例如：统一 `denoise_single`，不要混用 `denoise` / `single_denoise`）

> **规范即法律。** 凡与本节冲突的旧写法，一律以本节为准重写。

