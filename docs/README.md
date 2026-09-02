# OpenCV C++ 官方示例原理详解

> 系统梳理 `mingw-build/samples/cpp` 下 **233** 个 C++ 示例（根目录 97 + `tutorial_code` 136）所体现的*核心概念、算法原理、关键参数与模块功能*。
> **教学原则**：弱化编译，重点原理；重要参数必须讲清含义与调参效果。源码路径仅作对照索引。
> 本章是 `docs/` 系列的导航中枢，与原理总纲 [principles.md](./principles.md)、流程切片 [samples_flow.md](./samples_flow.md)、分章文件 `ch01`–`ch08` 配套。先在此建立"为什么这么做 / 参数如何影响结果"的知识骨架，再到分章对照源码。

---

## 四种入口（任选其一）

| 入口 | 适合 | 从哪开始 |
| --- | --- | --- |
| **按原理主题**（主入口） | 建立知识骨架 | [principles.md](./principles.md) → [samples_flow.md](./samples_flow.md) / `ch01`–`ch08` |
| **按任务找算法** | 「我要做光流/标定/拼接…」 | 下表「任务速查」→ 黄金示例 → 分章 |
| **按模块深耕** | 系统读完某一 OpenCV 模块 | `ch01`–`ch08` → [附录 B 示例清单](#附录-b-示例清单demo_map) |
| **动手练习** | 快速跑通最小可编译片段 | [learn/](../learn/README.md) → 对照官方 `samples/cpp` |

> **原理主入口**：[principles.md](./principles.md) 先建立"为什么 / 参数如何影响结果"的知识骨架，再对照流程篇与分章源码。

## 任务速查

| 我想搞… | 先读原理 | 黄金示例（对照源码） | 分章 |
| --- | --- | --- | --- |
| Mat / 像素 / 扫描 | principles §1 | `mat_the_basic_image_container.cpp`, `how_to_scan_images.cpp` | [ch01](./ch01_core.md) |
| 滤波 / 形态学 / 边缘 | §5–7 | `Smoothing.cpp`, `Morphology_1.cpp`, `CannyDetector_Demo.cpp` | [ch02](./ch02_imgproc.md) |
| 直方图 / 模板匹配 | §4 | `calcHist_Demo.cpp`, `MatchTemplate_Demo.cpp` | [ch02](./ch02_imgproc.md) |
| 阈值 / 距离变换 / 分水岭 | §12 | `Threshold.cpp`, `imageSegmentation.cpp`, `watershed.cpp` | [ch02](./ch02_imgproc.md) |
| 特征匹配与单应 | §8 | `SURF_FLANN_matching_homography_Demo.cpp` | [ch03](./ch03_features.md) |
| 轮廓与形状 | §11 | `findContours_demo.cpp` | [ch03](./ch03_features.md) |
| 光流 / 背景减除 | §13 | `lkdemo.cpp`, `optical_flow.cpp`, `bg_sub.cpp` | [ch04](./ch04_video.md) |
| SVM / 聚类 / PCA | §13.4 | `introduction_to_svm.cpp`, `digits_svm.cpp`, `kmeans.cpp` | [ch05](./ch05_ml.md) |
| 人脸 / 行人 / ArUco | §14 | `face_detect.cpp`, `peopledetect.cpp`, `detect_markers.cpp` | [ch06](./ch06_objdetect_photo.md) |
| 修复 / HDR / 无缝克隆 | §14.4 | `inpaint.cpp`, `hdr_imaging.cpp`, `cloning_demo.cpp` | [ch06](./ch06_objdetect_photo.md) |
| 标定 / 双目深度 / 拼接 | §15 | `calibration.cpp`, `stereo_match.cpp`, `stitching_detailed.cpp` | [ch07](./ch07_calib3d_stitching.md) |
| GUI / 视频 I/O / G-API / CUDA | §16 | `AddingImagesTrackbar.cpp`, `videocapture_basic.cpp`, `gpu-basics-similarity.cpp` | [ch08](./ch08_gui_gapi_gpu.md) |

## 章节总览

| 章节 | 文件 | 模块 | 内容主题 |
| --- | --- | --- | --- |
| 第 1 章 | [ch01_core.md](./ch01_core.md) | core / introduction / imgcodecs | Mat、ROI、扫描、DFT、FileStorage、并行与 SIMD、编解码 |
| 第 2 章 | [ch02_imgproc.md](./ch02_imgproc.md) | imgproc / imgtrans / hist | 滤波、形态学、阈值、边缘、Hough、几何、分水岭、直方图、颜色映射 |
| 第 3 章 | [ch03_features.md](./ch03_features.md) | features2d / ShapeDescriptors | 角点、SIFT/SURF/ORB/AKAZE…、匹配与单应、轮廓形状分析 |
| 第 4 章 | [ch04_video.md](./ch04_video.md) | video / TrackingMotion | LK/Farneback/DIS 光流、背景减除、MeanShift/CamShift、Kalman、角点跟踪 |
| 第 5 章 | [ch05_ml.md](./ch05_ml.md) | ml | SVM、PCA、k-means/EM、ANN、树模型、HOG 训练、dnn 桥梁 |
| 第 6 章 | [ch06_objdetect_photo.md](./ch06_objdetect_photo.md) | objdetect / photo | 级联、HOG 行人、ArUco、条码、GrabCut、inpaint、NPR、克隆、HDR |
| 第 7 章 | [ch07_calib3d_stitching.md](./ch07_calib3d_stitching.md) | calib3d / stitching | 张正友标定、双目、对极几何、立体匹配、拼接、PnP、Stitcher |
| 第 8 章 | [ch08_gui_gapi_gpu.md](./ch08_gui_gapi_gpu.md) | highgui / videoio / gapi / gpu | GUI 回调、绘图、Video I/O、G-API、CUDA、snippets |

## 推荐阅读顺序（零基础）

1. [principles.md](./principles.md) §1–7（像素→滤波→边缘）
2. [ch01](./ch01_core.md) Mat 黄金示例 + [ch02](./ch02_imgproc.md) 平滑/Canny
3. principles §8–12 + [ch03](./ch03_features.md) 单应匹配
4. principles §13–16 + 按兴趣进 [ch04](./ch04_video.md)–[ch08](./ch08_gui_gapi_gpu.md)
5. 根目录录屏切片到端链路：[samples_flow.md](./samples_flow.md)；文件归属用 [附录 B 示例清单](#附录-b-示例清单demo_map)

## 文件集合

| 文件 | 角色 |
| --- | --- |
| [learn/](../learn/README.md) | 118 个最小可运行练习（与分章互链） |
| [principles.md](./principles.md) | 原理总纲（主入口） |
| [samples_flow.md](./samples_flow.md) | 97 根目录录屏流程切片 + tutorial_code 主题索引 |
| [cheatsheet.md](./cheatsheet.md) | **一页 API 速查**：参数推荐取值 + 必记陷阱（写代码时手边必备） |
| [faq_troubleshooting.md](./faq_troubleshooting.md) | **排障 FAQ**：编译链接 / 运行时 / 算法调参 / 数值异常 四类 30 条 |
| [api_index.md](./api_index.md) | **四维映射**：learn 118 题 ↔ notes ↔ algorithms 17 模块 ↔ 官方 233 demo |
| [legacy_setup_notes.md](./legacy_setup_notes.md) | 早期环境搭建草稿归档（记录 MinGW 选型决策：为什么选 sjlj） |
| `README.md`（本文件） | 导航入口 + [结构章程](#附录-a-结构章程doc-charter)（附录 A） + [233 文件清单](#附录-b-示例清单demo_map)（附录 B） |
| `ch01`–`ch08` | 逐文件：概述 / 原理 / API / 流程 / **参数** / 对比 / 注意 / 应用 |

### 三本"工具书"该怎么选

同个问题在三份文档里都有，区别是**你处于什么状态**：

| 你的状态 | 打开 |
|---------|------|
| 正在写代码，忘了参数怎么填 / 类型用什么 | [cheatsheet.md](./cheatsheet.md) |
| 代码跑不起来或结果不对 | [faq_troubleshooting.md](./faq_troubleshooting.md) |
| 想找"实现 X 的代码在哪" | [api_index.md](./api_index.md) |
| 想搞懂 X 的数学原理 | [principles.md](./principles.md) + 对应 `ch0X` |

## 覆盖边界

- **dnn**：本树仅 `digits_lenet.cpp` 作桥梁；完整深度学习样例需 Model Zoo。
- **tracking 模块**（CSRT/KCF 等）：本 samples 树无官方样例。
- **CUDA Thrust**：`gpu-thrust-interop` 为 `.cu`，未计入 233 个 `.cpp`。
- **UMat / OpenCL**：分散在个别 demo，无独立教程章。

## 单示例字段（阅读时关注）

每个 `###` 示例固定八段：`功能概述` → `核心原理`（含 30 秒心智模型）→ `关键 API` → `处理流程` → **`参数说明`**（含含义 / 范围 / 调大调小）→ `关联与对比` → `注意事项` → `应用场景`。详见 [附录 A §4 模板](#4-单示例固定字段模板强制)。

---

## 附录 A 结构章程（Doc Charter）

> 本节是 `docs/` 下所有 Markdown 文档的*唯一结构规范*。任何章节的增删改都必须遵守本章程，确保 8 章正文 + 原理篇 + 流程篇 + 索引之间层级一致、编号连贯、模板统一、可一键校验。
> 教学约束：**弱化编译，重点原理，重要参数必须讲透**。文档回答"这个 demo 在算什么、为什么这样算、关键参数如何改变结果"。

### 1. 文件集合与定位

共 11 份文件：总索引与规范 `README.md`（本文件）、原理总纲 `principles.md`、流程切片 `samples_flow.md`、分章正文 `ch01`–`ch08`。
**唯一权威**：文件角色表、阅读顺序、章节总览、任务与黄金示例速查见上文（入口部分，不写正文）；233 文件 → 章节映射见 [附录 B 示例清单](#附录-b-示例清单demo_map)。本附录只规定"怎么写"，不重复"有什么"。

### 2. 标题层级（强制）

整库可用四级标题，语义固定，**禁止错位**：

| 层级 | 语义 | 格式 | 示例 |
| --- | --- | --- | --- |
| `#` H1 | **章标题**，全文唯一 | `# 第 N 章 <标题>` | `# 第 1 章 核心模块…` |
| `##` H2 | **分组/节**（章节导言、主题分组、本章小结） | `## N.M <分组名>` | `## 1.0 章节导言`、`## 1.2 core 模块` |
| `###` H3 | **单个示例文件**（或导言/小结内的子节） | `### N.M.K <file.cpp> —— <一句话>` | `### 2.4.3 CannyDetector_Demo.cpp —— …` |
| `####` H4 | **示例内固定字段**（见 §4 模板） | `#### <字段名>` | `#### 核心原理` |

**铁律**：
1. 每个 `.md` 文件**恰好一个** `#`（H1）。任何次级分区一律是 `##`，不得用 `#`。
2. 层级必须连续：`###` 必须挂在某个 `##` 下；`####` 必须挂在某个 `###` 下。
3. 不允许出现 `#####`。
4. 单个 `.cpp` 示例**必须**是 `###`，不得升格为 `##`。

### 3. 编号规则（强制）

采用统一的三段式编号 `N.M.K`：

- **N** = 章号（1–8），与文件名 `chNN_` 对应；原理篇 / 流程篇 / 清单分别用 `P.*` / `F.*` / `M.*` 内部自编。
- **M** = 分组/节序号，从 `0` 起：`0` 固定为「章节导言」，`1,2,3…` 为各主题分组。
- **K** = 组内示例序号，从 `1` 起。

**要求**：
- 同章内主题分组 `M` 连续不跳号（导言 `0` → 主题 `1…n` → 小结；小结可用 `N.9` 或 `N.(n+1)`）。
- 同组内示例 `K` 连续不跳号。
- 附录用 `## 附录 A` 等形式，不与示例 `K` 冲突。

### 4. 单示例固定字段模板（强制）

每个 `###` 示例内部，*统一使用以下固定 `####` 字段*，顺序固定、可缺不可增、缺项写「（无）」或整段省略。

```
### N.M.K <file.cpp> —— <一句话功能>
> **源文件**：`samples/cpp/...` · **所属模块**：<module> · **示例类型**：完整流程|snippet|多文件工程

#### 功能概述
（这个示例做什么、解决什么问题，2–4 句）

#### 核心原理
（开头 1–3 句「30 秒心智模型」；随后数学/算法，可用 LaTeX 与 mermaid）

#### 关键 API
（主要函数/类 + 一句话释义；必须能在源码中找到）

#### 处理流程
（输入 → 预处理 → 核心算法 → 结果输出，标注函数）

#### 参数说明
（见 §4.1：真正影响结果的参数表）

#### 关联与对比
（与同章/跨章相关示例的关系、异同）

#### 注意事项
（常见坑、精度/性能权衡；硬件或模型依赖一句话点明）

#### 应用场景
（典型落地场景）
```

#### 4.1 参数说明（强制写透）

`#### 参数说明` 必须覆盖该示例*真正影响结果**的算法/API 参数。推荐表格列：

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |

- CLI 若对应算法量（如标定棋盘宽高、Canny 高低阈值），写入本表**释义**，不写成命令行教程。
- 无独立超参：写「（无）」或「结果主要由输入数据决定」。
- **禁止**保留「无提取到的参数片段」等空洞占位。

#### 4.2 教学约束

- **弱化编译**：禁止新增「编译」「运行命令」「CMake target」「环境搭建」字段或专章。
- **重点原理**：`#### 核心原理` 回答"在算什么、为什么"；黄金示例写透心智模型与关键公式。
- **源码对照**：路径仅作索引；API/默认值以 `mingw-build/samples/cpp` 源码为准。

**字段规范**：
- 字段名只能是上表所列（blockquote 元信息 + 8 个 `####`），禁止自创标题字段。
- `源文件` / `所属模块` / `示例类型` 优先用 `>` blockquote 单行呈现。
- 字段层级固定 `####`。

### 5. 语言与格式约定

- 正文简体中文；API、类名、函数名、文件名、代码标识符保留英文并用反引号（如 `cv::Mat`、`findHomography`）。
- 路径统一用 `samples/cpp/...` 相对前缀；绝对路径仅在本附录出现一次作说明：源码根为仓库内 `mingw-build/samples/cpp`。
- 数学用 `$...$` / `$$...$$`；原理图用 mermaid。
- 交叉引用用相对链接：`[第 2 章边缘与梯度](./ch02_imgproc.md#24-边缘与梯度edge--gradient)`。
- 章节导言应给出*概念阅读顺序*与先修关系，不写实操清单。

### 6. 维护与扩展 SOP

1. **新增示例**：在对应 `## N.M` 末尾追加 `### N.M.K`，套用 §4；同步更新 [附录 B](#附录-b-示例清单demo_map)、`samples_flow.md`（根目录）或主题索引、`principles.md` 关联。
2. **新增章节**：仅当新增 OpenCV 大模块时新建 `ch09_*.md`，并在 `README.md`（[附录 B](#附录-b-示例清单demo_map)）登记。
3. **禁止**：跨文内写「本章导航 / 返回目录」；禁止把单示例升格为 `##`。
4. **校验**：改动后运行 `python docs/scripts/normalize_docs.py --check`，校验 H1 唯一、层级连续、编号无跳号、字段合法；通过后再提交。

> 章程即「法律」。凡与本附录冲突的旧写法，一律以本附录为准重写。

---

## 附录 B 示例清单（demo_map）

> 覆盖 `mingw-build/samples/cpp`：*根目录 97* + **tutorial_code 136** = **233** 个 `.cpp`。
> 用途：按文件查所属章节与示例类型。教学以原理为主；路径仅作源码对照索引。
> 类型：`完整流程` = 端到端演示；`snippet` = API/文档片段；`多文件工程` = 多源文件教程。
> 章节总览、任务速查与每章黄金示例见上文；根目录流程切片见 [samples_flow.md](./samples_flow.md)。

### M.1 根目录示例（97）

| 文件 | 类型 | 主要章节 |
| --- | --- | --- |
| `3calibration.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `application_trace.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `aruco_dict_utils.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `asift.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `audio_spectrogram.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `barcode.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `bgfg_segm.cpp` | 完整流程 | [`ch04_video.md`](./ch04_video.md) |
| `calibration.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `camshiftdemo.cpp` | 完整流程 | [`ch04_video.md`](./ch04_video.md) |
| `cloning_demo.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `cloning_gui.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `connected_components.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `contours2.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `convexhull.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `cout_mat.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `create_mask.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `dbt_face_detection.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `delaunay2.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `demhist.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `detect_blob.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `detect_mser.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `dft.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `digits_lenet.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `digits_svm.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `dis_opticalflow.cpp` | 完整流程 | [`ch04_video.md`](./ch04_video.md) |
| `distrans.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `drawing.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `edge.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `ela.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `em.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `epipolar_lines.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `essential_mat_reconstr.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `facedetect.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `facial_features.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `falsecolor.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `fback.cpp` | 完整流程 | [`ch04_video.md`](./ch04_video.md) |
| `ffilldemo.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `fitellipse.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `flann_search_dataset.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `gauge.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `grabcut.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `image_alignment.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `imagelist_creator.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `imagelist_reader.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `imgcodecs_jpeg.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `inpaint.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `intelligent_scissors.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `intersectExample.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `kalman.cpp` | 完整流程 | [`ch04_video.md`](./ch04_video.md) |
| `kmeans.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `laplace.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `letter_recog.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `lkdemo.cpp` | 完整流程 | [`ch04_video.md`](./ch04_video.md) |
| `logistic_regression.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `lsd_lines.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `mask_tmpl.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `matchmethod_orb_akaze_brisk.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `minarea.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `morphology2.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `neural_network.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `npr_demo.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `opencv_version.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `pca.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `peopledetect.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `phase_corr.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `points_classifier.cpp` | 完整流程 | [`ch04_video.md`](./ch04_video.md) |
| `polar_transforms.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `qrcode.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `segment_objects.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `select3dobj.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `simd_basic.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `smiledetect.cpp` | 完整流程 | [`ch06_objdetect_photo.md`](./ch06_objdetect_photo.md) |
| `squares.cpp` | 完整流程 | [`ch03_features.md`](./ch03_features.md) |
| `stereo_calib.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `stereo_match.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `stitching.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `stitching_detailed.cpp` | 完整流程 | [`ch07_calib3d_stitching.md`](./ch07_calib3d_stitching.md) |
| `text_skewness_correction.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `train_HOG.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `train_svmsgd.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `travelsalesman.cpp` | 完整流程 | [`ch01_core.md`](./ch01_core.md) |
| `tree_engine.cpp` | 完整流程 | [`ch05_ml.md`](./ch05_ml.md) |
| `videocapture_audio.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_audio_combination.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_basic.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_camera.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_gphoto2_autofocus.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_gstreamer_pipeline.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_image_sequence.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_microphone.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_obsensor.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_openni.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_realsense.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videocapture_starter.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `videowriter_basic.cpp` | 完整流程 | [`ch08_gui_gapi_gpu.md`](./ch08_gui_gapi_gpu.md) |
| `warpPerspective_demo.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |
| `watershed.cpp` | 完整流程 | [`ch02_imgproc.md`](./ch02_imgproc.md) |

### M.2 learn 新增练习（026 扩编）

| 练习 | 官方对照 | 归章 |
| --- | --- | --- |
| `L0_intro/07_opencv_version.cpp` | `opencv_version.cpp` | ch01 / ch08 |
| `L0_intro/08_videowriter.cpp` | `videowriter_basic.cpp` | ch08 |
| `L2_imgproc/32_phase_correlate.cpp` | `phase_corr.cpp` | ch02 |
| `L2_imgproc/33_contrast_brightness_trackbar.cpp` | `changing_contrast_brightness_image.cpp` | ch02 |
| `L2_imgproc/34_anisotropic_segmentation.cpp` | `anisotropic_image_segmentation.cpp` | ch02 |
| `L2_imgproc/35_motion_deblur.cpp` | `motion_deblur_filter.cpp` | ch02 |
| `L2_imgproc/36_generalized_hough.cpp` | `generalizedHoughTransform.cpp` | ch02 |
| `L3_features_video/23_mser.cpp` | `detect_mser.cpp` | ch03 |
| `L3_features_video/24_blob_lsd.cpp` | `detect_blob.cpp`, `lsd_lines.cpp` | ch03 |
| `L3_features_video/25_homography_decompose.cpp` | `decompose_homography.cpp` | ch03 |
| `L3_features_video/26_lk_stepwise.cpp` | `optical_flow.cpp` | ch04 |
| `L4_detect_calib/21_charuco_detect.cpp` | `detect_board_charuco.cpp` | ch06 / ch07 |
| `L5_ml_gapi/13_digits_dnn.cpp` | `digits_lenet.cpp` | ch05 |
| `L5_ml_gapi/14_hog_svm_train.cpp` | `train_HOG.cpp` | ch05 |
| `L5_ml_gapi/15_gapi_pipeline.cpp` | G-API 应用 demo（简化） | ch08 |

> 完整 118 题索引见 [learn/](../learn/README.md)；本节仅列本次新增。

### M.3 tutorial_code（136）

| 相对路径 | 类型 | 默认章节 | 文档提及 |
| --- | --- | --- | --- |
| `tutorial_code/calib3d/camera_calibration/camera_calibration.cpp` | 完整流程 | `ch07_*.md` | `ch07_calib3d_stitching.md` |
| `tutorial_code/calib3d/real_time_pose_estimation/src/CsvReader.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/CsvWriter.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/main_detection.cpp` | 多文件工程 | `ch07_*.md` | `ch07_calib3d_stitching.md` |
| `tutorial_code/calib3d/real_time_pose_estimation/src/main_registration.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/Mesh.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/Model.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/ModelRegistration.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/PnPProblem.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/RobustMatcher.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/calib3d/real_time_pose_estimation/src/Utils.cpp` | 多文件工程 | `ch07_*.md` | — |
| `tutorial_code/core/AddingImages/AddingImages.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/discrete_fourier_transform/discrete_fourier_transform.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/file_input_output/file_input_output.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/how_to_scan_images/how_to_scan_images.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/how_to_use_OpenCV_parallel_for_/how_to_use_OpenCV_parallel_for_.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/how_to_use_OpenCV_parallel_for_/how_to_use_OpenCV_parallel_for_new.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/mat_mask_operations/mat_mask_operations.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/mat_operations/mat_operations.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/mat_the_basic_image_container/mat_the_basic_image_container.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/parallel_backend/example-openmp.cpp` | 多文件工程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/parallel_backend/example-tbb.cpp` | 多文件工程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/core/univ_intrin/univ_intrin.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/features2D/AKAZE_match.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/AKAZE_tracking/planar_tracking.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/feature_description/SURF_matching_Demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/feature_detection/SURF_detection_Demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/feature_flann_matcher/SURF_FLANN_matching_Demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/feature_homography/SURF_FLNN_matching_homography_Demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/Homography/decompose_homography.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/Homography/homography_from_camera_displacement.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/Homography/panorama_stitching_rotating_camera.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/Homography/perspective_correction.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/features2D/Homography/pose_from_homography.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/gapi/age_gender_emotion_recognition/age_gender_emotion_recognition.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gapi/doc_snippets/api_ref_snippets.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gapi/doc_snippets/dynamic_graph_snippets.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gapi/doc_snippets/kernel_api_snippets.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gapi/face_beautification/face_beautification.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gapi/porting_anisotropic_image_segmentation/porting_anisotropic_image_segmentation_gapi.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gapi/porting_anisotropic_image_segmentation/porting_anisotropic_image_segmentation_gapi_fluid.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gapi/security_barrier_camera/security_barrier_camera.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/gpu/gpu-basics-similarity/gpu-basics-similarity.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/HighGUI/AddingImagesTrackbar.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/HighGUI/BasicLinearTransformsTrackbar.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/Histograms_Matching/calcBackProject_Demo1.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/Histograms_Matching/calcBackProject_Demo2.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/Histograms_Matching/calcHist_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/Histograms_Matching/compareHist_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/Histograms_Matching/EqualizeHist_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/Histograms_Matching/MatchTemplate_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/imgcodecs/animations.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/imgcodecs/GDAL_IO/gdal-image.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/ImgProc/anisotropic_image_segmentation/anisotropic_image_segmentation.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/basic_drawing/Drawing_1.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/basic_drawing/Drawing_2.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/BasicLinearTransforms.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/changing_contrast_brightness_image/changing_contrast_brightness_image.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/HitMiss/HitMiss.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/morph_lines_detection/Morphology_3.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/Morphology_1.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/Morphology_2.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/motion_deblur_filter/motion_deblur_filter.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/out_of_focus_deblur_filter/out_of_focus_deblur_filter.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/periodic_noise_removing_filter/periodic_noise_removing_filter.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/Pyramids/Pyramids.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/Smoothing/Smoothing.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/Threshold.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgProc/Threshold_inRange.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/CannyDetector_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/copyMakeBorder_demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/filter2D_demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/generalizedHoughTransform.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/Geometric_Transforms_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/HoughCircle_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/houghcircles.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/houghlines.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/HoughLines_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/imageSegmentation.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/Laplace_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/Remap_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/ImgTrans/Sobel_Demo.cpp` | 完整流程 | `ch02_*.md` | `ch02_imgproc.md` |
| `tutorial_code/introduction/display_image/display_image.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/introduction/documentation/documentation.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/introduction/windows_visual_studio_opencv/introduction_windows_vs.cpp` | 完整流程 | `ch01_*.md` | `ch01_core.md` |
| `tutorial_code/ml/introduction_to_pca/introduction_to_pca.cpp` | 完整流程 | `ch05_*.md` | `ch05_ml.md` |
| `tutorial_code/ml/introduction_to_svm/introduction_to_svm.cpp` | 完整流程 | `ch05_*.md` | `ch05_ml.md` |
| `tutorial_code/ml/non_linear_svms/non_linear_svms.cpp` | 完整流程 | `ch05_*.md` | `ch05_ml.md` |
| `tutorial_code/objectDetection/calibrate_camera.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/calibrate_camera_charuco.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/create_board.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/create_board_charuco.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/create_diamond.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/create_marker.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/detect_board.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/detect_board_charuco.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/detect_diamonds.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/detect_markers.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/objectDetection/objectDetection.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/photo/decolorization/decolor.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/photo/hdr_imaging/hdr_imaging.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/photo/non_photorealistic_rendering/npr_demo.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/photo/seamless_cloning/cloning_demo.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/photo/seamless_cloning/cloning_gui.cpp` | 完整流程 | `ch06_*.md` | `ch06_objdetect_photo.md` |
| `tutorial_code/ShapeDescriptors/findContours_demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/ShapeDescriptors/generalContours_demo1.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/ShapeDescriptors/generalContours_demo2.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/ShapeDescriptors/hull_demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/ShapeDescriptors/moments_demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/ShapeDescriptors/pointPolygonTest_demo.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |
| `tutorial_code/snippets/core_mat_checkVector.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/core_merge.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/core_reduce.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/core_split.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/core_various.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgcodecs_imwrite.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgproc_applyColorMap.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgproc_calc_hist.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgproc_drawContours.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgproc_HoughLinesCircles.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgproc_HoughLinesP.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgproc_HoughLinesPointSet.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/snippets/imgproc_segmentation.cpp` | snippet | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/TrackingMotion/cornerDetector_Demo.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/TrackingMotion/cornerHarris_Demo.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/TrackingMotion/cornerSubPix_Demo.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/TrackingMotion/goodFeaturesToTrack_Demo.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/video/bg_sub.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/video/meanshift/camshift.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/video/meanshift/meanshift.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/video/optical_flow/optical_flow.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/video/optical_flow/optical_flow_dense.cpp` | 完整流程 | `ch04_*.md` | `ch04_video.md` |
| `tutorial_code/videoio/openni_orbbec_astra/openni_orbbec_astra.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/videoio/video-input-psnr-ssim/video-input-psnr-ssim.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/videoio/video-write/video-write.cpp` | 完整流程 | `ch08_*.md` | `ch08_gui_gapi_gpu.md` |
| `tutorial_code/xfeatures2D/LATCH_match.cpp` | 完整流程 | `ch03_*.md` | `ch03_features.md` |

> 覆盖边界（dnn / tracking / CUDA Thrust / UMat 需独立成篇）见上文 [覆盖边界](#覆盖边界)。

---

> 共解析 **233** 个 C++ 示例；学习目标是更快掌握 OpenCV **原理与操作语义**，而不是把文档写成编译手册。
