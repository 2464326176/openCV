# OpenCV C++ 学习与实战项目（跨平台：Windows MinGW + Linux）

Windows + MinGW 工具链的 **OpenCV 4.13 学习 + 实战** 双形态仓库：从零基础逐题练习（118 个 .cpp）→ 官方 233 demo 中文原理详解 → **17 个端到端算法 Demo**（6 个 ISP 主流水线 + 11 个通用视觉算法）。

## 0. 项目概览

| 维度 | 说明 |
| --- | --- |
| 语言 / 工具链 | C++（OpenCV 4.13，MinGW 14.2.0 x86_64-posix-seh-ucrt 静态链接）+ CMake 3.24+ |
| 静态链接零依赖 | `mingw-build/x64/mingw/staticlib/` 自带预编译 OpenCV，exe 发布无需 DLL |
| 四套学习体系 | `learn/` 练习树 + `docs/` 原理详解 + `notes/` 毛星云笔记 + `algorithms/` 实战 Demo |
| 真机 NV21 数据 | `data/nv21/` 含 4032×3000 曝光序列 / GT HDR 输出 / 降噪 in-out 对，直接喂 ISP 算法 |
| 17 个算法 Demo | 6 ISP（单/多帧降噪、HDR、夜景、美颜、水印）+ 11 通用（边缘/形态/分割/特征/立体/去模糊/模板匹配/修复/霍夫/频域/光流） |
| 评估工具链 | PSNR / SSIM / MS-SSIM / MAE / LOE / NIQE 近似 / EME / 熵 / 色彩饱和度 内置 |
| 2 个 ONNX 模型 | YuNet 人脸检测 + SFace 人脸识别（OpenCV DNN / FaceDetectorYN API） |
| 跨平台一键构建 | Windows `build.ps1`（`-Action build`/`-Action clean`）+ Linux `build.sh`（子命令 `build`/`clean`） |

## 1. 环境与安装

### 1.1 MinGW-w64

下载：<https://sourceforge.net/projects/mingw-w64/files/>

推荐 **`x86_64-posix-sjlj`** — 与 `mingw-build/` 预编译产物工具链一致。

| 变体 | 说明 |
| --- | --- |
| x86_64-posix-seh | 64 位 + POSIX 线程 + SEH 异常（性能好，仅 64 位） |
| x86_64-posix-sjlj | 64 位 + POSIX 线程 + SJLJ（稳定、兼容 32 位） |
| i686-win32-dwarf | 32 位版本 |

- POSIX vs Win32：POSIX 线程模型让 `std::thread`/OpenMP 跨平台一致；Win32 线程仅 Windows 原生。
- SEH 性能好但仅 64 位；SJLJ 古老但稳定、32/64 通用。

### 1.2 CMake

<https://cmake.org/download/>，选 Windows x64 Installer 并勾选「Add CMake to PATH」。

### 1.3 OpenCV 源码

- 源码：<https://github.com/opencv/opencv>（需要 SURF/SIFT/xfeatures2d 时必选 `opencv_contrib`）
- 自编图文教程：<https://blog.huihut.com/2018/07/31/CompiledOpenCVWithMinGW64/>
- 自编产出即本仓库 `mingw-build/`，直接复用，无需再编。

## 2. 仓库结构

```
openCV/
├── learn/           分层逐题练习 L0~L5（100+ 独立 .cpp，每个有 main）
├── docs/            官方 233 demo 原理详解（8 章 + principles + samples_flow）
├── notes/           毛星云体例学习笔记（8 章 16+ 子目录，image_process 可独立编译）
├── algorithms/      算法实战 Demo 集（17 模块 + common 静态库）
├── common/          跨子项目共享工具（opencv_utils.{h,cpp}）
├── data/            测试数据（NV21 真机 + 通用 JPG/PNG）只读
├── models/          ONNX 模型（YuNet 检测 / SFace 识别）
├── mingw-build/     OpenCV 编译产物（头文件 + 静态库 + 官方 sample）只读
├── out/             algorithms 运行时输出 PNG（可清理）
├── build.ps1        Windows 一键构建（main/learn/algorithms/notes/all）
├── build.sh         Linux 构建/清理（bash，镜像 build.ps1）
├── CMakeLists.txt   根 CMake：自动发现 + 动态 BUILD_<GROUP> 开关 + LEARN_LAYER + ALGO_MODULE
├── openCv.cpp       根主程序源文件（产物 build/main/openCv）
└── README.md        本文件
```

### 2.1 目录角色与入口

| 目录 | 角色 | 适合谁 / 形态 | 入口 |
| --- | --- | --- | --- |
| `learn/` | 教程式逐题练习 | 边学边写，40~80 行最小可运行练习 | [learn/README.md](learn/README.md) |
| `docs/` | 原理 + 官方 233 demo 详解 | 啃原理 + 调参，参数表 | [docs/README.md](docs/README.md) / [principles.md](docs/principles.md) |
| `notes/` | 毛星云体例学习笔记 | 按章节组织（可独立构建 exe） | [notes/README.md](notes/README.md) |
| `algorithms/` | 算法实战 demo 集 | 生产级算法，跑 NV21 真机数据，看 PSNR/SSIM | [algorithms/README.md](algorithms/README.md) |
| `common/` | 公共工具 | learn / legacy 用 | [opencv_utils.h](common/opencv_utils.h) |
| `models/` | ONNX 模型 | 只读（新增写入 README 清单） | [models/README.md](models/README.md) |
| `out/` | 算法 demo 输出 PNG | build artifacts，可清理 | [out/README.md](out/README.md) |
| `data/` / `mingw-build/` | 只读依赖 | 永不修改 | — |

### 2.2 数据资产（data/ 导航）

```
data/
├── images/     *.jpg / *.png 通用图（lena / VCG / baboon / fruits / messi5 …）
├── nv21/       真机 NV21 raw（ISP 算法主粮）
│   ├── ev/             3 帧 ev=-8/-4/0 NV21 (4032×3000) → HDR / 夜景
│   ├── hdr_<编号>/      同组曝光序列 + ground truth merge_3.NV21 → HDR GT 对比
│   └── nr/             YNRCNR 单帧降噪 in/out (3264×2448) → 降噪 in-out 对
├── dnn/        类标签文件 (coco/yolo/imagenet/…) → DNN demo
├── aruco/      Charuco 标定数据 → ArUco demo
└── *.jpg / *.png  OpenCV 官方示例原图
```

NV21 命名约定：`..._WWWWxHHHH_..._ev_Z_iso_YYY_et_XXX_base_B.NV21`

- 宽高自动从文件名解析（`algo::parseNv21SizeFromName`）；
- `et` / `EV` / `iso` / `base` 元数据均从文件名解析（`parseExposureTimeFromName` 等）；
- 真实手机 dump 的 NV21 按此命名丢进 `data/nv21/`，ISP 类模块全部自动读，无需改代码。

## 3. 一分钟构建

环境前提（Windows）：MinGW、CMake 已在 PATH。

OpenCV 查找优先级（根 `CMakeLists.txt` 统一控制，Windows/Linux 一致）：
1. 显式 `-DOpenCV_DIR=...`（最高优先）；
2. 否则若本地 `mingw-build/x64/mingw/staticlib/` 静态库存在则用它；
3. 否则 `find_package(OpenCV)`（系统已装或用 `-DOpenCV_DIR` 指定）。

> 无需固定环境变量；仓库可拷贝到任意磁盘/机器，缺 `mingw-build` 时自动回退系统 OpenCV。

### 3.1 最快上手

```powershell
.\build.ps1 -Target all        # 编 legacy main + 全部 learn + 全部 algorithms
```

产物名 = 源 `.cpp` 主名，目录统一 `build/<group>/`，跨组同名不撞。

```powershell
# algorithms：17 个算法 Demo，输出 PNG 到 out/algorithms/（需先 -Module ALL，见 3.2）
cd build\algorithms
.\denoise_single.exe           # ISP：单帧降噪
.\denoise_multi.exe            # ISP：多帧降噪
.\hdr.exe                      # ISP：HDR
.\night_scene.exe              # ISP：夜景
.\beauty.exe                   # ISP：美颜
.\watermark.exe                # ISP：水印
.\edge_detection.exe           # 通用：边缘
.\morphology.exe               # 通用：形态
.\segmentation.exe             # 通用：分割
.\feature_detection.exe        # 通用：特征
.\stereo.exe                   # 通用：立体
.\deblur.exe                   # 通用：去模糊
.\template_matching.exe        # 通用：模板匹配
.\inpaint.exe                  # 通用：修复
.\hough_transform.exe          # 通用：霍夫
.\frequency_domain.exe         # 通用：频域
.\optical_flow.exe             # 通用：光流
# 一把全跑：
Get-ChildItem *.exe | ForEach-Object { Write-Host "=== $($_.Name) ===" -ForegroundColor Cyan; & $_.FullName }

# learn：产物在 build\learn
cd ..\..\build\learn
.\learn_07_opencv_version.exe
.\learn_01_hello_imread.exe

# notes：毛星云体例（产物在 build\notes）
cd ..\..\build\notes
.\image_read_image.exe
```

### 3.2 脚本典型用法

```powershell
.\build.ps1 -Target learn -Layer L2                                      # learn L2 层
.\build.ps1 -Target algorithms -Module "hdr;denoise_single;beauty"       # 只编 3 个模块
.\build.ps1 -Target notes                                                # 单独编 notes
.\build.ps1 -Config Debug                                                # 编 Debug
.\build.ps1 -Target all -NoBuild                                         # 只生成 Makefile
```

### 3.3 等价原生 CMake

```powershell
# A. 根 openCv → build/main
cmake -B build/main -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release `
  -DBUILD_MAIN=ON -DBUILD_LEARN=OFF -DBUILD_ALGORITHMS=OFF -DBUILD_NOTES=OFF
cmake --build build/main -j

# B. learn 单层 → build/learn
cmake -B build/learn -G "MinGW Makefiles" -DBUILD_MAIN=OFF -DBUILD_LEARN=ON -DLEARN_LAYER=L2
cmake --build build/learn -j

# C. algorithms → build/algorithms
cmake -B build/algorithms -G "MinGW Makefiles" -DBUILD_MAIN=OFF -DBUILD_ALGORITHMS=ON -DALGO_MODULE=ALL
cmake --build build/algorithms -j

# D. notes/image_process → build/notes
cmake -B build/notes -G "MinGW Makefiles" -DBUILD_MAIN=OFF -DBUILD_NOTES=ON
cmake --build build/notes -j

# E. 任意新增顶层目录（含 *.cpp）→ build/<dir>，自动生成 BUILD_<DIR>
cmake -B build/my_demo -G "MinGW Makefiles" -DBUILD_MAIN=OFF -DBUILD_MY_DEMO=ON
cmake --build build/my_demo -j
```

### 3.4 编译开关矩阵

| CMake 选项 | 默认 | 含义 |
| --- | --- | --- |
| `BUILD_MAIN` | `ON` | 构建根 `openCv` 可执行（VideoCapture + drawText 示例），产物 `build/main/openCv` |
| `BUILD_LEARN` | `OFF` | 启用 learn 子项目 |
| `LEARN_LAYER` | `ALL` | learn 编译层：`ALL` / `L0`~`L5` |
| `BUILD_ALGORITHMS` | `OFF` | 启用 algorithms 子项目 |
| `ALGO_MODULE` | `hdr;denoise_single` | algorithms 编译模块：`ALL` 或分号分隔列表 |
| `BUILD_NOTES` | `OFF` | 启用 notes/image_process legacy 子项目 |
| `BUILD_<GROUP>` | `OFF` | 自动发现：任意新增顶层目录（含 `*.cpp`）动态生成此开关，`ON` 即编译，**零 CMake 改动** |

### 3.5 清理

```powershell
.\build.ps1 -Action clean                          # 清 build*/ + CMake 缓存
.\build.ps1 -Action clean -Mode cmake              # 只清散落 CMake 临时文件（保留 build）
.\build.ps1 -Action clean -Mode out                # 清 out/algorithms/*.png（保留 README）
.\build.ps1 -Action clean -Mode all                # 上述三样一次性清
.\build.ps1 -Action clean -Mode all -DryRun        # 先打印会删什么，不落盘
```

> `build.ps1 -Action clean` 内含保护逻辑，`data/` 与 `mingw-build/` 无论何种参数都**绝不会触碰**。

### 3.6 Linux（bash，与 Windows 对称）

```bash
chmod +x build.sh
./build.sh main                              # 主程序 → build/main
./build.sh learn L2                          # learn L2 → build/learn
./build.sh algorithms "hdr;denoise_single"   # 指定模块 → build/algorithms
./build.sh notes                             # notes → build/notes
./build.sh all                               # 全构建（含 notes）
NO_BUILD=1 ./build.sh all                     # 只生成不编译
./build.sh clean                             # 清 build/ + CMake 缓存（保留 out/、mingw-build/data）
./build.sh clean all                         # 额外清 out/ 算法产物（保留 README）
```

> 两端共用同一份 `CMakeLists.txt`，构建目录一致（`build/main`、`build/learn`、`build/algorithms`、`build/notes`），产物名 = 源 `.cpp` 主名；OpenCV 查找逻辑一致。

## 4. 四条学习路径

### 路径 A：零基础 1 周入门 → `learn/`

| 阶段 | 主干练习（按 `learn/L<X>/NN_xxx.cpp`） |
| --- | --- |
| Day 1 入门 8 题 | L0: `07_opencv_version` → `01_hello_imread` → `04_drawing_primitives` → `02_named_window_trackbar` → `03_mouse_roi` → `06_split_merge` → `08_videowriter` → `05_videocapture_camera` |
| Day 2~3 Core 5 题 | L1: `01_mat_create_type` → `02_pixel_scan` → `04_mask_convolution` → `07_dft_spectrum` → `03_lut_color_reduce` |
| Day 4~5 Imgproc 8 题 | L2: `01_smoothing` → `08_threshold` → `13_canny` → `10_sobel` → `14_hough_lines` → `26_calc_hist` → `30_match_template` → `32_phase_correlate` |
| 按需 Features/Video | L3: `02_features2d` → `05_optical_flow` → `09_bg_sub` → `10_camshift` → `17_mean_shift` |
| 按需 Detect/Calib | L4: `01_yunet_face` → `05_calib_pnp` → `07_find_chess` → `12_grabcut` |

### 路径 B：原理深挖 → `docs/`

1. [principles.md](docs/principles.md) §1~6：像素 → 滤波 → 形态 → 边缘 → 阈值 → 几何变换
2. [ch01_core.md](docs/ch01_core.md) + [ch02_imgproc.md](docs/ch02_imgproc.md) 分章详解
3. principles §7~12：直方图 + 特征描述子 + 单应匹配 + 轮廓 + 光流 + 背景建模 → 配 [ch03_features.md](docs/ch03_features.md)
4. principles §13~16：ML + 目标检测 + 摄影学 + 三维 + GUI/GPU → 选 [ch04~ch08](docs/ch04_video.md)
5. 端到端流程：[samples_flow.md](docs/samples_flow.md)

### 路径 C：实战派 → `algorithms/`

> 完整总表见 [algorithms/README.md §1 模块速览总表](algorithms/README.md#1-模块速览总表17-个)。

#### ISP 主流水线（吃真机 NV21，有 GT 可量化）

| 算法 | 入口 | 推荐数据 |
| --- | --- | --- |
| 单帧降噪 11 种 × 4 类噪声 × 参数扫描 | [denoise_single](algorithms/denoise_single/README.md) | `data/images/` |
| 多帧降噪（ECC/Affine 配准 + 4 种融合） | [denoise_multi](algorithms/denoise_multi/README.md) | `data/nv21/nr/` |
| HDR：2 CRF × 2 Merge × 7 Tonemap + Mertens | [hdr](algorithms/hdr/README.md) | `data/nv21/ev/`, `data/nv21/hdr_*/` |
| 夜景增强 9 种：Gamma/CLAHE/SSR/MSR/MSRCR/DCP/ACE/LIME/RetinexTV | [night_scene](algorithms/night_scene/README.md) | `data/nv21/ev/` ev=-8 |
| 美颜：频率分离磨皮 + YCrCb CLAHE 提亮 + USM 锐化 | [beauty](algorithms/beauty/README.md) | `data/images/` 人像 |
| 水印：可见 3 类 + DFT 非盲 + DCT QIM 盲提取 + 30+ 攻击 | [watermark](algorithms/watermark/README.md) | `data/images/` + Logo/二维码 |

#### 通用视觉算法（吃通用图片，横向对比）

| 算法 | 入口 | 推荐数据 |
| --- | --- | --- |
| 边缘检测 8 算子：Sobel/Scharr/Prewitt/Laplacian/LoG/DoG/Canny(+Otsu) | [edge_detection](algorithms/edge_detection/README.md) | `data/images/lena.jpg` |
| 形态学全套 + SE 形状 / 核大小对比 | [morphology](algorithms/morphology/README.md) | `data/images/lena.jpg` |
| 分割 10 方法：Otsu/自适应/KMeans/MeanShift/GrabCut/Watershed/CC | [segmentation](algorithms/segmentation/README.md) | `data/images/`、`fruits.jpg` |
| 特征检测 6 种 + ORB 匹配 + RANSAC 内点率 | [feature_detection](algorithms/feature_detection/README.md) | `data/graf1.png` + `graf3.png` |
| 立体匹配 StereoBM/SGBM 视差 | [stereo](algorithms/stereo/README.md) | `data/aloeL.jpg` + `aloeR.jpg` |
| 去模糊：合成 PSF + 逆滤波/Wiener/RL/USM | [deblur](algorithms/deblur/README.md) | `data/images/`（自合成） |
| 模板匹配 6 种 TM + 多尺度 | [template_matching](algorithms/template_matching/README.md) | `data/images/` + `lena_tmpl.jpg` |
| 图像修复 Telea/NS + 合成损伤 + mask MAE | [inpaint](algorithms/inpaint/README.md) | `data/images/`（自合成） |
| 霍夫：直线 HoughLinesP + 圆 HoughCircles | [hough_transform](algorithms/hough_transform/README.md) | `data/sudoku.png`、`smarties.png` |
| 频域：DFT 谱 + 低通/高通/陷波 | [frequency_domain](algorithms/frequency_domain/README.md) | `data/images/`（自叠噪声） |
| 光流：LK 稀疏 + Farneback 稠密 | [optical_flow](algorithms/optical_flow/README.md) | `data/vtest.avi` |

### 路径 D：笔记式系统学 → `notes/`

1. 基础：`image_process/mat` + `image_process/image`
2. 增强：`image_process/morphology` + `image_transformation`
3. 分割：`image_segmentation` + `histogram_match`
4. 特征：`harris_detect` + `features2d`
5. 实战：`face_detect` + `image_process/image_algo`

> `notes/image_process/` 已做成可独立编译子项目，用 `.\build.ps1 -Target notes` 一把编完。

### 4.1 知识体系（4 层递进）

1. **理论骨架** `docs/principles.md §1~16`：像素 → 滤波 → 形态 → 边缘 → 阈值 → 几何 → 直方图 → 特征 → 匹配 → 轮廓 → 光流 → 背景 → ML → 检测 → 摄影 → 三维 → GUI/GPU
2. **分章详解** `docs/ch01~ch08`：每章统一 8 节（概述/原理/API/流程/参数表/同类对比/注意事项/应用场景）
3. **可运行练习**（×3 并列）：`learn/` L0~L5 ⇄ `notes/` 主题笔记 ⇄ `algorithms/` 17 模块（共享 common 工具，复用同一 data/）
4. **官方源码对照** `mingw-build/samples/cpp`（233 个 .cpp）：learn 列出练习对应官方 demo，docs 附录 B 给 233 demo→章节映射

### 4.2 6 周推荐组合

| 周 | learn 动手主线 | docs 原理主线 | algorithms 实战配套 |
| --- | --- | --- | --- |
| 1 | L0 入门 8 题 + L1 Core 5 题 | principles §1~6 | — |
| 2 | L2 Imgproc 8 题 | ch02_imgproc | denoise_single：Gauss→Bilateral→NLM→Guided→Wiener→各向异性 |
| 3 | L3 Features+Video | ch03 特征 + ch04 视频光流 | denoise_multi：ECC 配准 + 融合 |
| 4 | L4 §HDR | ch06 Photo §HDR/Tonemap | hdr：Debevec + Drugo → Tonemap → Mertens |
| 5 | L4 §Inpaint + GrabCut | ch06 §Inpaint + 分割 | beauty 磨皮 + watermark（DFT→DCT QIM） |
| 6 | L5 ML/GAPI/GPU | ch05 ML + ch08 GAPI/GPU | night_scene（MSRCR + DCP + LIME + RetinexTV） |

## 5. 图像处理基础知识（legacy 速查）

> 系统化版本见 [docs/principles.md](docs/principles.md) 与各 chXX.md。

### 5.1 图像滤波

**目的**：抽出对象特征（识别前预处理）/ 消除数字化噪音。

| 类别 | 典型算法 |
| --- | --- |
| 方框 / 均值滤波 | 最简单低通，去高频噪声但糊边缘 |
| 高斯滤波 | 最常用平滑，`G(x) ∝ exp(−x²/(2σ²))` |
| 中值滤波 | 椒盐噪声杀手，排序取中值，非线性 |
| 双边滤波 | 保边去噪（值域+空间核），美颜磨皮基础 |

形态学：腐蚀 / 膨胀 / 开运算（先蚀后胀去白噪）/ 闭运算（先胀后蚀去黑洞）/ 形态学梯度 / 顶帽 / 黑帽 / 漫水填充 / 尺寸缩放 / 阈值化。

### 5.2 图像变换

**边缘检测三步走**：滤波 → 增强（梯度幅值）→ 检测（阈值化）。

**Canny 3 步**：
1. 滤波：`GaussianBlur(img, Size(5,5), 1.4)`，对噪声极度敏感；
2. 增强：Sobel x/y 算梯度幅值 `G = √(Gx²+Gy²)` 和方向角；
3. 检测：双阈值高低比 `2:1~3:1`，弱边缘须连到强边缘才保留（滞后阈值）。

**Sobel**：离散微分 + 高斯平滑。3×3 核：

```
Gx = [ -1  0  +1 ]       Gy = [ -1  -2  -1 ]
     [ -2  0  +2 ]            [  0   0   0 ]
     [ -1  0  +1 ]            [ +1  +2  +1 ]
```

实际 `|Gx|+|Gy|`（L1 近似）够用，省 sqrt。同类：Scharr（更准 3×3 导数）、Laplacian（二阶，过零交叉敏感，须先降噪）。

**其他变换**：
- 霍夫：直线 `HoughLines`/`HoughLinesP`、圆 `HoughCircles`
- 重映射 `remap`：(x,y)→(map_x,map_y)，鱼眼校正/拼接基础
- 仿射 `warpAffine`：3 点定 2×3 矩阵，旋转+平移+缩放
- 透视 `warpPerspective`：4 点定 3×3 单应，文档矫正
- 直方图均衡 / CLAHE：`equalizeHist` 全局易过曝 → CLAHE（`clipLimit`+`tileGridSize`）分块自适应更优

## 6. 公共工具

### 6.1 [common/opencv_utils.h](common/opencv_utils.h)（learn 与 legacy 用）

| API | 用途 |
| --- | --- |
| `getImagePath(name)` / `getModelPath(name)` / `getDataRoot()` | 资源路径全部相对（含 `../../../` 多级回退），仓库挪任意位置无需重编即可定位 data/ 与 models/ |
| `makeSyntheticTestImage(W,H)` | 缺 data/ 图时自动合成（彩色渐变+白框+文字），保证 demo 可跑 |
| `dbgMatInfo` / `dbgStats` / `dbgPixel(x,y)` | Mat 诊断：type+size / min-max-mean / 指定坐标像素值 |
| `dbgShow` / `dbgShowMany` / `dbgSave(tag, dir)` | 可视化 + 自动时间戳 PNG 保存 |
| `dbgTime(label)` / `dbgTimeEnd(label)` | 打印代码块耗时 ms |
| `logInfo` / `logWarn` / `logErr` | `[echo][HH:MM:SS.mmm]` 时间戳日志，3 档级别 |
| `nv21_to_bgr(W,H,path)` | NV21 raw → BGR 一步转 + 大图自动缩放到 1600 px |

### 6.2 [algorithms/common/](algorithms/common/README.md)

| 头文件 | 内容 |
| --- | --- |
| `nv21_io.hpp` | `readNv21Auto`（文件名解 W×H）/ `readNv21Y` / `loadNv21Dir`（按 ev 降序）/ `writeNv21` / 元数据解析 `parseExposureTimeFromName` 等 |
| `algo_utils.hpp` | IQA：`psnr`/`ssim`/`msSsim`/`mae`/`mse`/`loe`/`niqeApprox`/`eme`/`entropy`/`colorSaturation`；拼图 `hstackWithLabels`/`imshowFit`；配准 `alignECC`/`alignToRef`；分块 `processTiled`（防大图 OOM） |
| `single_denoise.hpp` | 11 种单帧降噪：高斯/中值/双边/NLM/引导/自适应双边/Wiener/各向异性扩散/拉普拉斯软阈值/小波收缩/简化 BM3D |
| `hdr/hdr_pipeline.hpp` | 2 CRF（Debevec/Drugo）× 2 Merge × 7 Tonemap（Drago/Reinhard/Mantiuk/Durand/Linear/LogLuv/CLAHE 近似）+ Mertens 曝光融合 |

### 6.3 [models/README.md](models/README.md)

YuNet 2023mar 人脸检测 + SFace 2021dec 人脸识别两张 ONNX：文件清单（Hash+来源）、调用速查（`FaceDetectorYN::create`→`alignCrop`→`SFace::create`→`feature`→cosine match）、新增 Checklist、License 说明。

## 7. 常见坑 & FAQ

| 现象 | 根因 | 解决 |
| --- | --- | --- |
| 改 ROI 后原图也变 | Mat 是头+数据，ROI 浅拷贝共享指针 | 独立副本用 `clone()`/`copyTo()`，别写 `Mat dst = src(Rect(x,y,w,h));` 就改 dst |
| 窗口一闪而过 | HighGUI 无事件循环 | 末尾必须 `waitKey(>0)`，且窗口对象别先析构 |
| Sobel/Scharr 全花屏 | 负值被 `saturate_cast<uchar>` 截 0 | 中间用 `CV_16S`/`CV_32F`，最后 `convertScaleAbs` |
| Canny 边缘碎成片 | 没先降噪 | 先 `GaussianBlur(Size(5,5),1.4)`，再 `Canny(thrLow, 2×thrLow)` |
| `calcHist` 显示空 | ranges 上界 exclusive | 用 `[0,256)` 而非 `[0,255]` |
| HSV `inRange` 不工作 | 写了 H∈[0,360) | OpenCV 8U HSV 里 **H 范围 0~179**；S/V 才是 0~255 |
| `cornerHarris` 响应看不见 | R 值极小（0.x） | `normalize` 到 [0,255] 再 `imshow`，或 `threshold` 后彩色标记 |
| SURF/SIFT 报 has no member | 没编 opencv_contrib | 自编译设 `OPENCV_EXTRA_MODULES_PATH` 指向 contrib/modules |
| Debevec HDR 4.13 崩断言 | `cv::CalibrateCRF` BGR reshape 已知 bug | 拆 BGR 逐通道跑再 Merge，或改用 **Mertens 曝光融合** |
| `*.exe` 找不到 data | CWD 不是 exe 目录 | `cd` 到 exe 目录，或用 `getImagePath()` 自带回退 |

## 8. 后续路线图

**短期（1~2 天/项）**：
- algorithms 补齐：完整 BM3D/VBM3D（需 contrib xphoto）、LIME 夜景、Skin Mask 美颜
- watermark：DCT 域水印 + 几何攻击鲁棒性自动测试
- beauty 接入 `models/face_detection_yunet_2023mar.onnx`（人脸框内磨皮 + 皮肤保护）
- HDR 加 Tonemap 对比图 + 参数扫描矩阵
- `algorithms/common` 加 `--headless` 跳过 imshow（CI 友好）

**中期（一周级，向 DNN 迁移）**：
- learn/L5 `digits_dnn.cpp` → ONNX Runtime 风格 UNet 降噪/美颜
- 人脸识别小流水线：YuNet→`alignCrop`→SFace 128 维→cosine
- 引入 G-API 流水线对照（参考 `mingw-build/samples/cpp/tutorial_code/gapi/face_beautification/`）

**长期（月级，ISP 小引擎）**：
- algorithms 升级"端到端小 ISP"：NV21→多帧降噪→HDR→夜景→美颜→水印→JPEG
- benchmarks 目录：固定数据集+参数 PSNR/SSIM/LOE/timing 表，支持回归
- 接入 CI（GitHub/Gitea Actions）：push 跑 benchmarks 输出 diff

## 9. 参考资源

| 类别 | 链接 / 位置 |
| --- | --- |
| 官方源码（自带） | [mingw-build/samples/cpp](mingw-build/samples/cpp) — 233 个 .cpp |
| 官方教程（自带） | [mingw-build/samples/cpp/tutorial_code](mingw-build/samples/cpp/tutorial_code) — 136 个 |
| OpenCV 在线文档 | <https://docs.opencv.org/4.x/> |
| 中文参考书 | 《OpenCV3编程入门》毛星云 · 电子工业出版社 |
| 自编 MinGW 编译教程 | <https://blog.huihut.com/2018/07/31/CompiledOpenCVWithMinGW64/> |

## 10. 维护硬约束 & 命名规范

### 10.1 维护约定

- ❌ `data/` 与 `mingw-build/` 是只读依赖，永不修改其中任何文件（含手动改名/增删）。
- ✅ 新增算法 demo 放 `algorithms/<模块名>/`，主源文件 `<模块名>.cpp`，至少还有 `README.md`。
- ✅ 新增顶层 demo 目录：只含 `*.cpp`（且含 `int main`）即被根 `CMakeLists.txt` 自动发现并编译，自动生成 `BUILD_<目录名>` 开关，无需改 CMake。
- ✅ 新增逐题练习放 `learn/`，遵循 L0~L5 分层与"主干 80%"主线（见 [learn/README.md](learn/README.md)）。
- ✅ 原理文档统一在 `docs/`，遵循 [docs/README.md 附录 A 结构章程](docs/README.md#附录-a-结构章程doc-charter)。
- ✅ 笔记式整理放 `notes/`，按主题分子目录，每目录配 README。
- ✅ 新增 ONNX 模型放 `models/`，必须同步写入 [models/README.md](models/README.md) 清单表（来源/用途/License）。

### 10.2 命名规范

> 仓库内所有目录与文件命名的唯一规范；旧写法不符的改齐。

| 元素 | 规则 | 正确示例 |
| --- | --- | --- |
| 文件夹 | `snake_case` | `image_process/` `harris_detect/` |
| C++ 源文件 | `snake_case` | `corner_harris.cpp` |
| C++ 头文件 | `snake_case` | `opencv_utils.h` `nv21_io.hpp` |
| Markdown | `snake_case`，固定名例外：`README.md`/`OUTLINE.md` | `ch01_core.md` |
| 构建/清理脚本 | `build.ps1`（含 `-Action clean`）/ `build.sh`（含 `clean`），均构建+清理合一 | `build.ps1`+`build.sh` |
| 数据/模型文件 | 保留原始/上游命名 | `face_detection_yunet_2023mar.onnx` |
| CMake target/选项 | 全大写+下划线；布尔开关前缀 `BUILD_` | `BUILD_LEARN` `ALGO_MODULE` `LEARN_LAYER` `BUILD_NOTES` |

**例外与保留**：
- 保护目录：`data/`、`mingw-build/`、`.idea/`、`.git/` 内任何文件不允许修改。
- 特殊固定名：`README.md`、`OUTLINE.md`（保留跳转，避免书签断链）、`CMakeLists.txt`。
- 根主程序源文件：`openCv.cpp`（原 `main.cpp`，产物 `build/main/openCv`）；根 target 名 `openCv` 不变。

**PR / 提交前检查清单**：
- [ ] 文件夹/文件是否 `snake_case`？避免 `camelCase`/`PascalCase`？
- [ ] Windows 单步大小写重命名是否走中转名两步（`Foo`→`_tmp_foo`→`foo`）？
- [ ] 新增 CMake 布尔开关是否 `BUILD_<SUBPROJECT>`？模块选择器用 `<MODULE>_MODULE`/`<PROJECT>_LAYER`？
- [ ] 同一概念在文档/源码/CMake 三处是否用同一词（如统一 `denoise_single`，不混用 `denoise`/`single_denoise`）？

> 规范即法律。凡与本节冲突的旧写法，一律以本节为准重写。
