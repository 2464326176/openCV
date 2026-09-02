# 第 8 章 GUI、VideoIO、G-API 与 GPU：交互、数据流和异构执行

> 本章以仓库内 `mingw-build/samples/cpp` 的官方示例为源码基准。主线是“窗口交互 → 多媒体 IO → 声明式数据流 → 异构加速”，不包含编译与环境配置。

***

## 8.0 章节导言

前面的章节解决“给定一张图怎么算”，本章解决“图从哪来、怎么让用户调参、怎么把一串算法组织起来并跑得快”。四组能力各司其职：

- **HighGUI**：窗口、滑动条与绘图原语，把算法参数从“改代码重编译”变成“拖滑块看结果”；

- **VideoIO**：统一摄像头、视频文件、图像序列与音轨的读写抽象，是流式应用的入口；

- **G-API**：把多个算子声明为一张计算图，让“图怎么算”与“用什么后端算”解耦，并支持编译期优化与流式流水线；

- **GPU/CUDA**：把计算密集型算子搬到显存执行，核心是显存分配昂贵、复用缓冲与异步流。

**30 秒心智模型**：`VideoCapture`/`VideoWriter` 是 IO 边界，HighGUI 是“人—算法”界面，G-API 是把算子串成图的“装配层”，CUDA 是图里某些算子的“加速器”。四者可在同一程序中叠加：摄像头取流 → G-API 图（部分算子落到 GPU）→ 窗口 + 滑动条展示与调参。

概念阅读顺序：

1. `AddingImagesTrackbar.cpp`、`BasicLinearTransformsTrackbar.cpp`：HighGUI 交互骨架（窗口 + 滑动条回调）；
2. `videocapture_basic.cpp`、`videowriter_basic.cpp`、`video-input-psnr-ssim.cpp`：VideoIO 读写闭环；
3. `api_ref_snippets.cpp`、`dynamic_graph_snippets.cpp`、`kernel_api_snippets.cpp`：G-API 图构造、执行与自定义 kernel；
4. `age_gender_emotion_recognition.cpp`、`security_barrier_camera.cpp`：G-API 多模型推理流水线；
5. `gpu-basics-similarity.cpp`：把同一条 PSNR/MSSIM 算法分别在 CPU、基础 CUDA、缓冲复用 CUDA 上计时对比。

滑动条与绘图沿用[第 1 章核心模块](./ch01_core.md)、[第 2 章图像处理](./ch02_imgproc.md)的 `Mat` 模型；视频帧处理与背景建模见[第 4 章视频分析](./ch04_video.md)。

***

## 8.1 HighGUI

### 8.1.1 `AddingImagesTrackbar.cpp` —— 滑动条控制双图混合

> **源文件**：`samples/cpp/tutorial_code/HighGUI/AddingImagesTrackbar.cpp` ｜ **所属模块**：`highgui` ｜ **示例类型**：`完整流程`

#### 功能概述

载入两张大小、类型相同的图片，在窗口中用一条滑动条控制两张图的线性混合权重，滑块变化即时触发重绘，展示 `createTrackbar` + 回调函数的交互范式。

#### 核心原理

**30 秒心智模型**：混合就是“两杯颜料各倒多少”。一条 `[0, 100]` 的滑块只代表 `alpha`，`beta = 1 - alpha` 由回调自动补全，保证两张图权重始终归一。

OpenCV 的加权和由 `addWeighted` 完成：

$$
dst(I) = \alpha \cdot src\_1(I) + \beta \cdot src\_2(I) + \gamma.
$$

本示例取 $\beta=1-\alpha$、$\gamma=0$，因此滑块在左端是纯 `src2`、右端是纯 `src1`，中间平滑过渡。回调函数通过 `userdata` 指针传入上下文，这里只用了全局变量，因此该参数传 `nullptr`。

#### 关键 API

- `namedWindow`：预先创建带标题的窗口；

- `createTrackbar`：绑定滑块与回调，`&alpha_slider` 为共享的滑块值；

- `addWeighted`：按 `alpha/beta/gamma` 计算加权和；

- `imshow`、`waitKey`：显示并阻塞等待按键。

#### 处理流程

读取 `src1/src2` → `namedWindow` 建窗 → `snprintf` 生成滑动条标题 → `createTrackbar` 注册回调 → 手动调用一次回调显示初始帧 → `waitKey(0)` 进入事件循环。

#### 参数说明

| 参数                 | 含义          | 典型范围/默认             | 调大/调小会怎样                                  |
| ------------------ | ----------- | ------------------- | ----------------------------------------- |
| `alpha_slider_max` | 滑块上限        | `100`               | 决定滑块的调节粒度；越大每个刻度的混合步长越小                   |
| `alpha`            | 第一张图权重      | `0`–`1`（由滑块/100 得到） | 接近 `1` 时结果几乎全为 `src1`；接近 `0` 时几乎全为 `src2` |
| `beta`             | 第二张图权重      | `1 - alpha`         | 无需单独调，保证两权重之和恒为 `1`                       |
| `gamma`            | 加性常数        | `0.0`               | 非零会给每个像素加常数亮度偏移                           |
| 输入尺寸               | 两张图必须同尺寸同类型 | 任意                  | 不同尺寸会因 ROI 越界或类型不匹配报错                     |

#### 关联与对比

与 8.1.2 同属“滑动条 + 回调”骨架，区别是这里混合两幅图，那里单图亮度/对比度变换。`addWeighted` 是 `src1*src2`、`+` 等逐元素运算的推广，在[第 1 章](./ch01_core.md)的算术运算中也可见。

#### 注意事项

- 两张输入图必须同尺寸、同通道数，否则 `addWeighted` 抛异常；

- 滑块值回调里读取的是整数 `pos`，换算权重时注意浮点除法，避免整型截断；

- `waitKey(0)` 会阻塞主线程，适合一次性演示；实时循环中应换成短超时（如 `waitKey(5)`）。

#### 应用场景

图片融合预览、蒙版强度调节、多源可视化对比、任何需要“拖动参数立即看效果”的调参工具。

### 8.1.2 `BasicLinearTransformsTrackbar.cpp` —— 交互调节亮度与对比度

> **源文件**：`samples/cpp/tutorial_code/HighGUI/BasicLinearTransformsTrackbar.cpp` ｜ **所属模块**：`highgui` ｜ **示例类型**：`完整流程`

#### 功能概述

在“New Image”窗口上放置两条滑动条，分别控制对比度（`alpha`）与亮度（`beta`），用逐像素线性变换加饱和裁剪生成新图，展示多滑动条共享一个回调的交互方式。

#### 核心原理

**30 秒心智模型**：对比度是“拉伸/压缩灰度差”，亮度是“整体加/减一个常数”。两者的作用顺序（先乘后加）决定最终效果，裁剪负责把越界值拉回 `[0,255]`。

逐像素变换为

$$
dst(x,y) = \operatorname{saturate}\big(\alpha \cdot src(x,y) + \beta\big),
$$

其中 $\alpha$ 为增益、$\beta$ 为偏移。`saturate_cast<uchar>` 把计算结果钳位到 `[0,255]`：负值变 `0`、超过 `255` 变 `255`，避免无符号 8 位溢出环绕。与直接用 `convertTo` 的 `alpha,beta` 参数相比，这里手动写循环，便于看清逐通道语义。

#### 关键 API

- `createTrackbar`：为“Contrast”与“Brightness”各注册一条滑块，共享同一个 `on_trackbar` 回调；

- `saturate_cast<uchar>`：饱和裁剪，防止溢出；

- `Mat::at<Vec3b>`：逐像素逐通道读写；

- `namedWindow`、`imshow`、`waitKey`：窗口显示与事件循环。

#### 处理流程

读取图片 → 初始化 `alpha=1, beta=0` → 创建“Original Image”与“New Image”两窗口 → 注册两条滑动条 → 滑块变化触发回调，逐像素计算并显示结果。

#### 参数说明

| 参数          | 含义      | 典型范围/默认             | 调大/调小会怎样                            |
| ----------- | ------- | ------------------- | ----------------------------------- |
| `alpha`     | 对比度增益   | 滑块 `[0,5]`，默认 `1`   | 大于 `1` 拉开明暗差（对比度增强），小于 `1` 压向均值（变灰） |
| `alpha_max` | 对比度滑块上限 | `5`                 | 决定可调的拉伸幅度上限                         |
| `beta`      | 亮度偏移    | 滑块 `[0,125]`，默认 `0` | 越大整图越亮；过大会使高光区饱和成纯白                 |
| `beta_max`  | 亮度滑块上限  | `125`               | 决定可加的偏移上限                           |
| 通道数         | 逐通道处理   | 固定 3（`Vec3b`）       | 灰度图需改用 `Vec3b` 之外的访问方式              |

#### 关联与对比

与 8.1.1 共用“回调驱动重算”结构；单图线性增强的批量版本是 `Mat::convertTo` 与 `cv::addWeighted`。该示例把 `alpha/beta` 的作用讲透后，可与[第 2 章](./ch02_imgproc.md)直方图均衡对比：前者是全局线性映射，后者是非线性重分布。

#### 注意事项

- 先乘后加的顺序不能颠倒：先加后乘会放大偏移量；

- 必须用 `saturate_cast`，否则 8 位溢出会让暗部翻到亮部；

- 循环用 `Vec3b` 假定三通道彩色图，对灰度图会越界读取。

#### 应用场景

图像后处理预览、显示器/打印的色彩与亮度校正、交互式对比度亮度调参工具。

***

## 8.2 Drawing 与 snippets

### 8.2.1 `drawing.cpp` —— 基础绘图原语总览

> **源文件**：`samples/cpp/drawing.cpp` ｜ **所属模块**：`imgproc`（绘图）+ `core` ｜ **示例类型**：`完整流程`

#### 功能概述

在一张黑底画布上用随机颜色依次绘制线段/箭头、矩形/标记、椭圆、多边形与文字，逐个阶段演示 OpenCV 常用绘图原语的参数（坐标、颜色、线宽、线型）。

#### 核心原理

**30 秒心智模型**：绘图就是在 `Mat` 上按“先画的在下、后画的在上”覆盖写像素；所有绘图函数共享 `color`、`thickness`、`lineType` 三件套，`thickness < 0` 表示实心填充。

`randomColor` 用 `RNG` 的低 24 位构造 BGR 颜色，保证每帧颜色随机。`LINE_AA` 通过抗锯齿在像素间做灰度过渡，比 `LINE_8` 边缘更平滑但更慢。`ellipse` 的起始/终止角以角度计，绘制椭圆弧；`drawMarker` 用预定义标记类型画交叉/菱形等记号。

#### 关键 API

- `line`、`arrowedLine`：直线与带箭头线段；

- `rectangle`、`drawMarker`：矩形与标记；

- `ellipse`：旋转椭圆与椭圆弧；

- `polylines`、`fillPoly`：折线与填充多边形；

- `putText`、`circle`：文字与圆；

- `RNG`、`Mat::zeros`、`imshow`、`waitKey`：随机源、画布与显示。

#### 处理流程

创建 `1000x700` 黑底 `CV_8UC3` → 分四个阶段随机绘制（线/箭头、矩形/标记、椭圆、多边形与文字）→ 每步 `imshow` + `waitKey(5)` 逐步播放 → 按键退出。

#### 参数说明

| 参数               | 含义       | 典型范围/默认                          | 调大/调小会怎样               |
| ---------------- | -------- | -------------------------------- | ---------------------- |
| `thickness`      | 线宽/填充    | `>=1` 描边，`<0`（如 `-1`/`FILLED`）实心 | 越大线越粗；负值从描边切换为填充       |
| `lineType`       | 光栅化类型    | `LINE_8`/`LINE_AA`               | `LINE_AA` 抗锯齿更平滑，但开销略高 |
| `NUMBER`/`DELAY` | 每阶段数量与间隔 | `100`/`5` 毫秒                     | 数量越多演示越久；间隔越大播放越慢      |
| `color`          | BGR 三元组  | 每通道 `0–255`                      | 决定绘制颜色；随机值时每次运行不同      |
| 起止角度             | 椭圆弧范围    | 任意角度                             | 决定圆弧覆盖范围，影响椭圆是否闭合      |

#### 关联与对比

绘图原语是其他所有示例的输出层：如 8.2.10 画轮廓、8.2.11/8.2.12 画检测结果、8.3.3 画音视频窗口。`fillPoly` 与 `drawContours` 的填充语义一致，见 8.2.10。

#### 注意事项

- 坐标必须落在画布范围内，越界会被裁剪，但极端值可能引发断言；

- 实心填充用负线宽而非 `0`；

- `putText` 中文字符需使用支持 CJK 的字体与底层渲染，否则显示为方块。

#### 应用场景

检测/跟踪结果叠加、调试可视化、简单图表绘制、标注工具与演示动效。

### 8.2.2 `core_mat_checkVector.cpp` —— 校验 Mat 的向量解释

> **源文件**：`samples/cpp/tutorial_code/snippets/core_mat_checkVector.cpp` ｜ **所属模块**：`core` ｜ **示例类型**：`snippet`

#### 功能概述

用 `Mat::checkVector` 判断一个 `Mat` 能否被解释为“每元素含固定通道数的一维向量”，并演示行向量、列向量与三维 `Mat` 的判定规则。

#### 核心原理

**30 秒心智模型**：很多算法要求输入是“N 个元素，每个元素 C 个通道”。`checkVector(C)` 回答“这张图能摊平成 N×C 的一维结构吗”，不能就返回 `-1`。

`checkVector(elemChannels)` 检查 `Mat` 是单行、单列，还是 3D 且仅单通道，并据此把“行数×列数”折叠成元素个数。例如 `20x1` 的 `CV_32FC2` 是 20 个双通道元素（返回 20）；`20x2` 的 `CV_32FC1` 无法解释为单通道向量（返回 `-1`），但可解释为 20 个双通道元素（返回 20）。三维情况要求仅 1 通道，按 planes × rows 计数。

#### 关键 API

- `Mat::checkVector(elemChannels, depth=-1, requireContinuous=false)`：返回可解释的元素数，否则 `-1`；

- `Mat::create`：按尺寸与类型重建矩阵；

- `CV_Assert`：断言检查。

#### 处理流程

构造二维/三维 `Mat` → 对每种布局调用 `checkVector` → 用 `CV_Assert` 验证返回的元素数与预期一致。

#### 参数说明

| 参数                  | 含义         | 典型范围/默认        | 调大/调小会怎样                                     |
| ------------------- | ---------- | -------------- | -------------------------------------------- |
| `elemChannels`      | 每个元素期望的通道数 | `1` 或与列数/通道数匹配 | 设为单通道时会把“列数>1 且行数>1”判为非法（`-1`）；设为列数时视为单元素多通道 |
| `depth`             | 期望深度       | `-1`（任意）       | 指定后不匹配返回 `-1`                                |
| `requireContinuous` | 是否要求连续内存   | `false`        | 为 `true` 时非连续 ROI 会被判为非法                     |
| 维度                  | 2D 或 3D    | 2D/3D          | 3D 仅支持单通道，按 planes×rows 计数                   |

#### 关联与对比

这是 `cv::Mat` 布局语义的“自检工具”，很多算法内部用它判断输入能否当作点集/向量处理，与 8.2.3/8.2.4 的通道操作共同构成 `Mat` 布局基础，见[第 1 章](./ch01_core.md)。

#### 注意事项

- 非连续子矩阵（如 ROI、转置）在 `requireContinuous=true` 时会返回 `-1`；

- 返回 `-1` 不代表“错误”，而是“不是期望布局”，调用方需据此调整输入。

#### 应用场景

算法库输入校验、判断图像能否按点集/矩阵向量处理、泛型接口适配。

### 8.2.3 `core_merge.cpp` —— 合并通道平面

> **源文件**：`samples/cpp/tutorial_code/snippets/core_merge.cpp` ｜ **所属模块**：`core` ｜ **示例类型**：`snippet`

#### 功能概述

把三个单通道矩阵按通道维度合并成一个三通道 `Mat`，展示 `cv::merge` 的通道交错内存布局。

#### 核心原理

**30 秒心智模型**：三个“黑白图”叠成一张“彩色图”。`merge` 把每个源矩阵的第 `k` 个平面排进目标矩阵的第 `k` 个通道，目标矩阵的像素按 `[c0,c1,c2]` 交错存储。

设三输入 $m\_1,m\_2,m\_3$，目标多通道元素为

$$
dst(x,y)=\big(m\_1(x,y),,m\_2(x,y),,m\_3(x,y)\big),
$$

在内存中 `CV_8UC3` 按 B、G、R 连续排布，因此遍历顺序是“像素内三通道相邻、像素间按行优先”。

#### 关键 API

- `cv::merge(const Mat* mv, size_t count, OutputArray dst)` 或 `cv::merge(InputArrayOfArrays, dst)`；

- `Mat_<uchar>`：编译期类型化的矩阵字面量构造。

#### 处理流程

用 `Mat_<uchar>` 构造三个 `2x2` 单通道矩阵 → 装入 `Mat channels[3]` → `merge` 生成 `CV_8UC3` 目标 → 打印目标内容与 `channels()` 验证。

#### 参数说明

| 参数     | 含义          | 典型范围/默认               | 调大/调小会怎样                |
| ------ | ----------- | --------------------- | ----------------------- |
| 输入矩阵个数 | 源平面数量       | 任意 `>=1`，通常 `3` 或 `4` | 决定目标通道数；源尺寸/深度必须一致      |
| 源通道数   | 每个源矩阵的通道    | `1`（单通道平面）            | 多通道输入会先被拆分语义混淆，规范用法是单通道 |
| 尺寸一致性  | 所有源必须同尺寸同类型 | 相等                    | 不一致会抛异常或内存越界            |

#### 关联与对比

与 8.2.4 `split` 互为逆运算：`split` 把 `CV_8UC3` 拆成三张单通道图，`merge` 再拼回去。合成 BGR→RGB、处理单通道后重组彩色图都依赖它。

#### 注意事项

- 所有输入必须同尺寸、同深度；类型不同会报错；

- 输出通道数等于输入个数，用 `cv::merge` 前应确认输入按通道顺序排列（B,G,R）。

#### 应用场景

通道重组、R/G/B 分离处理后再合并、alpha 合成、多平面传感器数据拼装。

### 8.2.4 `core_split.cpp` —— 拆分多通道 Mat

> **源文件**：`samples/cpp/tutorial_code/snippets/core_split.cpp` ｜ **所属模块**：`core` ｜ **示例类型**：`snippet`

#### 功能概述

把一个三通道 `Mat` 拆成三个独立单通道矩阵，展示 `cv::split` 的通道分离语义与输出排列。

#### 核心原理

**30 秒心智模型**：把一张彩色图的“红、绿、蓝三张胶片”抽出来各放一边。`split` 按通道索引依次填充输出数组，第 `k` 个输出即原图的第 `k` 个通道。

对 `CV_8UC3` 的输入

$$
dst\_k(x,y) = src(x,y).ch(k), \quad k=0,1,2,
$$

输出为三个单通道 `CV_8UC1`，与原图同尺寸。通道顺序与 `merge` 一致（B,G,R），保证二者互逆。

#### 关键 API

- `cv::split(const Mat& src, Mat* mv)` 或 `cv::split(InputArray, OutputArrayOfArrays)`；

- `Mat::at`、`CV_8UC3`：类型化读写与类型宏。

#### 处理流程

用 `char d[]` 构造 `2x2` 的 `CV_8UC3` → 声明 `Mat channels[3]` → `split` 拆分 → 逐通道打印验证（每通道各含 4 个元素）。

#### 参数说明

| 参数     | 含义       | 典型范围/默认    | 调大/调小会怎样          |
| ------ | -------- | ---------- | ----------------- |
| 输入通道数  | 待拆分通道数   | 通常 `3`/`4` | 决定输出矩阵个数，不足会截断    |
| 输出数组容量 | 预分配的输出数量 | 至少等于通道数    | 容量不足只填前几个通道       |
| 输入类型   | 任意通道数    | `CV_8U` 等  | 输出继承输入深度，只降维不改变类型 |

#### 关联与对比

与 8.2.3 `merge` 互逆；与 `Mat::channels()`、`Mat::reshape(1)` 一样是通道语义的工具。视频处理中常先 `split` 提取单通道再处理（如 8.3.17 的通道提取）。

#### 注意事项

- 输出数组必须预先分配足够容量；

- `split` 输出是独立内存，修改不会回写原图；需要原图留白通道时用 `merge` 重建。

#### 应用场景

单通道分析（如只处理亮度）、通道直方图、色彩空间拆解、视频单通道提取。

### 8.2.5 `core_reduce.cpp` —— 沿行列聚合

> **源文件**：`samples/cpp/tutorial_code/snippets/core_reduce.cpp` ｜ **所属模块**：`core` ｜ **示例类型**：`snippet`

#### 功能概述

用 `cv::reduce` 沿行或列方向做求和、均值、最小、最大聚合，演示 `dim` 与 `rtype` 如何决定输出形状与数值。

#### 核心原理

**30 秒心智模型**：把矩阵“压扁”成一行或一列。`dim=0` 沿列方向聚合（结果是一行），`dim=1` 沿行方向聚合（结果是一列）。

对 $m$ 的列和与行和：

$$
\text{col\_sum}_j=\sum\_i m_{ij},\qquad
\text{row\_sum}_i=\sum\_j m_{ij}.
$$

`REDUCE_AVG` 在求和基础上除以聚合长度，`REDUCE_MAX/MIN` 取极值。`dtype` 控制输出深度：`CV_32F` 保留浮点均值，`CV_8U` 用于整型极值。

#### 关键 API

- `cv::reduce(src, dst, dim, rtype, dtype=-1)`；

- 聚合类型：`REDUCE_SUM`、`REDUCE_AVG`、`REDUCE_MAX`、`REDUCE_MIN`。

#### 处理流程

构造 `3x2` 的 `CV_8UC1` → 分别以 `dim=0/1` 调用 `reduce` 与四种 `rtype` → 打印列/行维结果验证形状与数值。

#### 参数说明

| 参数      | 含义       | 典型范围/默认              | 调大/调小会怎样                           |
| ------- | -------- | -------------------- | ---------------------------------- |
| `dim`   | 聚合方向     | `0`（列→一行）/ `1`（行→一列） | 决定输出是 1×N 还是 N×1                   |
| `rtype` | 聚合操作     | `SUM/AVG/MAX/MIN`    | `AVG` 是 `SUM` 除以计数；`MAX/MIN` 只保留极值 |
| `dtype` | 输出深度     | `-1`（同输入）或 `CV_32F`  | 求和/均值用浮点避免溢出与截断                    |
| 输入尺寸    | 被聚合的维度长度 | 任意                   | 决定求和的累加项数与 `AVG` 分母                |

#### 关联与对比

与 `sum`、`meanStdDev`、`norm` 同属全局归约族；`reduce` 的优势是可按方向聚合且输出仍是 `Mat`，可用于 8.2.6 计时等场景的统计。更完整的归约语义见[第 1 章](./ch01_core.md)。

#### 注意事项

- `REDUCE_SUM` 输出用 `CV_32F`，避免 8 位累加溢出；

- `dim` 与输出形状易混淆：`dim=0` 结果是“一行”，`dim=1` 结果是“一列”。

#### 应用场景

行/列统计、图像投影（如字符分割的水平/垂直投影）、特征向量聚合。

### 8.2.6 `core_various.cpp` —— Blob、旋转矩形与计时

> **源文件**：`samples/cpp/tutorial_code/snippets/core_various.cpp` ｜ **所属模块**：`features2d` + `core` ｜ **示例类型**：`snippet`

#### 功能概述

把三个独立的 `core`/`features2d` 用法拼在一份片段里：`SimpleBlobDetector` 斑点检测与 `FileStorage` 参数读写、`RotatedRect` 的最小外接矩形绘制、`TickMeter` 的计时与 FPS 统计。

#### 核心原理

**30 秒心智模型**：三个互不相关的小工具各干各的——检测斑点、画斜框、掐表，合在一起说明“`Mat` 之外的核心工具箱”。

`SimpleBlobDetector` 通过阈值分割连通域并按面积、圆度等过滤得到斑点；参数可用 `FileStorage` 序列化到 XML 供复现。`RotatedRect` 用中心、尺寸与旋转角描述斜矩形，`points()` 给出四个顶点，`boundingRect()` 给出轴对齐外接框。`TickMeter` 基于 `getTickCount/getTickFrequency`，`getFPS()` 直接给出平均帧率。

#### 关键 API

- `SimpleBlobDetector::create()`、`detect`、`FileStorage`；

- `RotatedRect::points`、`boundingRect`；

- `TickMeter`：`start/stop/getTimeSec/getAvgTimeSec/getFPS`；

- `drawKeypoints`、`circle`、`rectangle`、`line`、`putText`。

#### 处理流程

读取斑点参数（存在则读入，否则写默认）→ `detect` 出 `KeyPoint` → 绘制并叠加外接圆 → 构造旋转矩形、画四边与外接框 → 用 `TickMeter` 计时多轮并打印平均耗时与 FPS。

#### 参数说明

| 参数                  | 含义    | 典型范围/默认        | 调大/调小会怎样                       |
| ------------------- | ----- | -------------- | ------------------------------ |
| Blob 阈值与面积范围        | 过滤连通域 | 依图像而定          | 面积下限调大过滤小噪点；阈值步长影响分割敏感性        |
| `RotatedRect` 尺寸/角度 | 斜框几何  | 中心、`Size2f`、角度 | 角度决定倾斜程度，`boundingRect` 只给轴对齐框 |
| `TickMeter` 迭代次数    | 计时轮数  | 如 `100`        | 越多平均越稳，但总耗时越长                  |

#### 关联与对比

斑点检测与特征点相关，可延伸到[第 3 章特征匹配](./ch03_features.md)；旋转矩形在[第 2 章](./ch02_imgproc.md)的轮廓外接框中也常见；`TickMeter` 的计时语义与 8.3.5、8.5.1 的性能测量一脉相承。

#### 注意事项

- `SimpleBlobDetector` 需要 `features2d` 模块；

- `RotatedRect` 的角度定义与坐标系相关，绘制时注意顶点顺序；

- 计时应把“首次分配”与“稳态循环”分开，避免冷启动污染平均值。

#### 应用场景

斑点计数、倾斜目标外接框（OCR/仪表盘）、算法基准测试与 FPS 统计。

### 8.2.7 `imgcodecs_imwrite.cpp` —— Alpha PNG 与多页 TIFF

> **源文件**：`samples/cpp/tutorial_code/snippets/imgcodecs_imwrite.cpp` ｜ **所属模块**：`imgcodecs` ｜ **示例类型**：`snippet`

#### 功能概述

演示 `imwrite` 的两个进阶用法：用 `CV_8UC4` 写带 alpha 通道的 PNG，以及把一个 `vector<Mat>` 写进单个多页 TIFF。

#### 核心原理

**30 秒心智模型**：文件格式决定“能存哪些通道、存几页”。PNG 支持 RGBA 四通道，TIFF 支持多页，而 `imwrite` 的编码参数与重载形式决定具体怎么存。

`paintAlphaMat` 逐像素构造 BGRA，alpha 取 $0.5(B+G)$ 的均值，形成由绿到红渐变、透明度渐变的图案。压缩参数 `IMWRITE_PNG_COMPRESSION` 设为 `9` 是最大压缩。`imwrite("test.tiff", imgs)` 的重载把三张图按页写入多页 TIFF。

#### 关键 API

- `cv::imwrite(filename, mat, params)`；

- 编码参数：`IMWRITE_PNG_COMPRESSION` 等；

- 多页重载：`cv::imwrite(filename, std::vector<Mat>)`；

- `saturate_cast`、`Mat::at<Vec4b>`。

#### 处理流程

构造 `CV_8UC4` 并填充 BGRA 渐变 → 组装 PNG 压缩参数 → `imwrite("alpha.png")`（异常用 `try/catch` 捕获）→ 把原图、取反图、ROI 三图打包 → `imwrite("test.tiff")` 多页写出。

#### 参数说明

| 参数                        | 含义         | 典型范围/默认        | 调大/调小会怎样                               |
| ------------------------- | ---------- | -------------- | -------------------------------------- |
| 通道数                       | 是否含 alpha  | `4` 才写透明 PNG   | `CV_8UC3` 写 PNG 会丢 alpha；`CV_8UC1` 存灰度 |
| `IMWRITE_PNG_COMPRESSION` | PNG 压缩级别   | `0–9`，示例 `9`   | 越大文件越小但编码越慢；`0` 不压缩                    |
| 页向量                       | 多页 TIFF 内容 | 任意多张 `Mat`     | 页数决定 TIFF 页数；页尺寸/类型可不同                 |
| 输出扩展名                     | 决定编码器      | `.png/.tiff` 等 | 扩展名不匹配会走其他编码器或失败                       |

#### 关联与对比

读取侧的对应是 `imread`/`imreadmulti`；8.3.17 视频写出用 `VideoWriter` 而非 `imwrite`。文件编码细节（压缩、多页）可对比[第 2 章](./ch02_imgproc.md)的保存读写示例。

#### 注意事项

- 带 alpha 必须用 `CV_8UC4` 且扩展名为 `.png`，否则 alpha 被丢弃；

- 多页 TIFF 依赖编码器支持，写失败时 `imwrite` 返回 `false`，应检查返回值；

- PNG 压缩级别越高越慢，批量保存时权衡体积与耗时。

#### 应用场景

透明贴图导出、批量图像归档（多页 TIFF）、带元数据的可视化输出。

### 8.2.8 `imgproc_applyColorMap.cpp` —— 标量伪彩映射

> **源文件**：`samples/cpp/tutorial_code/snippets/imgproc_applyColorMap.cpp` ｜ **所属模块**：`imgproc` ｜ **示例类型**：`snippet`

#### 功能概述

把单通道灰度图通过 `applyColorMap` 映射为 `COLORMAP_JET` 伪彩图并显示，说明“标量 → RGB”的颜色编码。

#### 核心原理

**30 秒心智模型**：把“数值高低”翻译成“颜色冷暖”。灰度值按查色表线性插值成 RGB，低值偏蓝、高值偏红，人眼对色差比对灰阶更敏感。

伪彩映射是逐像素查表

$$
dst(x,y)=L\Big(\frac{src(x,y)-\min}{\max-\min}\Big),
$$

其中 $L$ 是 OpenCV 内置的 22 种色表之一（`COLORMAP_JET`、`COLORMAP_HOT`、`COLORMAP_INFERNO` 等）。输入可为灰度或彩色，彩色输入先转灰度再查表。

#### 关键 API

- `cv::applyColorMap(src, dst, colormap)`；

- 色表枚举：`COLORMAP_JET` 等；

- `imread`、`imshow`、`waitKey`。

#### 处理流程

读取图像 → 判空 → `applyColorMap(img_in, img_color, COLORMAP_JET)` → 显示“colorMap”窗口 → 等待按键。

#### 参数说明

| 参数         | 含义     | 典型范围/默认                | 调大/调小会怎样                        |
| ---------- | ------ | ---------------------- | ------------------------------- |
| `colormap` | 色表类型   | `COLORMAP_JET/HOT/...` | 决定冷暖方向与色阶，如 `JET` 蓝→红，`HOT` 黑→白 |
| 输入深度/通道    | 待映射图   | 8 位灰度或彩色               | 彩色图先转灰度再映射；其他深度需先归一             |
| 对比度        | 输入灰度分布 | 依赖图像                   | 分布窄则伪彩区分度低，可先直方图拉伸              |

#### 关联与对比

伪彩是可视化手段而非算法：深度图、热力图、置信度图常用（见 8.3.11/8.3.12 的深度可视化）。与[第 2 章](./ch02_imgproc.md)直方图均衡配合可增强区分度。

#### 注意事项

- `applyColorMap` 对单通道输出 `CV_8UC3`，多通道输入会先转灰度；

- 色表方向影响解读（如深度“近红远蓝”或反之），须与坐标轴标注一致。

#### 应用场景

深度/热力/显著性可视化、科学数据着色、红外成像显示。

### 8.2.9 `imgproc_calcHist.cpp` —— HSV 二维直方图可视化

> **源文件**：`samples/cpp/tutorial_code/snippets/imgproc_calcHist.cpp` ｜ **所属模块**：`imgproc` ｜ **示例类型**：`snippet`

#### 功能概述

把图像转到 HSV，用 `calcHist` 统计 H–S 二维直方图，再以亮度正比于统计值的方式把直方图渲染成图像。

#### 核心原理

**30 秒心智模型**：把“色相 × 饱和度”分成 30×32 个格子，数每个格子里的像素有多少，再把格子密度画成图。

直方图统计对第 $h,s$ 个 bin：

$$
H\[h]\[s]=\sum\_{x,y}\mathbf 1\big(\lfloor Hsv\_{x,y}/bins\rfloor=(h,s)\big),
$$

`calcHist` 对第 0、1 通道统计，`histSize={30,32}`、`ranges` 分别为 `[0,180)`（Hue）与 `[0,256)`（Saturation）。渲染时用 `minMaxLoc` 取最大 bin 值做归一化，`intensity = binVal*255/maxVal` 决定格子灰度，`rectangle` 逐格填充。

#### 关键 API

- `cvtColor(..., COLOR_BGR2HSV)`；

- `calcHist`：输入、通道、`histSize`、`ranges`、`uniform`；

- `minMaxLoc`、`rectangle`、`Mat::zeros`。

#### 处理流程

读图 → BGR 转 HSV → 设 `hbins=30, sbins=32` 与双通道范围 → `calcHist` 得 `MatND` → `minMaxLoc` 求最大 bin → 逐格 `rectangle` 渲染 → 双窗口显示原图与直方图。

#### 参数说明

| 参数              | 含义            | 典型范围/默认            | 调大/调小会怎样                       |
| --------------- | ------------- | ------------------ | ------------------------------ |
| `hbins`/`sbins` | 每维 bin 数      | `30`/`32`          | 越多越精细但越稀疏；过少丢失色彩分布细节           |
| `hranges`       | Hue 范围        | `[0,180)`（8 位 HSV） | 越界数值会按 `clipHistogram` 处理或统计不准 |
| `sranges`       | Saturation 范围 | `[0,256)`          | 需与数据实际范围一致                     |
| `uniform`       | 是否均匀分箱        | `true`             | `false` 时需自定义边界数组              |
| `scale`         | 渲染缩放          | `10`               | 决定直方图图像的分辨率/格子大小               |

#### 关联与对比

`calcHist` 是一维直方图（见[第 2 章](./ch02_imgproc.md)均衡）的二维推广；此处 H–S 分布可做肤色/色卡分析。直方图反向投影与追踪见[第 4 章](./ch04_video.md)的 CamShift 家族。

#### 注意事项

- 8 位 Hue 范围是 `[0,180)` 而非 `[0,256)`，填错范围统计会失真；

- `calcHist` 输入必须预处理为统计目标通道（此处 HSV）；

- 稀疏二维直方图直接用 `at<float>` 读取，注意类型为 `CV_32F`。

#### 应用场景

色彩分析、图像检索、肤色分割、白平衡与色卡标定。

### 8.2.10 `imgproc_drawContours.cpp` —— 轮廓层级绘制

> **源文件**：`samples/cpp/tutorial_code/snippets/imgproc_drawContours.cpp` ｜ **所属模块**：`imgproc` ｜ **示例类型**：`snippet`

#### 功能概述

对二值图调用 `findContours` 得到轮廓及其层级树，再用 `hierarchy` 沿 `[0]`（同级下一个）指针遍历顶层轮廓，为每个连通域随机着色 `drawContours`。

#### 核心原理

**30 秒心智模型**：把“哪些像素连成一片”变成“一串多边形”。`findContours` 输出轮廓点集与四元组层级，`hierarchy[idx]=[next, prev, firstChild, parent]`，示例用 `hierarchy[idx][0]` 沿同一层级的“下一个”遍历，保证每片只画一次、父轮廓不会覆盖子轮廓。

二值化用 `src = src > 1` 过滤近黑背景；`RETR_CCOMP` 提供两层（外轮廓 + 孔洞）层级；`CHAIN_APPROX_SIMPLE` 只保留角点压缩数据量。

#### 关键 API

- `findContours(image, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE)`；

- `drawContours(image, contours, contourIdx, color, FILLED, 8, hierarchy)`；

- `Mat::zeros`、`rand` 随机色。

#### 处理流程

灰度读图 → 二值化 `src > 1` → `findContours` 得轮廓与层级 → 从 `idx=0` 沿 `hierarchy[idx][0]` 迭代 → 随机色 `drawContours` 填充 → 显示“Source”与“Components”。

#### 参数说明

| 参数           | 含义      | 典型范围/默认               | 调大/调小会怎样                         |
| ------------ | ------- | --------------------- | -------------------------------- |
| 二值化阈值        | 前景/背景切分 | 示例 `>1`               | 提高阈值会缩小前景区域、改变轮廓形状               |
| `mode`       | 层级提取模式  | `RETR_CCOMP` 等        | `EXTERNAL` 只留最外轮廓；`TREE` 保留完整嵌套树 |
| `method`     | 轮廓压缩    | `CHAIN_APPROX_SIMPLE` | `NONE` 保留全部点；`SIMPLE` 只留角点，少画错角  |
| `contourIdx` | 绘制的轮廓号  | `FILLED` 填充           | 填具体索引只画单条；`FILLED` 填充整个区域        |
| `hierarchy`  | 层级信息    | 需与 `findContours` 同源  | 缺失或错位会导致绘制顺序/覆盖错误                |

#### 关联与对比

轮廓是最常用的“对象边界”表示，面积/外接框等度量见[第 2 章](./ch02_imgproc.md)；与 8.2.1 的 `fillPoly` 相比，`drawContours(FILLED)` 直接按检测到的边界填充。连通域分析的另一入口是 `connectedComponents`。

#### 注意事项

- `findContours` 会修改输入图，二值图应复制一份；

- 层级指针遍历依赖 `hierarchy` 非空，否则顶层索引循环可能越界；

- `RETR_CCOMP` 只有两层，深层嵌套需换 `RETR_TREE`。

#### 应用场景

目标计数与标注、区域分割可视化、掩膜重建、医学图像组织提取。

### 8.2.11 `imgproc_HoughLinesCircles.cpp` —— 霍夫圆检测片段

> **源文件**：`samples/cpp/tutorial_code/snippets/imgproc_HoughLinesCircles.cpp` ｜ **所属模块**：`imgproc` ｜ **示例类型**：`snippet`

#### 功能概述

对灰度图先高斯模糊再用 `HoughCircles`（`HOUGH_GRADIENT`）检测圆，把圆心与半径绘制到原图，展示霍夫圆变换的主要参数。

#### 核心原理

**30 秒心智模型**：圆由“圆心 + 半径”两个量决定。`HOUGH_GRADIENT` 先算梯度方向，每个边缘点沿梯度方向“投票”给可能的圆心，圆心得票多的位置再按半径聚类。

霍夫圆对参数空间 $(a,b,r)$ 投票，代价高，因此 `HOUGH_GRADIENT` 用两阶段：先用 Canny 得到边缘，再沿梯度累加圆心票数，最后对候选圆心统计半径直方图。参数 `dp` 为累加器分辨率倒数、`minDist` 为圆心最小间距、`param1` 为 Canny 高阈值、`param2` 为圆心累加器阈值。

#### 关键 API

- `GaussianBlur` 预平滑；

- `HoughCircles(gray, circles, HOUGH_GRADIENT, dp, minDist, param1, param2)`；

- `circle` 绘制圆心（半径 3）与圆廓。

#### 处理流程

读图 → 转灰度 → `GaussianBlur(9x9, σ=2)` 去噪 → `HoughCircles` 得 `Vec3f(x,y,r)` 列表 → 逐个画圆心与圆廓 → 显示。

#### 参数说明

| 参数        | 含义        | 典型范围/默认    | 调大/调小会怎样            |
| --------- | --------- | ---------- | ------------------- |
| `dp`      | 累加器分辨率倒数  | `2`        | 越大累加器越粗、速度越快但圆心定位越糙 |
| `minDist` | 圆心最小间距    | `rows/4`   | 过小把相邻圆误判为一圆；过大漏掉相邻圆 |
| `param1`  | Canny 高阈值 | `200`      | 越高边缘越少，弱圆可能丢失       |
| `param2`  | 圆心累加阈值    | `100`      | 越大要求投票越集中，减少误检但漏检增多 |
| 高斯核/σ     | 预平滑强度     | `(9,9), 2` | 平滑不足误检多，过强小圆被抹平     |

#### 关联与对比

与 8.2.12 的概率霍夫线段、8.2.13 的点集霍夫直线同属霍夫家族，区别在检测基元（圆/线段/直线）与输入（图像/点集）。圆检测常接在轮廓之后做工业零件定位，见[第 2 章](./ch02_imgproc.md)。

#### 注意事项

- 不预平滑会有大量假圆；

- `HOUGH_GRADIENT` 依赖 Canny，`param1` 与图像梯度尺度强相关；

- 输出是 `Vec3f`，半径可能为浮点，绘制前取整。

#### 应用场景

硬币/药片计数、镜头与孔位检测、球体定位、医学细胞圆斑提取。

### 8.2.12 `imgproc_HoughLinesP.cpp` —— 概率霍夫线段

> **源文件**：`samples/cpp/tutorial_code/snippets/imgproc_HoughLinesP.cpp` ｜ **所属模块**：`imgproc` ｜ **示例类型**：`snippet`

#### 功能概述

用 Canny 提取边缘后，`HoughLinesP` 输出线段端点而非无限直线，把检测到的线段叠加到彩色图显示。

#### 核心原理

**30 秒心智模型**：只对“少量随机采样的边缘点”投票，找到直线后再沿直线延伸“把散点连成线段”。

标准霍夫直线用 $(r,\theta)$ 参数空间，每条边缘点投票给经过它的所有直线，得票峰值即直线：

$$
r = x\cos\theta + y\sin\theta.
$$

`HoughLinesP` 的改进：随机采样边缘点子集投票，命中后沿直线方向连接邻近点并截断为线段；`minLineLength` 过滤过短线段，`maxLineGap` 允许缺口处拼接。

#### 关键 API

- `Canny(src, dst, 50, 200, 3)`；

- `HoughLinesP(dst, lines, rho, theta, threshold, minLineLength, maxLineGap)`；

- `line` 绘制 `Vec4i(x1,y1,x2,y2)`。

#### 处理流程

灰度读图 → `Canny` 边缘（`50/200/3`）→ 转 BGR 作画布 → `HoughLinesP` 得线段列表 → 逐条 `line` 红色绘制 → 显示源图与检测图。

#### 参数说明

| 参数              | 含义       | 典型范围/默认       | 调大/调小会怎样         |
| --------------- | -------- | ------------- | ---------------- |
| Canny 阈值        | 边缘高低阈值   | `50,200`      | 高阈值控制强边缘；过低噪声边缘多 |
| `rho`/`theta`   | 累加器分辨率   | `1` / `π/180` | 越细定位越准、计算越慢      |
| `threshold`     | 最少投票数    | `80`          | 越大误检越少，但弱/短直线被丢弃 |
| `minLineLength` | 线段最短长度   | `30`          | 越大过滤短碎线段         |
| `maxLineGap`    | 同线断点最大间距 | `10`          | 越大越容易把断开线段连成一条   |

#### 关联与对比

与 8.2.11 圆检测同族；与 8.2.13 点集霍夫相比，这里输入是边缘图、输出带端点。与[第 2 章](./ch02_imgproc.md)的 LSD 直线检测可对比“参数化投票”与“区域生长”两种思路。

#### 注意事项

- `HoughLinesP` 输出为线段端点，需 4 个坐标值；

- 边缘质量决定检测质量，Canny 阈值要配合场景调；

- `maxLineGap` 过大会把不同直线误拼。

#### 应用场景

车道线/轨道检测、文档倾斜校正、表框/网格提取、仪表刻度定位。

### 8.2.13 `imgproc_HoughLinesPointSet.cpp` —— 点集霍夫直线

> **源文件**：`samples/cpp/tutorial_code/snippets/imgproc_HoughLinesPointSet.cpp` ｜ **所属模块**：`imgproc` ｜ **示例类型**：`snippet`

#### 功能概述

给定一组二维点（`std::vector<Point2f>`），用 `HoughLinesPointSet` 直接在点集上做霍夫直线检测，输出投票数、$r$ 与 $\theta$，展示“点云直线拟合”的接口。

#### 核心原理

**30 秒心智模型**：没有图像，只有一堆点。每个点仍是“一条直线上的候选”，把 $(r,\theta)$ 参数空间按给定步长离散化，统计每根直线的点数。

对每个点 $(x\_i,y\_i)$，遍历离散的 $\theta$，计算

$$
r=x\_i\cos\theta+y\_i\sin\theta,
$$

落入对应 $(r,\theta)$ 累加格则加一票，峰值格对应拟合直线。`HoughLinesPointSet` 直接接收点集输入，输出 `Vec3d(votes, rho, theta)`。

#### 关键 API

- `HoughLinesPointSet(point, lines, point_count, lines_max, rhoMin, rhoMax, rhoStep, thetaMin, thetaMax, thetaStep)`；

- 输出为 `Vec3d(votes, rho, theta)`。

#### 处理流程

构造 20 个近似共线的 `Point2f` → 设 `rho` 范围 `[0,360]`、步长 `1`，`theta` 范围 `[0,π/2]`、步长 `π/180` → 调用 `HoughLinesPointSet` → 打印首条直线投票、`rho`、`theta`。

#### 参数说明

| 参数                            | 含义      | 典型范围/默认            | 调大/调小会怎样       |
| ----------------------------- | ------- | ------------------ | -------------- |
| `rhoMin/rhoMax/rhoStep`       | 距离范围与步长 | `0–360`，步长 `1`     | 步长越小定位越细、累加器越大 |
| `thetaMin/thetaMax/thetaStep` | 角度范围与步长 | `0–π/2`，步长 `π/180` | 范围过窄会漏掉垂直方向直线  |
| `linesMax`                    | 最多输出直线数 | `1`（示例）            | 增大可返回次优直线      |
| 点集质量                          | 点的共线程度  | 依赖输入               | 离群点会分散票数、拉偏峰值  |

#### 关联与对比

与 8.2.11/8.2.12 相比，这里绕过图像直接对点集操作，是“无图直线拟合”接口；常用于由关键点拟合直线。霍夫家族的数学基础见[第 2 章](./ch02_imgproc.md)。

#### 注意事项

- 输出 `Vec3d` 的三分量是（投票数，$r$，$\theta$），读取顺序勿错；

- 参数范围需覆盖数据实际的 $r,\theta$ 空间，否则无峰值；

- 点集规模很大时累加器内存与耗时显著增长。

#### 应用场景

关键点共线判定、直线轨迹拟合、网格角点对齐、点云基元提取。

### 8.2.14 `imgproc_segmentation.cpp` —— Intelligent Scissors 最短路径

> **源文件**：`samples/cpp/tutorial_code/snippets/imgproc_segmentation.cpp` ｜ **所属模块**：`imgproc`（`segmentation`） ｜ **示例类型**：`snippet`

#### 功能概述

演示 `IntelligentScissorsMB`（智能剪刀）的完整调用链：设置边缘特征参数 → `applyImage` 计算全图特征 → `buildMap` 以起点建最短路径代价图 → `getContour` 从目标点回溯出轮廓。

#### 核心原理

**30 秒心智模型**：把“从 A 到 B 沿物体边界的路径”看成图上的最短路径。边缘越强，边权越小，于是 Dijkstra 最短路径会自然地“贴着”边缘走。

`IntelligentScissorsMB` 对每个像素构建 8 邻接图，边权由梯度幅值、方向与拉普拉斯零交叉组合，边缘处代价低。`buildMap` 从种子点跑一次最短路径，`getContour(target)` 沿前驱指针回溯即得轮廓。`setEdgeFeatureCannyParameters` 指定用 Canny 特征提取边缘。

#### 关键 API

- `segmentation::IntelligentScissorsMB`；

- `setEdgeFeatureCannyParameters(threshold1, threshold2)`；

- `setGradientMagnitudeMaxLimit`；

- `applyImage`、`buildMap(source)`、`getContour(target, pts)`。

#### 处理流程

构造测试图 → 设置 Canny 特征参数（`16,100`）与梯度上限（`200`）→ `applyImage` 算特征 → 以 `(200,100)` 为源 `buildMap` → 对 `(400,300)` 目标 `getContour` 得点序列。

#### 参数说明

| 参数                          | 含义       | 典型范围/默认  | 调大/调小会怎样             |
| --------------------------- | -------- | -------- | -------------------- |
| Canny 阈值 1/2                | 边缘特征提取阈值 | `16,100` | 阈值过高弱边缘消失，路径会“穿出”边界  |
| `GradientMagnitudeMaxLimit` | 梯度上限     | `200`    | 决定梯度归一化，影响边权分布       |
| 源/目标点                       | 路径端点     | 图像内任意点   | 端点落在边界内/外决定能否贴边      |
| 权重组合                        | 特征权重系数   | 内部默认     | 调节“边缘强度 vs 方向连续性”的取舍 |

#### 关联与对比

这是交互式分割，与[第 2 章](./ch02_imgproc.md)的 GrabCut 同属分割族：GrabCut 是图割能量最小化，智能剪刀是图上最短路。它也体现“算法参数由特征提取器决定”，与 8.2.11 的 Canny 参数类似。

#### 注意事项

- 需要 `imgproc/segmentation.hpp` 头文件与对应模块；

- `buildMap` 每次换起点都要重算代价图；

- 目标点须在已建图的连通范围内，否则回溯失败。

#### 应用场景

交互式抠图、医学图像轮廓勾画、照片编辑的精确选区。

***

## 8.3 VideoIO

### 8.3.1 `audio_spectrogram.cpp` —— 音频波形与 STFT 频谱

> **源文件**：`samples/cpp/audio_spectrogram.cpp` ｜ **所属模块**：`videoio` + `imgproc` ｜ **示例类型**：`完整流程`

#### 功能概述

用 `VideoCapture` 读取音频文件或麦克风，绘制时域幅度图与短时傅里叶变换（STFT）频谱图（支持静态整段与动态滑动窗口两种模式），验证 `CAP_PROP_AUDIO_*` 系列的音频 IO 能力。

#### 核心原理

**30 秒心智模型**：波形看“响不响”，频谱看“什么频率在响”。长信号不能一次做傅里叶，于是切成重叠的小窗，每窗一次 DFT，把时间–频率–幅度画成彩色谱图。

STFT 把信号 $x\[n]$ 加窗后逐段 DFT：

$$
X(m,k)=\sum\_{n=0}^{W-1} x\[n+mS],w\[n],e^{-j2\pi kn/W},
$$

其中 $S$ 为窗移（`windLen - overlap`）。示例用 `dft(section, dstMat, DFT_COMPLEX_OUTPUT)` 并取前四分之一频谱（实信号频谱对称），取模后转分贝 $10\log\_{10}(\cdot)$。`readAudioFile` 通过 `grab/retrieve` 按块读取音轨，`CAP_PROP_AUDIO_BASE_INDEX` 定位音频流索引。

#### 关键 API

- `VideoCapture::open(file, CAP_ANY, params)`，参数含 `CAP_PROP_AUDIO_STREAM`、`CAP_PROP_VIDEO_STREAM=-1`、`CAP_PROP_AUDIO_DATA_DEPTH=CV_16S`；

- `CAP_PROP_AUDIO_SAMPLES_PER_SECOND/TOTAL_CHANNELS/TOTAL_STREAMS`；

- `dft`、`resize`、`normalize`、`applyColorMap(COLORMAP_INFERNO)`；

- `line`、`putText` 绘制坐标轴与网格。

#### 处理流程

解析命令行（`inputType/draw/graph/windowType/windLen/overlap` 等）→ 打开音频源读入 `vector<int>` 采样 → 补零到整秒 → 静态模式整段 `STFT` 或动态模式滑窗 → `drawAmplitude`/`drawSpectrogram` 绘制并加刻度 → 显示。

#### 参数说明

| 参数                          | 含义        | 典型范围/默认             | 调大/调小会怎样          |
| --------------------------- | --------- | ------------------- | ----------------- |
| `windLen`                   | STFT 窗长   | `256`               | 越大频率分辨率越高、时间分辨率越低 |
| `overlap`                   | 窗重叠       | `128`               | 越大时间轴更密，计算更重      |
| `windowType`                | 窗函数       | `Rect/Hann/Hamming` | 加窗抑制频谱泄漏，影响旁瓣     |
| `CAP_PROP_AUDIO_DATA_DEPTH` | 采样位深      | `CV_16S`            | 与文件位深不匹配会读不出正确值   |
| `audioStream`               | 选择的音轨号    | `0`/`1`             | 多音轨文件需显式选择        |
| `microTime/updateTime`      | 录制时长/滑窗步长 | `20`/`1` 秒          | 决定动态窗口的时间跨度与刷新率   |

#### 关联与对比

同一套 `VideoCapture` 也读视频（8.3.4），此处把音频帧当“一行采样”处理。STFT 是[第 1 章](./ch01_core.md) DFT 的窗化扩展；频谱渲染复用 8.2.8 的 `applyColorMap`。

#### 注意事项

- 音频读取依赖后端（Windows 上常用 `CAP_MSMF`），文件与位深不匹配时 `grab` 失败；

- STFT 只取前四分之一频谱，是因为实信号频谱共轭对称，勿误以为数据丢失；

- 静态模式会把末段补零到整秒，时长会显示为补零后的值。

#### 应用场景

语音可视化、乐器调音、声纹分析、音频质量诊断与录制预览。

### 8.3.2 `videocapture_audio.cpp` —— 文件音轨分块读取

> **源文件**：`samples/cpp/videocapture_audio.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

只读视频文件的音轨：用 `VideoCapture` 打开文件、屏蔽视频流，按通道逐块 `grab/retrieve` 取回音频帧并统计采样数。

#### 核心原理

**30 秒心智模型**：`VideoCapture` 不只是“读视频”，还能选择“要哪些流”。把 `CAP_PROP_VIDEO_STREAM` 设为 `-1`、`CAP_PROP_AUDIO_STREAM` 设为 `0`，就只抓音轨。

`CAP_PROP_AUDIO_BASE_INDEX` 给出音轨帧的检索起点，每通道的音频帧是一个 `1xN` 的 `CV_16S` 行矩阵（`frame.cols` 即样本数）。循环 `grab` 直至返回 `false`（文件读完），每次对每个通道 `retrieve(frame, audioBaseIndex+nCh)`。

#### 关键 API

- `VideoCapture::open(file, CAP_MSMF, params)`；

- `CAP_PROP_AUDIO_STREAM/VIDEO_STREAM/AUDIO_DATA_DEPTH`；

- `CAP_PROP_AUDIO_BASE_INDEX/TOTAL_CHANNELS/SAMPLES_PER_SECOND`；

- `grab` / `retrieve`。

#### 处理流程

解析文件路径 → 设置音频流参数打开 → 打印位深、采样率、通道数 → 循环 `grab`，逐通道 `retrieve` 收集 `audioData` → 累加样本数输出。

#### 参数说明

| 参数                          | 含义    | 典型范围/默认  | 调大/调小会怎样            |
| --------------------------- | ----- | -------- | ------------------- |
| `CAP_PROP_AUDIO_STREAM`     | 选择的音轨 | `0`      | 多音轨文件选错会取到空数据       |
| `CAP_PROP_VIDEO_STREAM`     | 视频流开关 | `-1`（关闭） | 打开后会占用解码带宽，帧率耦合     |
| `CAP_PROP_AUDIO_DATA_DEPTH` | 采样位深  | `CV_16S` | 与文件实际位深不符会读乱        |
| 通道数                         | 音轨声道数 | 由文件决定    | 决定每次 `retrieve` 的次数 |

#### 关联与对比

与 8.3.1 共用音频 IO 基础，区别是本例“只读不画”；与 8.3.3 相比，本例关闭了视频流，而 8.3.3 同时取回音视频。音频参数体系与视频属性（8.3.5）互为镜像。

#### 注意事项

- 必须用 `retrieve` 的第二个参数显式指定音频索引，否则默认取视频帧；

- 无音轨文件 `grab` 会立即失败，读取前应确认 `isOpened` 且通道数大于 0。

#### 应用场景

音频转写前置、音轨抽取、音频分析与无损转存。

### 8.3.3 `videocapture_audio_combination.cpp` —— 同文件音视频同步取回

> **源文件**：`samples/cpp/videocapture_audio_combination.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

同时打开同一文件的视频流与音轨（`CAP_PROP_VIDEO_STREAM=0`、`CAP_PROP_AUDIO_STREAM=0`），每次 `grab` 后分别 `retrieve` 出视频帧与各声道音频帧，边显示边统计帧数与采样数。

#### 核心原理

**30 秒心智模型**：一个 `grab` 把“这一时刻的所有流”都解出来，`retrieve` 再按需挑视频帧或某声道。视频与音频共用同一个捕获时间轴，因此同步是隐式的。

每次 `grab` 后：`retrieve(videoFrame)` 取默认视频帧；再对每个声道 `retrieve(audioFrame, audioBaseIndex+nCh)` 取音频块（`1xN` 的 `CV_16S`）。视频帧显示在“Live”窗口，`waitKey(5)` 兼顾刷新与按键响应。

#### 关键 API

- `CAP_PROP_AUDIO_STREAM` 与 `CAP_PROP_VIDEO_STREAM` 同时设为 `0`；

- `retrieve` 无参（视频）与带索引（音频）两种调用；

- `CAP_PROP_AUDIO_BASE_INDEX/TOTAL_CHANNELS`；

- `imshow`、`waitKey`。

#### 处理流程

解析文件 → 同时启用音视频流打开 → 打印音频属性 → 循环 `grab` → `retrieve` 视频帧显示、逐声道取音频帧 → 文件结束统计“音频采样数 + 视频帧数”。

#### 参数说明

| 参数                          | 含义     | 典型范围/默认  | 调大/调小会怎样                |
| --------------------------- | ------ | -------- | ----------------------- |
| `CAP_PROP_VIDEO_STREAM`     | 视频流号   | `0`      | 设为 `-1` 则退回纯音频模式（8.3.2） |
| `CAP_PROP_AUDIO_STREAM`     | 音轨号    | `0`      | 多音轨需显式选择                |
| `CAP_PROP_AUDIO_DATA_DEPTH` | 采样位深   | `CV_16S` | 位深不匹配会导致音频块内容无效         |
| 声道数                         | 同时取回声道 | 由文件决定    | 循环次数随声道增加               |

#### 关联与对比

与 8.3.2 相比，这里同时维持视频与音频流，是音视频同步消费的最小范式；与 8.3.4 纯视频循环相比多出音频分支。8.3.15 讨论跨设备时间戳对齐时，本示例代表“单设备内天然同步”。

#### 注意事项

- 视频与音频帧率不同：一次 `grab` 可能只有其中一个流有数据，需判空；

- 音频帧 `frame.cols` 才是样本数，不要按 `rows` 统计。

#### 应用场景

音画同步预览、字幕/配音对齐、多媒体容器分析。

### 8.3.4 `videocapture_basic.cpp` —— 摄像头采集最小循环

> **源文件**：`samples/cpp/videocapture_basic.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

打开默认摄像头（`deviceID=0`、`apiID=CAP_ANY` 自动探测后端），循环 `read` 帧并实时显示，是 VideoCapture 的最短可用范式。

#### 核心原理

**30 秒心智模型**：摄像头是一个“源源不断吐帧的流”。`open` 建立连接，`read` 抓一帧（等于 `grab+retrieve`），`frame.empty()` 表示流中断，`waitKey` 短超时让窗口刷新并响应按键。

`cap.open(deviceID, apiID)` 用“设备号 + 后端 API”组合寻址；`CAP_ANY` 让 OpenCV 自动选择可用后端。`read` 返回后若 `frame.empty()` 说明抓帧失败（设备拔出、传输错误），应跳出循环而非继续显示黑帧。

#### 关键 API

- `VideoCapture cap; cap.open(deviceID, apiID)`；

- `cap.isOpened()`、`cap.read(frame)`；

- `imshow`、`waitKey(5)`。

#### 处理流程

声明 `VideoCapture` → 指定设备 `0` 与 `CAP_ANY` → `isOpened` 校验 → 无限循环 `read` → 空帧判错退出 → `imshow("Live")` + `waitKey(5)` → 析构自动释放。

#### 参数说明

| 参数           | 含义     | 典型范围/默认   | 调大/调小会怎样                |
| ------------ | ------ | --------- | ----------------------- |
| `deviceID`   | 摄像头编号  | `0` 起     | 多摄像头时依次递增选择             |
| `apiID`      | 后端 API | `CAP_ANY` | 指定后端可绕开自动探测失败；错误选择打不开设备 |
| `waitKey` 超时 | 窗口刷新间隔 | `5` ms    | 越小响应越快但 CPU 占用越高；负值阻塞   |

#### 关联与对比

与 8.3.13 的通用入口（文件/序列/设备自动识别）相比，本示例硬编码摄像头；与 8.3.5 相比不测 FPS、不做处理。它是所有“摄像头实时应用”的最小骨架。

#### 注意事项

- 摄像头被其他程序占用时 `open` 失败，需先释放；

- 首帧可能偏暗（自动曝光收敛中），可预热几帧；

- `waitKey` 超时过大界面卡顿、过小 CPU 空转。

#### 应用场景

实时预览、安防监控底座、一切以摄像头为输入的演示程序。

### 8.3.5 `videocapture_camera.cpp` —— 采集属性与处理 FPS

> **源文件**：`samples/cpp/videocapture_camera.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

读取摄像头宽高与 FPS 属性，用 `getTickCount` 统计采集/处理耗时与平均 FPS，空格键切换是否执行 Canny 处理，量化“采集 vs 处理”的时间占比。

#### 核心原理

**30 秒心智模型**：摄像头“吐帧”和你的“处理”各花时间。`getTickCount` 掐表，用 `getTickFrequency` 换算秒，就能看出瓶颈在采集还是处理。

`capture.get(CAP_PROP_FRAME_WIDTH/HEIGHT/FPS)` 读取设备属性。性能统计：每 10 帧用

$$
\overline{\text{FPS}}=\frac{N\cdot f\_{tick}}{t\_1-t\_0},\qquad
\bar t\_{frame}=\frac{t\_1-t\_0}{N\cdot f\_{tick}},
$$

其中 $f\_{tick}$ 为时钟频率。处理分支对每帧 `Canny(frame, processed, 400, 1000, 5)`，累积处理时间单独统计，从而对比开关处理前后的采集/处理分配。

#### 关键 API

- `VideoCapture capture(0)`；

- `get(CAP_PROP_FRAME_WIDTH/HEIGHT/FPS)`；

- `getTickCount`、`getTickFrequency`；

- `Canny`、`imshow`、`waitKey`。

#### 处理流程

打开摄像头 → 打印分辨率与 FPS → 循环 `read` → 每 10 帧打印平均 FPS 与帧耗时 → 空格切换 `enableProcessing` → 开启时 `Canny` 后显示，否则原图显示 → ESC 退出并打印总帧数。

#### 参数说明

| 参数             | 含义     | 典型范围/默认      | 调大/调小会怎样                  |
| -------------- | ------ | ------------ | ------------------------- |
| 统计窗口 `N`       | 测速帧数   | `10`         | 越大平均越稳，但读数更新越慢            |
| `Canny` 阈值     | 处理负荷   | `400,1000,5` | 影响处理耗时，用于观察处理占采集的比例       |
| `CAP_PROP_FPS` | 设备标称帧率 | 设备决定         | 只是标称值，实际 FPS 由带宽与处理耗时共同决定 |
| 分辨率属性          | 采集分辨率  | 设备默认         | 改分辨率需 `set`，可能被设备拒绝       |

#### 关联与对比

测速手段与 8.2.6 的 `TickMeter`、8.5.1 的计时一致，此处关心“采集–处理”流水线分配；Canny 参数语义见[第 2 章](./ch02_imgproc.md)。

#### 注意事项

- `CAP_PROP_FPS` 是标称值，与实测帧率可能不同；

- 高分辨率 + 高 FPS 可能超出 USB 带宽，`read` 返回空或帧率骤降；

- 统计时应避开首帧（自动曝光收敛期）。

#### 应用场景

采集链路性能评估、处理管线负载测试、嵌入式视觉的帧率预算分配。

### 8.3.6 `videocapture_gphoto2_autofocus.cpp` —— DSLR 边缘清晰度自动对焦

> **源文件**：`samples/cpp/videocapture_gphoto2_autofocus.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

通过 `CAP_GPHOTO2` 控制 DSLR，用“图像清晰度度量 + 爬山搜索”自动调节对焦（`CAP_PROP_ZOOM` 映射到对焦步进），实现不依赖硬件 AF 的软件自动对焦。

#### 核心原理

**30 秒心智模型**：清晰度是“高频细节有多少”。把镜头焦点当作一维坐标，清晰度度量沿坐标是单峰函数，朝上升方向走，越过峰值后回退，即找到最清晰焦点。

清晰度度量取拉普拉斯/梯度能量的对比度值，压缩、噪声等会引入固定偏差 `epsylon`。对焦状态机维护 `step/direction/rateMax`：在当前方向移动 `FOCUS_STEP` 并采样清晰度，若上升继续同向，若下降则反向并缩小步长（`minFocusStep`），连续 `breakLimit` 次无改进即收敛。`focusDriveEnd` 用 `cap.set(CAP_PROP_ZOOM, ±MAX_FOCUS_STEP)` 驱动镜头到机械边界以确定起始方向。

#### 关键 API

- `VideoCapture::set(CAP_PROP_ZOOM, value)`：对焦驱动（DSLR 后端映射）；

- 清晰度度量：边缘/梯度统计（如 Laplacian 方差）；

- `getTickCount` 控制采样节奏，`VideoWriter` 输出录像。

#### 处理流程

解析参数 → 打开 `CAP_GPHOTO2` 设备 → 驱动到机械边界确立方向 → 循环：移动焦点 → 抓帧算清晰度 → 更新 `FocusState` 决定下一步方向/步长 → 收敛或超时停止 → 可选写视频。

#### 参数说明

| 参数             | 含义        | 典型范围/默认  | 调大/调小会怎样           |
| -------------- | --------- | -------- | ------------------ |
| `FOCUS_STEP`   | 对焦步进      | `1024`   | 越大收敛越快但易越过峰值（来回震荡） |
| `minFocusStep` | 最小步长      | 自动探测     | 决定最终对焦精度，过小收敛慢     |
| `breakLimit`   | 无改进即停止的轮数 | `5`      | 越大越稳但耗时更长          |
| `epsylon`      | 清晰度容差     | `0.0005` | 抑制压缩/噪声造成的伪峰       |
| 清晰度度量窗口        | 采样的图像区域   | 中央区域     | 区域含背景纹理会导致误判       |

#### 关联与对比

“爬山 + 度量”与 8.5.1 的性能测量同属“迭代采样评估”；与硬件 AF 相比这是纯软件方案，依赖 `CAP_GPHOTO2` 后端与 `CAP_PROP_ZOOM` 映射，无法在普通 UVC 摄像头使用。

#### 注意事项

- 只在 DSLR/支持 gphoto2 的设备可用，普通网络摄像头不映射 `CAP_PROP_ZOOM`；

- 机械驱动到边界需小心，部分镜头会报错退出循环；

- 低光照或纯色场景清晰度曲线平坦，会误收敛。

#### 应用场景

无硬件 AF 的相机对焦、显微自动聚焦、文档拍摄机自动对焦。

### 8.3.7 `videocapture_gstreamer_pipeline.cpp` —— 编解码后端性能管线

> **源文件**：`samples/cpp/videocapture_gstreamer_pipeline.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

在 `CAP_GSTREAMER` 后端下用自定义 GStreamer 管线（`appsink`）采集，并演示按分辨率（720p/1080p/4K）与编解码器（h264/h265/mpeg4/mjpeg/vp8）组合构建解/编码元素，衡量软硬件编解码性能。

#### 核心原理

**30 秒心智模型**：视频 IO 由“后端 + 管线字符串”驱动。`videocapture_gstreamer_pipeline` 把摄像头接入自定义管线，通过 `appsink` 把帧送回 OpenCV；写出则用 `appsrc` 把帧送进编码器。

`CAP_GSTREAMER` 使用 OpenCV 构建的 GStreamer 管线，`gst_parse_launch` 风格的字符串拼出 `videotestsrc ! ... ! appsink`。示例用 `map<string,...>` 维护：`sizeByResolution`（720p/1080p/4K）、`fourccByCodec`（H264/H265/MPEG2/MPEG4/MJPEG/VP8）、软/硬件编解码元素（`x264enc` vs `vaapih264enc`、`mfxh264dec` 等），从而在纯 CPU 与硬件加速间切换。

#### 关键 API

- `VideoCapture::open(0, CAP_GSTREAMER, params)` / `VideoWriter` 对应后端；

- `CAP_GSTREAMER` 参数：`CAP_PROP_GSTREAMER_QUEUE_LENGTH` 等；

- GStreamer 元素名映射（`x264enc`、`vaapi*`、`mfx*`）。

#### 处理流程

解析命令行（分辨率 + 编解码器 + 硬件加速开关）→ 查表得尺寸、`fourcc`、编解码元素名 → 拼接 GStreamer 管线字符串 → 打开捕获/写出 → 循环读帧/写帧并计时 → 输出平均 FPS。

#### 参数说明

| 参数     | 含义      | 典型范围/默认                        | 调大/调小会怎样             |
| ------ | ------- | ------------------------------ | -------------------- |
| 分辨率    | 采集/输出尺寸 | `720p/1080p/4K`                | 越大每帧耗时与带宽越大          |
| 编解码器   | 编码格式    | `h264/h265/mpeg4/mjpeg/vp8`    | 决定码率、兼容性与硬件加速可行性     |
| 软/硬件元素 | 编解码实现   | `x264enc` 等 vs `vaapi*`/`mfx*` | 硬件加速大幅降 CPU 但受驱动支持限制 |
| 队列长度   | 管线缓冲    | 可配                             | 越长抗抖动但延迟越大           |

#### 关联与对比

与 8.3.4 的 `CAP_ANY` 相比，本示例把“后端选型”显式化：GStreamer 管线把解码、缩放、编码全部交给系统多媒体框架。与 8.3.14 的 `VideoWriter` 相比，这里直接操控编码元素而非 `fourcc` 封装。

#### 注意事项

- 依赖构建时的 `WITH_GSTREAMER` 与系统 GStreamer 插件（含硬件加速插件）；

- 管线字符串语法错误会导致 `open` 静默失败；

- 硬件编解码元素（`vaapi*`/`mfx*`）依赖具体 GPU 驱动，不具备可移植性。

#### 应用场景

高吞吐视频采集、低延迟直播推流、软硬件编解码性能基准。

### 8.3.8 `videocapture_image_sequence.cpp` —— 图像编号序列读取

> **源文件**：`samples/cpp/videocapture_image_sequence.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

把“按编号命名的图像序列”当作视频打开：`VideoCapture sequence("example_%02d.jpg")`，逐帧读取并显示，直到返回空图。

#### 核心原理

**30 秒心智模型**：一张张编号图片就是“一帧帧视频”。`%02d` 是 printf 风格的通配符，`VideoCapture` 按编号递增自动找到下一张，没有下一张即序列结束。

`image_%02d.jpg` 会匹配 `image_00.jpg, image_01.jpg, ...`，`%02d` 中的宽度决定编号补零位数。读帧循环用 `sequence >> image`，`image.empty()` 表示序列结束。

#### 关键 API

- `VideoCapture sequence("left%02d.jpg")`；

- `operator>>` 读帧、`isOpened`、`empty` 判结束；

- `namedWindow`、`imshow`、`waitKey`。

#### 处理流程

解析图像掩码（默认 `../data/left%02d.jpg`）→ 构造 `VideoCapture` → 循环 `sequence >> image` → 空图退出 → `imshow` + `waitKey(500)` → 打印“End of Sequence”。

#### 参数说明

| 参数           | 含义     | 典型范围/默认  | 调大/调小会怎样         |
| ------------ | ------ | -------- | ---------------- |
| `%0Nd` 宽度    | 编号补零位数 | 如 `%02d` | 与实际文件命名不符则找不到后续帧 |
| 起始编号         | 序列第一张  | 取决于首帧    | 通配符从编号 `0` 开始    |
| `waitKey` 间隔 | 播放节奏   | `500` ms | 越大播放越慢，便于逐帧检查    |

#### 关联与对比

与 8.3.13 的统一入口共用“文件/序列/设备”识别，这里是序列专用；图像序列常作为“无压缩参考视频”，用于 8.3.16 的质量评测。

#### 注意事项

- 掩码编号宽度必须与真实文件名一致（`%02d` 不会自动补 `%3d` 的文件）；

- 编号不连续或跳跃会在缺失处结束序列；

- 不同帧尺寸不一致时 `imshow` 可能异常。

#### 应用场景

高速相机帧序列回放、数据集可视化、离线视频质量评估的输入。

### 8.3.9 `videocapture_microphone.cpp` —— MSMF 麦克风采集

> **源文件**：`samples/cpp/videocapture_microphone.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

用 `CAP_MSMF` 打开系统麦克风（设备 `0`），关闭视频流，`grab/retrieve` 采集 10 秒音频块，统计采样数。

#### 核心原理

**30 秒心智模型**：麦克风和摄像头一样是一个“流”。区别只在流里是音频帧（`1xN` 的 `CV_16S`），且要用 `CAP_PROP_AUDIO_BASE_INDEX` 指定检索索引。

`cap.open(0, CAP_MSMF, params)` 把 `CAP_PROP_AUDIO_STREAM=0`、`CAP_PROP_VIDEO_STREAM=-1` 传入，即“纯音频采集”。用 `getTickCount/getTickFrequency` 限制采集时长（`10` 秒），每 `grab` 后对每个声道 `retrieve(frame, audioBaseIndex+nCh)`，`frame.cols` 即该块样本数。

#### 关键 API

- `VideoCapture::open(0, CAP_MSMF, params)`；

- `CAP_PROP_AUDIO_STREAM/VIDEO_STREAM`；

- `CAP_PROP_AUDIO_BASE_INDEX/TOTAL_CHANNELS/SAMPLES_PER_SECOND`；

- `getTickCount`、`getTickFrequency` 计时。

#### 处理流程

设参数打开麦克风 → 打印音频属性 → `while (tick < 10s)`：`grab` → 逐声道 `retrieve` 收集 → 累加 `frame.cols` → 输出总采样数。

#### 参数说明

| 参数                      | 含义    | 典型范围/默认     | 调大/调小会怎样                   |
| ----------------------- | ----- | ----------- | -------------------------- |
| 采集时长                    | 录音秒数  | `10`        | 越长收集采样越多、耗时越长              |
| `CAP_PROP_AUDIO_STREAM` | 麦克风流号 | `0`         | 多输入设备时选择目标                 |
| 采样率                     | 每秒采样数 | 设备默认（如 48k） | 决定单块 `frame.cols` 大小       |
| 声道数                     | 采集声道  | 设备决定        | 决定每 `grab` 的 `retrieve` 次数 |

#### 关联与对比

与 8.3.2 的文件音轨读取逻辑完全同构，只是数据源换成了麦克风；音频处理（波形/频谱）见 8.3.1。这体现 `VideoCapture` 对“采集源”的统一抽象。

#### 注意事项

- Windows 上通常需 `CAP_MSMF` 后端；其他平台可能不同；

- 麦克风被占用或权限未授予时 `open` 失败；

- 计时用墙钟，实际采集样本数受驱动缓冲影响，勿假设与秒数精确对应。

#### 应用场景

语音识别前端、录音工具、实时音频可视化与声控交互。

### 8.3.10 `videocapture_obsensor.cpp` —— Orbbec 彩色与深度流

> **源文件**：`samples/cpp/videocapture_obsensor.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

用 `CAP_OBSENSOR` 打开 Orbbec 深度相机，读取彩色图与深度图，打印相机内参（fx/fy/cx/cy）与畸变参数，并支持通过 `CAP_PROP_OBSENSOR_*` 设置深度/彩色分辨率和帧率。

#### 核心原理

**30 秒心智模型**：深度相机是一个“彩色流 + 深度流”的双流设备。深度图记录每个像素到相机的距离，配合内参就能算 3D 坐标。

深度图在 `minVal/maxVal`（示例 `300–5000` mm）内有效，用线性映射 `depthMap.convertTo/归一化` 转成 `CV_8UC1` 再伪彩显示。内参与畸变由 `CAP_PROP_OBSENSOR_INTRINSIC_FX/FY/CX/CY` 与 `CAP_PROP_OBSENSOR_COLOR_DISTORTION_K*` 读出，是后续 3D 重建的输入。

#### 关键 API

- `VideoCapture::open(0, CAP_OBSENSOR, params)`；

- `CAP_PROP_OBSENSOR_DEPTH_WIDTH/HEIGHT/FPS`、`CAP_PROP_FRAME_WIDTH/HEIGHT/FPS`；

- `CAP_PROP_OBSENSOR_INTRINSIC_FX/FY/CX/CY`、`CAP_PROP_OBSENSOR_COLOR_DISTORTION_K1..K6/P1/P2`；

- `grab/retrieve` 取深度与彩色帧。

#### 处理流程

解析深度/彩色尺寸与帧率参数 → 组装 `params` 打开设备 → 读内参与畸变打印 → 循环 `grab`：`retrieve` 深度图与彩色图 → 深度归一化 + 伪彩 → 双窗口显示。

#### 参数说明

| 参数                                         | 含义     | 典型范围/默认       | 调大/调小会怎样              |
| ------------------------------------------ | ------ | ------------- | --------------------- |
| `CAP_PROP_OBSENSOR_DEPTH_WIDTH/HEIGHT/FPS` | 深度流规格  | 设备支持集合        | 超出支持列表会被拒绝或回退         |
| `CAP_PROP_FRAME_WIDTH/HEIGHT/FPS`          | 彩色流规格  | 同上            | 影响彩色分辨率与带宽            |
| `minVal/maxVal`                            | 深度显示范围 | `300–5000` mm | 范围外深度被截断；范围越窄对比越强但丢细节 |
| 内参/畸变                                      | 相机标定参数 | 设备固件          | 用于 3D 反投影，参数错误会扭曲点云   |

#### 关联与对比

与 8.3.11/8.3.12 同属深度相机示例，差异在厂商/后端：Obsensor 走 `CAP_OBSENSOR`，OpenNI 走 `CAP_OPENNI2`，RealSense 走 `CAP_INTELPERC`。深度→点云与内参的关系见[第 7 章标定](./ch07_calib3d_stitching.md)。

#### 注意事项

- 依赖 Orbbec SDK 与构建时 `WITH_OBSENSOR`；

- 深度与彩色流的帧率/时间戳需对齐（见 8.3.15）；

- 部分型号仅支持特定内核版本，硬件兼容性需确认。

#### 应用场景

人体/物体 3D 重建、手势交互、避障与抓取、SLAM 前端。

### 8.3.11 `videocapture_openni.cpp` —— OpenNI 多种深度输出

> **源文件**：`samples/cpp/videocapture_openni.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

通过 `CAP_OPENNI2` 打开 Kinect/Xtion 等深度传感器，用 `-m` 掩码选择显示深度图、视差图、有效像素掩膜、RGB/灰度/IR 等输出，并支持 `.oni` 录制文件回放。

#### 核心原理

**30 秒心智模型**：一个传感器同时提供“深度、视差、掩膜、彩色、IR”多个映射，`retrieve` 的第二参数决定取哪一路。

支持的输出映射及类型：

- `CAP_OPENNI_DEPTH_MAP`：深度，`CV_16UC1`（毫米）；

- `CAP_OPENNI_POINT_CLOUD_MAP`：三维点云，`CV_32FC3`（米）；

- `CAP_OPENNI_DISPARITY_MAP`：视差，`CV_8UC1`（像素）；

- `CAP_OPENNI_VALID_DEPTH_MASK`：有效像素掩膜；

- `CAP_OPENNI_BGR_IMAGE`/`GRAY_IMAGE`：彩色/灰度；

- `CAP_OPENNI_IR_IMAGE`：红外，`CV_16UC1`。

视差到深度的关系为 $Z=\dfrac{bF}{d}$，`getMaxDisparity` 用基线 `CAP_OPENNI_DEPTH_GENERATOR_BASELINE` 与焦距 `FOCAL_LENGTH` 及最小距离估算视差上限；`colorizeDisparity` 把视差线性映射到 `COLORMAP_JET`。

#### 关键 API

- `VideoCapture capture(CAP_OPENNI2)`；

- `retrieve(dst, CAP_OPENNI_DEPTH_MAP/POINT_CLOUD_MAP/DISPARITY_MAP/...)`；

- `CAP_OPENNI_DEPTH_GENERATOR_BASELINE/FOCAL_LENGTH`；

- `CAP_OPENNI_VGA_30HZ/SXGA_15HZ` 等模式常量；

- `applyColorMap`、`minMaxLoc`、`convertScaleAbs`。

#### 处理流程

解析 `-cd/-fmd/-mode/-m/-r` → 打开设备或 `.oni` 文件 → 按掩码循环 `grab` 并 `retrieve` 所选映射 → 深度/视差转伪彩 → 拼接或分窗显示。

#### 参数说明

| 参数         | 含义          | 典型范围/默认                   | 调大/调小会怎样         |
| ---------- | ----------- | ------------------------- | ---------------- |
| `-m` 掩码    | 选择输出映射      | 默认 `010100`（视差+RGB）       | 决定同时取回哪些流，影响带宽   |
| `-mode`    | 图像模式        | `VGA_30HZ/SXGA_15HZ/30HZ` | 分辨率与帧率权衡，高分辨率降帧率 |
| `-cd/-fmd` | 是否伪彩/固定视差上限 | `1`/`0`                   | 伪彩便于观察；固定上限稳定色标  |
| 最小距离       | 视差上限的推算基准   | `400` mm                  | 越小视差上限越大，近处越细    |

#### 关联与对比

与 8.3.10/8.3.12 同为深度多流，区别是后端与映射集：OpenNI 输出点云/视差/掩膜等更丰富。深度、视差与相机参数的关系可与[第 7 章立体视觉](./ch07_calib3d_stitching.md)对照。

#### 注意事项

- 需安装 OpenNI 与 PrimeSensor 模块，构建时 `WITH_OPENNI` 开启；

- 各映射帧率一致但时间戳可能错位，多流同步见 8.3.15；

- 点云为 `CV_32FC3`，直接显示需转 `CV_8U`。

#### 应用场景

体感交互、深度分割、RGB-D SLAM、点云采集。

### 8.3.12 `videocapture_realsense.cpp` —— RealSense 深度、彩色与 IR

> **源文件**：`samples/cpp/videocapture_realsense.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

用 `CAP_INTELPERC` 打开 Intel RealSense 相机，`grab` 后分别 `retrieve` 出深度图、彩色图与红外图，深度归一化后伪彩显示。

#### 核心原理

**30 秒心智模型**：RealSense 把“彩色 + 红外 + 深度”三条流放进同一个捕获对象，一次 `grab` 同时解出三者，`retrieve` 按流编号挑选。

`capture.grab()` 后依次

- `retrieve(depthMap, CAP_INTELPERC_DEPTH_MAP)`；

- `retrieve(image, CAP_INTELPERC_IMAGE)`；

- `retrieve(irImage, CAP_INTELPERC_IR_MAP)`。

深度用 `normalize(depthMap, adjMap, 0, 255, NORM_MINMAX, CV_8UC1)` 线性拉伸到 `[0,255]`，再 `applyColorMap(COLORMAP_JET)`。

#### 关键 API

- `VideoCapture capture(CAP_INTELPERC)`；

- `CAP_INTELPERC_DEPTH_MAP/IMAGE/IR_MAP`；

- `normalize`、`applyColorMap`、`imshow`、`waitKey`。

#### 处理流程

以 `CAP_INTELPERC` 打开设备 → 循环 `grab` → `retrieve` 深度/彩色/IR → 深度 `normalize` 到 8 位并伪彩 → 三窗口显示 RGB/IR/DEPTH → 任意键退出。

#### 参数说明

| 参数                        | 含义     | 典型范围/默认 | 调大/调小会怎样           |
| ------------------------- | ------ | ------- | ------------------ |
| `CAP_INTELPERC_DEPTH_MAP` | 深度流索引  | 固定常量    | 取错索引得到空或错误数据       |
| `NORM_MINMAX` 范围          | 深度拉伸范围 | `0–255` | 固定范围受深度极值影响，动态拉伸更稳 |
| `COLORMAP_JET`            | 深度伪彩色表 | 可换      | 决定远近颜色方向           |
| `waitKey` 间隔              | 刷新率    | `30` ms | 越小越流畅、CPU 占用越高     |

#### 关联与对比

与 8.3.11 OpenNI 的结构几乎相同，仅流常量与后端不同，直接对比可看出“同一抽象、不同厂商”。深度归一化的可视化思路贯穿 8.3.10–8.3.12。

#### 注意事项

- 需安装 Intel RealSense SDK 且构建时 `WITH_INTELPERC`；

- 三流的时间戳并不严格对齐，需自行配准（见 8.3.15）；

- 深度有效范围外是 `0` 或极大值，归一化前建议先掩膜。

#### 应用场景

RGB-D 采集、姿态估计、深度感知交互、避障。

### 8.3.13 `videocapture_starter.cpp` —— 文件、序列和设备统一入口

> **源文件**：`samples/cpp/videocapture_starter.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

用一个命令行参数同时支持视频文件、图像序列与摄像头：先按字符串打开（文件/序列），失败再按整数打开（摄像头），随后统一进入 `process` 循环，空格存帧、`q/ESC` 退出。

#### 核心原理

**30 秒心智模型**：`VideoCapture` 的构造/`open` 按参数“猜类型”：字符串是文件路径或序列掩码，整数是摄像头编号。猜错就换一种试，最终都汇入同一个“读帧–显示”循环。

```cpp
VideoCapture capture(arg);            // 先当作文件/序列
if (!capture.isOpened())
    capture.open(atoi(arg.c_str()));  // 再当作摄像头编号
```

`process` 循环用 `waitKey(30)` 兼顾刷新与按键；空格键用 `imwrite("filename%.3d.jpg")` 保存当前帧。

#### 关键 API

- `VideoCapture capture(arg)` / `capture.open(atoi(arg))`；

- `capture >> frame`、`frame.empty()`；

- `imshow`、`waitKey(30)`、`imwrite`；

- `WINDOW_KEEPRATIO` 可缩放窗口。

#### 处理流程

解析 `@input` → 先按字符串打开，失败按整数打开 → 均失败则打印帮助退出 → 循环读帧 → 空帧结束 → 空格保存 `filenameNNN.jpg`、`q/Q/ESC` 退出。

#### 参数说明

| 参数           | 含义        | 典型范围/默认            | 调大/调小会怎样          |
| ------------ | --------- | ------------------ | ----------------- |
| `@input`     | 视频/序列/设备号 | 任意                 | 字符串走文件/序列，纯数字走摄像头 |
| `waitKey` 间隔 | 刷新/响应     | `30` ms            | 越小越流畅；过大按键响应慢     |
| 保存命名         | 帧存盘名      | `filename%.3d.jpg` | 决定连续帧的文件名编号宽度     |

#### 关联与对比

是 8.3.4（摄像头）、8.3.8（序列）的统一封装，体现 `VideoCapture` 的多态寻址；常作为新项目的起点骨架。

#### 注意事项

- `atoi` 对非数字字符串返回 `0`，若字符串文件打开失败会误尝试摄像头 0；

- 序列掩码需含 `%0Nd`，否则当作单个文件处理。

#### 应用场景

通用媒体播放器骨架、多源输入工具、算法演示的统一入口。

### 8.3.14 `videowriter_basic.cpp` —— 摄像头帧编码写出

> **源文件**：`samples/cpp/videowriter_basic.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

从摄像头取流，用 `VideoWriter` 以 MJPG 编码、25 FPS 写入 `live.avi`，同时窗口实时预览，构成“采集 → 编码 → 写盘 → 显示”的完整闭环。

#### 核心原理

**30 秒心智模型**：`VideoWriter` 是 `VideoCapture` 的镜像——一个把 `Mat` 流编码成视频文件。四个要素缺一不可：编码器、帧率、帧尺寸、是否彩色。

先 `cap >> src` 取一帧确定尺寸与类型（`isColor = src.type() == CV_8UC3`），再

```cpp
writer.open(filename,
            VideoWriter::fourcc('M','J','P','G'),
            fps, src.size(), isColor);
```

`fourcc('M','J','P','G')` 用四个字符拼出编码标识。循环 `cap.read` + `writer.write`（等价 `writer << src`），析构自动关闭容器。

#### 关键 API

- `VideoWriter::fourcc('M','J','P','G')`；

- `VideoWriter::open(filename, codec, fps, size, isColor)`；

- `writer.write(src)` / `writer << src`；

- `cap.read`、`imshow`、`waitKey`。

#### 处理流程

打开摄像头 → 取首帧得尺寸与类型 → 初始化 `VideoWriter`（MJPG/25FPS/`live.avi`）→ 循环 `read` → `write` 编码写盘 + `imshow` 预览 → 任意键退出。

#### 参数说明

| 参数        | 含义    | 典型范围/默认  | 调大/调小会怎样             |
| --------- | ----- | -------- | -------------------- |
| `fourcc`  | 编码器   | `MJPG` 等 | 需系统支持；不支持时 `open` 失败 |
| `fps`     | 输出帧率  | `25.0`   | 与采集帧率不匹配会音画/时间轴失配    |
| `isColor` | 彩色标志  | 由首帧类型决定  | 填错会导致颜色通道错乱或写失败      |
| 帧尺寸       | 输出分辨率 | 首帧尺寸     | 后续帧尺寸不一致会写坏文件        |

#### 关联与对比

与 8.3.17 的 `video-write` 相比，本示例固定 MJPG 且不处理像素；`fourcc` 与编码选择与 8.3.7 的 GStreamer 元素一脉相承。

#### 注意事项

- 编码器必须运行时可用，否则 `isOpened` 为假；

- 每帧尺寸/类型必须与 `open` 一致；

- 写盘失败常见于磁盘空间或编码器不支持。

#### 应用场景

录像存档、监控录制、屏幕/摄像头采集导出。

### 8.3.15 `openni_orbbec_astra.cpp` —— 跨设备深浅时间戳对齐

> **源文件**：`samples/cpp/tutorial_code/videoio/openni_orbbec_astra/openni_orbbec_astra.cpp` ｜ **所属模块**：`videoio` ｜ **示例类型**：`完整流程`

#### 功能概述

同时打开 OpenNI 深度流与 V4L2 彩色流，两个线程分别抓帧并带 `getTickCount` 时间戳入队，主线程按“时间差小于半帧周期”配对深浅帧，演示跨设备流的软件同步。

#### 核心原理

**30 秒心智模型**：两台设备各自吐帧、时间轴不同步。给每帧盖时间戳，再按“最接近的时刻”配对：时间差超过半帧就丢较旧的一帧，直到两帧落在同一时间窗。

配对判据：令

$$
T\_{max}=\frac{1}{2\cdot fps}
$$

为半帧周期，若 $t\_{depth}+T\_{max}\<t\_{color}$ 说明深度帧太旧，弹掉；若 $t\_{color}+T\_{max}\<t\_{depth}$ 说明彩色帧太旧，弹掉；否则配对成功，双双出队并显示。队列用 `std::list<Frame>` 并设 `maxFrames=64` 上限防止积压。

#### 关键 API

- `VideoCapture depthStream(CAP_OPENNI2_ASTRA)`、`VideoCapture colorStream(0, CAP_V4L2)`；

- `CAP_PROP_OPENNI2_MIRROR`、`CAP_PROP_FRAME_WIDTH/HEIGHT/FPS`；

- `getTickCount` 打时间戳；

- `std::thread` + `std::mutex` + `std::condition_variable` 实现双线程采集。

#### 处理流程

开双流并设 `640x480` → 两线程各自 `grab/retrieve` 入队并带时间戳 → 主线程等待双队非空 → 按半帧阈值弹旧帧配对 → 深度 `convertTo` 归一化 + `COLORMAP_OCEAN` 显示 → ESC 结束并 `join` 线程。

#### 参数说明

| 参数          | 含义     | 典型范围/默认          | 调大/调小会怎样          |
| ----------- | ------ | ---------------- | ----------------- |
| `maxFrames` | 每流缓冲上限 | `64`             | 过小易丢帧，过大增加延迟      |
| 半帧阈值 `Tmax` | 允许时间差  | $1/(2\cdot fps)$ | 越大容错越强但配对越不“同时”   |
| 帧率          | 流输出帧率  | 设备能力             | 决定半帧阈值，也影响队列积压速度  |
| 镜像开关        | 深度镜像   | `0`（关）           | 与彩色镜像不一致会导致左右翻转错位 |

#### 关联与对比

与 8.3.10/8.3.11/8.3.12 的单设备多流不同，本示例解决“两台独立设备”的同步，是把多深度流接入统一时间轴的通用范式。时间戳与内参共同支撑[第 7 章](./ch07_calib3d_stitching.md)的深度融合。

#### 注意事项

- 依赖线程支持与 `CAP_OPENNI2_ASTRA`、`CAP_V4L2` 后端；

- 时间戳用本地 `getTickCount`，两台设备时钟必须一致（同一进程内满足）；

- 深度单位毫米、彩色为 `CV_8UC3`，配对后仍需各自归一化显示。

#### 应用场景

RGB-D 配准、多相机同步采集、3D 重建与 SLAM 前端、手势识别。

### 8.3.16 `video-input-psnr-ssim.cpp` —— 视频 PSNR 与 MSSIM

> **源文件**：`samples/cpp/tutorial_code/videoio/video-input-psnr-ssim/video-input-psnr-ssim.cpp` ｜ **所属模块**：`videoio` + `imgproc` ｜ **示例类型**：`完整流程`

#### 功能概述

逐帧比较参考视频与测试视频，用 PSNR 度量整体像素误差；当 PSNR 低于触发值时再用 MSSIM 逐通道输出结构相似度，检验编码/处理前后的质量损失。

#### 核心原理

**30 秒心智模型**：PSNR 是“误差的数学化”，MSSIM 是“人眼感知的相似度”。前者看均方误差，后者比较亮度、对比度、结构三者的联合匹配。

PSNR 由均方误差定义：

$$
MSE=\frac{1}{3\cdot WH}\sum\_{ch}\sum\_{x,y}\big(I\_1-I\_2\big)^2,\qquad
PSNR=10\log\_{10}\frac{255^2}{MSE}\ \text{dB}.
$$

实现用 `absdiff` → 平方 → `sum` 求 SSE。SSIM 逐像素/通道：

$$
SSIM(x,y)=\frac{(2\mu\_1\mu\_2+C\_1)(2\sigma\_{12}+C\_2)}
{(\mu\_1^2+\mu\_2^2+C\_1)(\sigma\_1^2+\sigma\_2^2+C\_2)},
$$

其中 $\mu,\sigma$ 用 `GaussianBlur(11x11, 1.5)` 估计，常数 $C\_1=6.5025,\ C\_2=58.5225$（对应动态范围 255）。`getMSSIM` 返回逐通道均值 `Scalar`。

#### 关键 API

- `absdiff`、`mul`、`sum`、`log10`；

- `GaussianBlur`、`mean`、`divide`；

- `VideoCapture` 双路读取、`CAP_PROP_FRAME_WIDTH/HEIGHT/COUNT`；

- `moveWindow`、`imshow`、`waitKey(delay)`。

#### 处理流程

校验两视频尺寸一致 → 逐帧 `>>` 取双路帧 → `getPSNR` 打印 dB → 若 `psnr < trigger` 则 `getMSSIM` 打印 B/G/R 相似度 → 双窗口显示并 `waitKey(delay)` → 帧空结束。

#### 参数说明

| 参数                     | 含义         | 典型范围/默认          | 调大/调小会怎样               |
| ---------------------- | ---------- | ---------------- | ---------------------- |
| `PSNR_Trigger`         | MSSIM 触发阈值 | 如 `30` dB        | 高于阈值只算 PSNR；低于才补 MSSIM |
| `Wait_Between_Frames`  | 帧间隔        | 毫秒               | 越大播放越慢；`0` 连续          |
| `GaussianBlur(11,1.5)` | 均值/方差估计窗口  | 固定               | 窗口决定 SSIM 的局部性，越大越平滑   |
| `C1/C2`                | SSIM 稳定常数  | `6.5025/58.5225` | 防止除零；随动态范围调整           |

#### 关联与对比

PSNR/MSSIM 是质量评测标准工具，同一算法在 8.5.1 被移植到 CUDA 并对比性能；SSIM 的局部窗口统计与[第 2 章](./ch02_imgproc.md)高斯滤波同源。

#### 注意事项

- 两视频必须同尺寸、同通道，否则逐像素比较无意义；

- 极小 SSE 时 PSNR 趋向无穷，示例返回 `360` 作上限；

- MSSIM 需 `CV_32F` 精度，8 位直接运算会溢出。

#### 应用场景

编码器质量评测、超分/去噪算法验证、视频处理回归测试。

### 8.3.17 `video-write.cpp` —— 通道提取与视频重编码

> **源文件**：`samples/cpp/tutorial_code/videoio/video-write/video-write.cpp` ｜ **所属模块**：`videoio` + `core` ｜ **示例类型**：`完整流程`

#### 功能概述

读取视频，把 R/G/B 某一通道保留、其余两通道清零（`split` + `merge`），再以原编解码或 `fourcc=-1` 交互选择编码器写成新视频。

#### 核心原理

**30 秒心智模型**：把彩色视频“只留一个颜色分量”，再重新编码。`split` 抽出三张单通道图，把不要的通道填零，`merge` 拼回，`VideoWriter` 写盘。

通道选择：`switch(argv[2][0])` 把 `R/G/B` 映射到通道索引 `2/1/0`；对 `i != channel` 的通道用 `Mat::zeros` 清零。`fourcc` 从输入视频 `CAP_PROP_FOURCC` 读回，用位运算拆成四字符；`askOutputType=='Y'` 时用 `ex=-1` 弹出系统编码器选择对话框。

#### 关键 API

- `VideoCapture`、`CAP_PROP_FOURCC/FRAME_WIDTH/HEIGHT/FPS/FRAME_COUNT`；

- `split` / `merge`（见 8.2.3/8.2.4）；

- `VideoWriter::open`、`outputVideo << res`。

#### 处理流程

解析参数（源、通道、是否询问编码器）→ 打开输入并取 `fourcc/尺寸/FPS` → 初始化输出（原码或 `-1` 选择）→ 循环 `>> src` → `split` → 保留通道其余清零 → `merge` → `outputVideo << res` → 帧空结束。

#### 参数说明

| 参数         | 含义      | 典型范围/默认       | 调大/调小会怎样         |
| ---------- | ------- | ------------- | ---------------- |
| `R/G/B` 通道 | 保留的颜色分量 | `R=2/G=1/B=0` | 决定输出是红/绿/蓝单色视频   |
| `fourcc`   | 输出编码    | 继承输入或 `-1` 选择 | 选择不同编码器影响兼容与码率   |
| `Y/N`      | 是否询问编码器 | 询问时弹出对话框      | `N` 用原码直接写，自动化友好 |
| 输出帧率/尺寸    | 写盘参数    | 继承输入          | 不一致会写坏文件         |

#### 关联与对比

`split/merge` 通道操作与 8.2.3/8.2.4 呼应；视频写出与 8.3.14 同为 `VideoWriter`，区别是这里含像素处理。是“读 → 处理 → 写”视频管线的最小完整示例。

#### 注意事项

- `fourcc=-1` 依赖 GUI 编码器对话框，无头环境需用具体编码；

- 保留通道为零会丢失其余颜色信息，属预期行为；

- 输入帧尺寸/类型与 `VideoWriter` 初始化必须一致。

#### 应用场景

单通道视频导出、医学伪彩预加工、教学演示颜色分量、视频格式转换。

***

## 8.4 G-API

### 8.4.1 `api_ref_snippets.cpp` —— 图构造、编译与执行接口

> **源文件**：`samples/cpp/tutorial_code/gapi/doc_snippets/api_ref_snippets.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`snippet`

#### 功能概述

汇编 G-API 核心接口：`GMat` 图构造、`GComputation` 捕获与 `apply` 执行、`compile` 预编译、`GCompileArgs` 内核包、序列化（`serialize/deserialize`）与自定义编译参数。

#### 核心原理

**30 秒心智模型**：先“画一张算子图”，再“决定用什么实现跑”。图构造阶段不知道也不关心后端，执行阶段通过 `compile_args` 注入 kernel 包与推理参数。

图用 `GMat` 声明式拼接，例如 Sobel 幅值图：

```cpp
cv::GMat in;
cv::GMat gx = cv::gapi::Sobel(in, CV_32F, 1, 0);
cv::GMat gy = cv::gapi::Sobel(in, CV_32F, 0, 1);
cv::GMat g  = cv::gapi::sqrt(cv::gapi::mul(gx,gx) + cv::gapi::mul(gy,gy));
cv::GMat out = cv::gapi::convertTo(g, CV_8U);
cv::GComputation sobelEdge(cv::GIn(in), cv::GOut(out));
```

执行分即时（`apply(gin, gout)`，每次编译）与预编译（`compile(descr_of(...))` 后复用 `GCompiled`）。`cv::gapi::combine` 拼装 Fluid/CPU kernel 包，`cv::gapi::kernels<...>()` 声明自定义 kernel。`s11n` 命名空间支持把图、元数据、运行参数与自定义编译参数序列化到字节流。

#### 关键 API

- `GMat`、`GComputation`、`GIn/GOut`；

- `apply`、`compile`、`GCompiled`；

- `cv::gapi::combine`、`cv::gapi::kernels<T...>()`、`compile_args`；

- `cv::gapi::serialize` / `deserialize`、`cv::gapi::bind`；

- `cv::detail::CompileArgTag`：自定义编译参数的标记特化。

#### 处理流程

声明式拼图（Sobel 梯度幅值）→ `GComputation` 捕获边界 → 即时 `apply` 出结果 → 用 Fluid kernel 包 `compile_args` 重执行 → 示例化子图捕获、`GCompiled` 预编译与序列化往返。

#### 参数说明

| 参数             | 含义     | 典型范围/默认      | 调大/调小会怎样               |
| -------------- | ------ | ------------ | ---------------------- |
| `ddepth`       | 中间深度   | `CV_32F`     | 浮点保留梯度幅值精度，`CV_8U` 会截断 |
| kernel 包       | 后端实现集合 | Fluid/CPU/推理 | 决定每个算子的实际实现与执行后端       |
| `compile_args` | 编译期参数  | 内核/网络包       | 注入错误或不完整包会导致编译失败       |
| 序列化格式          | 字节流编码  | 库内部          | 用于跨进程/跨进程边界传递图         |

#### 关联与对比

是理解 8.4.2–8.4.8 的基础；`GMat` 声明式图与命令式 OpenCV 调用形成对比，可回看[第 1 章](./ch01_core.md)的逐算子调用方式。`compile` 预编译与 8.4.2 的流式执行互补。

#### 注意事项

- 图构造是“表达式”，真正计算发生在 `apply/compile`；

- `descr_of` 描述输入元数据用于预编译，错误会改变编译结果；

- 自定义编译参数需特化 `CompileArgTag`，否则序列化/查询失效。

#### 应用场景

异构管线原型、图级性能优化、跨进程任务分发、后端无关的算法封装。

### 8.4.2 `dynamic_graph_snippets.cpp` —— 动态 I/O 与流式拉取

> **源文件**：`samples/cpp/tutorial_code/gapi/doc_snippets/dynamic_graph_snippets.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`snippet`

#### 功能概述

演示可变的图输入/输出：用 `GIn()/GOut()` 在运行时按条件增减端口，配合 `gin()/gout()` 动态装配运行参数，并以 `compileStreaming` 建立流式管线 `start/pull/stop`。

#### 核心原理

**30 秒心智模型**：普通 `apply` 是“一把抓进去全算完”，流式是“搭好管线后源源不断拉结果”。这里还允许“端口数量运行时才定”，图定义与数据装配解耦。

`GIn()/GOut()` 是可增长的协议容器，按条件 `ins += cv::GIn(in1)` 追加端口；`gin()/gout()` 同理追加运行期 `Mat` 与输出目标。流式执行：

```cpp
auto stream = cc.compileStreaming(cv::compile_args(cv::gapi::imgproc::cpu::kernels()));
stream.setSource(std::move(in_vector));
stream.start();
stream.pull(std::move(out_vector));
stream.stop();
```

`compileStreaming` 让图以后端管线方式运行，`pull` 阻塞取回一帧结果。

#### 关键 API

- `GIn()/GOut()`、`gin()/gout()`；

- `compileStreaming`、`setSource`、`start`、`pull`、`stop`；

- `cv::gapi::imgproc::cpu::kernels()` 等后端 kernel 包。

#### 处理流程

按布尔条件构建动态端口集 → 用 `gin/gout` 装配运行期数据 → `compileStreaming` 生成流式对象 → `setSource` → `start` → 循环 `pull` 取结果 → `stop`。

#### 参数说明

| 参数       | 含义         | 典型范围/默认            | 调大/调小会怎样        |
| -------- | ---------- | ------------------ | --------------- |
| 端口条件     | 是否启用某输入/输出 | 运行时布尔              | 决定图边界与数据装配数量    |
| kernel 包 | 执行后端       | `cpu::kernels()` 等 | 流式后端选择决定吞吐与延迟   |
| 源数据      | 输入流        | `gin(...)` 集合      | 与图端口数量不一致会运行期报错 |

#### 关联与对比

与 8.4.1 的静态图相比，这里强调“运行时可变边界 + 流式拉取”；流式模式在 8.4.4/8.4.8 的 `compileStreaming` 推理流水线中真正落地。

#### 注意事项

- 动态端口的 `ins/outs` 与运行期 `gin/gout` 必须一一对应；

- 流式对象需 `setSource` 后才能 `start`，`pull` 返回 `false` 表示流结束；

- 端口在编译期不确定时，`compile` 无法预编译，需流式或运行时 `apply`。

#### 应用场景

参数化管线（可选处理分支）、实时流式处理、按需输入输出的服务封装。

### 8.4.3 `kernel_api_snippets.cpp` —— 自定义 operation 与 CPU kernel

> **源文件**：`samples/cpp/tutorial_code/gapi/doc_snippets/kernel_api_snippets.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`snippet`

#### 功能概述

展示 G-API 的“操作（operation）与实现（kernel）分离”：用 `G_TYPED_KERNEL` 声明接口与元数据推导，用 `GAPI_OCV_KERNEL` 提供 CPU 实现，并用 `GAPI_COMPOUND_KERNEL` 把多个算子组合成一个复合操作（Harris 角点）。

#### 核心原理

**30 秒心智模型**：G-API 里“算子长什么样”和“算子怎么算”是两回事。`G_TYPED_KERNEL` 只定义输入输出签名与 `outMeta`（推导输出形状），`GAPI_OCV_KERNEL` 才写真正的 `cv::filter2D` 实现。

`GFilter2D` 声明了签名 `<GMat(GMat,int,Mat,Point,double,int,Scalar)>` 与 `outMeta`（`in.withDepth(ddepth)`），`GCPUFilter2D` 在 `run` 里调用 `cv::filter2D`。复合内核 `GAPI_COMPOUND_KERNEL(GFluidHarrisCorners, HarrisCorners)` 在 `expand` 里把 `HarrisResponse` + `ArrayNMS` 组合成一个对外 operation，实现复用与后端内局部优化。

#### 关键 API

- `G_TYPED_KERNEL`：声明 operation 与 `outMeta`；

- `GAPI_OCV_KERNEL`：CPU 实现，`run` 是实际执行体；

- `GAPI_COMPOUND_KERNEL`：复合实现；

- `G_API_OP`/`G_API_OP_M`：多输入多输出的操作声明（见 8.4.4）。

#### 处理流程

用 `G_TYPED_KERNEL` 声明 `GFilter2D` 签名 → 用 `GAPI_OCV_KERNEL` 绑定 `cv::filter2D` 实现 → 声明 Harris 复合内核，把响应计算与 NMS 串起来 → 在图上 `GFilter2D::on(...)` 或包装函数调用。

#### 参数说明

| 参数        | 含义     | 典型范围/默认       | 调大/调小会怎样           |
| --------- | ------ | ------------- | ------------------ |
| `outMeta` | 输出形状推导 | 返回 `GMatDesc` | 推导错误会导致下游形状不匹配     |
| `ddepth`  | 输出深度   | `-1` 或指定      | 决定输出像素类型与数值范围      |
| 复合展开      | 内部算子组合 | 任意 DAG        | 决定复合操作的内部执行顺序与可优化性 |

#### 关联与对比

是 8.4.4/8.4.5/8.4.8 自定义后处理 kernel 的基础（如 `PostProc`、`ProcessDetections`）；复合内核展示了把“多算子 + 后端局部实现”封装成单 operation 的方法。

#### 注意事项

- `G_TYPED_KERNEL` 的签名必须与 `outMeta` 参数一一对应；

- `GAPI_OCV_KERNEL` 的 `run` 参数顺序与 operation 签名一致，最后一个是输出；

- 复合内核里的子 operation 也要有对应 kernel 实现，否则编译失败。

#### 应用场景

自定义算子封装、后端局部优化、把多步处理打包成可复用 G-API 操作。

### 8.4.4 `age_gender_emotion_recognition.cpp` —— 多模型人脸属性图

> **源文件**：`samples/cpp/tutorial_code/gapi/age_gender_emotion_recognition/age_gender_emotion_recognition.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`完整流程`

#### 功能概述

用 G-API 编排“人脸检测 → 年龄/性别 → 情绪”三个推理模型：先在整帧上跑 SSD 人脸检测，再对每个 ROI 跑属性模型，流式（`compileStreaming`）或串行（`ser`）执行并叠加标注。

#### 核心原理

**30 秒心智模型**：把“多个 DNN + 后处理”画成一张图：`infer<Faces>(in)` 出检测 blob，自定义 `PostProc` 解析出 `GArray<Rect>`，再用 ROI 列表式 `infer<AgeGender>(faces, in)` 与 `infer<Emotions>(faces, in)` 逐脸分类。

网络用 `G_API_NET` 声明为“图上的操作”：

```cpp
G_API_NET(Faces, <cv::GMat(cv::GMat)>, "face-detector");
using AGInfo = std::tuple<cv::GMat, cv::GMat>;
G_API_NET(AgeGender, <AGInfo(cv::GMat)>, "age-gender-recoginition");
```

SSD 输出是 `1x1x200x7` 的 blob，`PostProc` 解析 `[image_id, class, confidence, x1,y1,x2,y2]`，按 `confidence >= 0.5` 过滤并换算成原图像素 ROI。`cv::gapi::ie::Params<T>` 给每个网络配置 IR 路径与设备，`cfgOutputLayers` 指定年龄/性别输出层。

#### 关键 API

- `G_API_NET`：声明网络类型；

- `cv::gapi::infer<Faces>(in)`（整帧）与 `infer<AgeGender>(faces, in)`（ROI 列表）；

- `G_API_OP`/`GAPI_OCV_KERNEL`：自定义 `PostProc`；

- `cv::gapi::ie::Params<T>{xml, bin, device}`、`cfgOutputLayers`；

- `compileStreaming`、`GCaptureSource`、`pull/try_pull`。

#### 处理流程

定义三个网络与 `PostProc` → 构图：`infer<Faces>` → `PostProc` 出 ROI → `infer<AgeGender>` / `infer<Emotions>` → 配置三个 `ie::Params` → `kernels+networks` 编译 → 流式 `start/pull` 循环取结果 → `DrawResults` 画框与年龄/性别/情绪文字 → 打印 FPS。

#### 参数说明

| 参数                | 含义       | 典型范围/默认       | 调大/调小会怎样         |
| ----------------- | -------- | ------------- | ---------------- |
| 置信度阈值             | SSD 过滤阈值 | 硬编码 `0.5`     | 越高误检越少，越低召回越高    |
| `MAX_PROPOSALS`   | 最多候选框    | `200`         | 限制解析的 SSD 输出条数   |
| 网络设备              | IE 执行设备  | `fdw/fdd` 等参数 | CPU/GPU 选择决定推理耗时 |
| `cfgOutputLayers` | 输出层名     | 按模型指定         | 层名错误会取不到正确输出     |
| `ser` 开关          | 串行/流水线   | `false`       | 串行便于调试，流水线吞吐更高   |

#### 关联与对比

与 8.4.8 的车辆/车牌同属“检测 + 逐 ROI 推理”图结构；`G_API_NET` + `infer` 把 DNN 当普通算子接入图。对比命令式 DNN 调用（见[第 6 章目标检测](./ch06_objdetect_photo.md)），G-API 版本自动处理 ROI 裁剪与后端调度。

#### 注意事项

- 需要 G-API 的 IE（OpenVINO）后端与对应模型 IR；

- 年龄/性别/情绪模型输出层需用 `cfgOutputLayers` 显式指定；

- 流式 `try_pull` 需与 UI 刷新节奏配合，避免阻塞主循环。

#### 应用场景

客流画像、智能广告屏、安防人脸属性分析、边缘端多模型推理。

### 8.4.5 `face_beautification.cpp` —— 推理与自定义美颜 kernel 图

> **源文件**：`samples/cpp/tutorial_code/gapi/face_beautification/face_beautification.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`完整流程`

#### 功能概述

用 G-API 实现“人脸美颜”：人脸检测 + 关键点模型定位面部，再以自定义 `bilateralFilter`、`Laplacian` 等 kernel 对脸部区域做磨皮，流式处理摄像头/视频并输出美颜结果。

#### 核心原理

**30 秒心智模型**：先用 DNN 找到脸和五官，再用“保边磨皮”只处理脸部：`bilateralFilter` 平滑纹理但保留边缘，配合 `Laplacian` 细节增强/去瑕疵，最终只把处理结果合成回脸部区域。

G-API 侧：`G_API_NET(FaceDetector, ...)` 与 `G_API_NET(LandmDetector, ...)` 声明两个网络；自定义 `GBilatFilter`、`GLaplacian` 用 `G_TYPED_KERNEL` 声明、CPU 实现绑定 `cv::bilateralFilter/cv::Laplacian`。配置常量：检测置信度 `kConfThresh=0.7`，高斯核 `kGKernelSize(5,5)`/`kGSigma=0.0`，双边 `kBSize=9`、`kBSigmaCol=kBSigmaSp=30`，USM 强度 `kUnshStrength=0.7`。

#### 关键 API

- `G_API_NET(FaceDetector, LandmDetector)`；

- `G_TYPED_KERNEL(GBilatFilter/GLaplacian, ...)` + CPU 实现；

- `cv::gapi::infer` 与 ROI 列表推理；

- 关键点 → 下巴/眼睛/额头椭圆轮廓（`getForeheadEllipse` 等）确定美化区域。

#### 处理流程

声明双网络与美颜 kernel → 构图：`infer<FaceDetector>` → `infer<LandmDetector>` 得关键点 → 由关键点生成面部椭圆 ROI → 对脸部做双边滤波 + 拉普拉斯细节处理 → `GComputation` 捕获 → 流式 `compileStreaming` 运行 → 输出美颜帧与可选标注。

#### 参数说明

| 参数                       | 含义     | 典型范围/默认         | 调大/调小会怎样            |
| ------------------------ | ------ | --------------- | ------------------- |
| `kConfThresh`            | 检测置信度  | `0.7`           | 越高漏检越多、误检越少         |
| `kBSize`/`kBSigmaCol/Sp` | 双边滤波参数 | `9` / `30`/`30` | 空间/颜色 σ 越大磨皮越强但边缘越糊 |
| `kUnshStrength`          | 细节增强强度 | `0.7`           | 越大细节/瑕疵越明显，过强不自然    |
| 美化区域                     | 面部 ROI | 由关键点椭圆决定        | 区域外不处理，保证背景不失真      |

#### 关联与对比

与 8.4.4 共用“检测 + ROI 处理”模式，区别是这里推理结果驱动自定义图像 kernel 而非分类。`bilateralFilter`/`Laplacian` 的数学见[第 2 章](./ch02_imgproc.md)滤波与边缘。

#### 注意事项

- 需 IE 后端与检测/关键点模型；

- 美化区域由关键点推导，脸部姿态极端时区域错位；

- 双边滤波计算量大，低端设备需在磨皮强度与帧率间取舍。

#### 应用场景

实时视频美颜、直播滤镜、拍照后期、边缘端人像增强。

### 8.4.6 `porting_anisotropic_image_segmentation_gapi.cpp` —— 结构张量 G-API 移植

> **源文件**：`samples/cpp/tutorial_code/gapi/porting_anisotropic_image_segmentation/porting_anisotropic_image_segmentation_gapi.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`完整流程`

#### 功能概述

把“各向异性图像分割”算法移植为 G-API 图：用 `calcGST` 计算梯度结构张量，得到一致性（coherency）与方向（orientation）图，再阈值二值化叠加到原图输出。

#### 核心原理

**30 秒心智模型**：把每个像素的梯度聚成一个 $2\times2$ 张量，张量的两个特征值之差衡量“这里是不是各向异性的纹理/边缘”，特征向量给出方向。

先算梯度：

$$
J\_{11}=\overline{G\_x^2},\quad J\_{22}=\overline{G\_y^2},\quad J\_{12}=\overline{G\_xG\_y},
$$

（$\overline{\cdot}$ 为窗口盒滤波）。特征值

$$
\lambda\_{1,2}=\frac{(J\_{11}+J\_{22})\pm\sqrt{(J\_{11}-J\_{22})^2+4J\_{12}^2}}{2},
$$

一致性 $C=\dfrac{\lambda\_1-\lambda\_2}{\lambda\_1+\lambda\_2}$ 接近 1 表示强各向异性，方向 $\theta=\tfrac12\arctan\frac{2J\_{12}}{J\_{22}-J\_{11}}$。示例用阈值 $C>C\_{Thr}$ 且 `inRange(orientation, LowThr, HighThr)` 提取目标纹理，再 `addWeighted` 叠加。

#### 关键 API

- `cv::gapi::Sobel`、`mul`、`boxFilter`、`sqrt`、`phase`；

- `cv::gapi::inRange`、`addWeighted`、`normalize`；

- `GComputation`、`GIn/GOut`、`apply`。

#### 处理流程

声明 `calcGST` 子图（Sobel → 平方 → 盒滤波 → 特征值）→ 主图组合一致性/方向二值化 → `GComputation` 捕获三路输出 → `apply` 运行 → 保存 `result/Coherency/Orientation` 三张图。

#### 参数说明

| 参数               | 含义    | 典型范围/默认         | 调大/调小会怎样           |
| ---------------- | ----- | --------------- | ------------------ |
| `W`              | 盒滤波窗口 | `52`            | 越大统计窗口越平滑，细小纹理被平均掉 |
| `C_Thr`          | 一致性阈值 | `0.43`          | 越高只保留强各向异性区域       |
| `LowThr/HighThr` | 方向阈值  | `35/57`（0–180°） | 决定保留的纹理方向带         |
| `addWeighted` 权重 | 叠加比例  | `0.5/0.5`       | 控制分割掩膜在原图上的可见度     |

#### 关联与对比

与 8.4.7 是同一算法两种执行后端（默认 vs Fluid）；`calcGST` 的结构张量思想与[第 3 章特征](./ch03_features.md)的角点响应同源，也与[第 7 章](./ch07_calib3d_stitching.md)的纹理分析相关。

#### 注意事项

- 一致性/方向图需 `normalize` 到 8 位再保存，否则不可见；

- 方向用 `phase` 输出，阈值在 0–180° 内定义；

- 窗口 `W` 与阈值需按图像纹理尺度联合调。

#### 应用场景

纹理方向分析、各向异性分割、织物/细胞图像结构提取。

### 8.4.7 `porting_anisotropic_image_segmentation_gapi_fluid.cpp` —— Fluid 流水线执行

> **源文件**：`samples/cpp/tutorial_code/gapi/porting_anisotropic_image_segmentation/porting_anisotropic_image_segmentation_gapi_fluid.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`完整流程`

#### 功能概述

在 8.4.6 的同一算法上，把执行后端切换为 Fluid：用 `cv::gapi::combine(core::fluid::kernels(), imgproc::fluid::kernels())` 组装内核包，并通过 `remove<GBoxFilter>` 让不合适的算子回落到 OpenCV。

#### 核心原理

**30 秒心智模型**：图不变，只是“用哪个实现跑”变了。Fluid 以“逐行流式 + 行缓冲”方式执行，省去中间整图分配；不适合 Fluid 的算子可回退到通用实现。

Fluid 内核包通过 `combine` 组装，`compile_args(fluid_kernels)` 传给 `apply`：

```cpp
cv::GKernelPackage fluid_kernels = cv::gapi::combine(
    cv::gapi::core::fluid::kernels(),
    cv::gapi::imgproc::fluid::kernels());
fluid_kernels.remove<cv::gapi::imgproc::GBoxFilter>(); // 回退到 OpenCV
segm.apply(cv::gin(imgIn), cv::gout(...), cv::compile_args(fluid_kernels));
```

`remove<T>` 从包里剔除某个内核，G-API 会自动为该算子寻找其他实现（这里是 OpenCV 的盒滤波），实现“混合执行”。

#### 关键 API

- `cv::gapi::core::fluid::kernels()`、`cv::gapi::imgproc::fluid::kernels()`；

- `cv::gapi::combine`、`GKernelPackage::remove<T>`；

- `apply(gin, gout, compile_args(...))`。

#### 处理流程

与 8.4.6 相同的图 → 组装 Fluid 内核包 → 移除 `GBoxFilter`（回退 OpenCV）→ 带 `compile_args` 执行 → 保存三路结果图。

#### 参数说明

| 参数          | 含义           | 典型范围/默认              | 调大/调小会怎样              |
| ----------- | ------------ | -------------------- | --------------------- |
| 内核包内容       | 可用的 Fluid 实现 | `core+imgproc` Fluid | 缺少某算子会自动回退，但性能/精度可能不同 |
| `remove<T>` | 剔除的算子        | `GBoxFilter`         | 剔除后该算子回退到其他后端实现       |
| 输出路数        | 图边界输出        | 3 路                  | 决定 `gout` 数量与保存文件     |

#### 关联与对比

与 8.4.6 唯一差别在后端选择，直接对比体现 G-API“一次构图、多后端执行”的价值；Fluid 的行流式思想与 8.4.2 的流式执行互补。

#### 注意事项

- 非所有算子都有 Fluid 实现，缺失算子会自动回退；

- `remove<T>` 后 G-API 必须有替代实现，否则编译失败；

- Fluid 输出与 OpenCV 在边界/浮点细节上可能有微小差异。

#### 应用场景

低内存嵌入式推理、逐行流水线优化、跨后端算法移植评估。

### 8.4.8 `security_barrier_camera.cpp` —— 车辆与车牌级联推理

> **源文件**：`samples/cpp/tutorial_code/gapi/security_barrier_camera/security_barrier_camera.cpp` ｜ **所属模块**：`gapi` ｜ **示例类型**：`完整流程`

#### 功能概述

把安防卡口的“车辆检测 → 车辆颜色/类型 → 车牌识别”三段模型串成一张 G-API 数据流图：先在同一帧上跑 SSD 检测同时输出车辆框与车牌框，再分别对车辆 ROI 推理颜色与类型、对车牌 ROI 推理车牌字符序列，最后把结果与 FPS 画回原帧。支持串行（`ser`）与流式（默认）两种执行模式。

#### 核心原理

**30 秒心智模型**：`infer<VehicleLicenseDetector>(in)` 用一个 blob 同时出两类框，自定义 `ProcessDetections` 把它拆成车辆与车牌两个 `GArray<Rect>`，随后 `infer<VehicleAttributes>(vehicles, in)` 与 `infer<LPR>(plates, in)` 按 ROI 列表并行推理，图的输出是 `(frame, vehicles, colors, types, plates, numbers)`。

网络用 `G_API_NET` 声明，多输出自定义操作用 `G_API_OP_M` 声明、`GAPI_OCV_KERNEL` 提供 CPU 实现：

```cpp
G_API_NET(VehicleLicenseDetector, <cv::GMat(cv::GMat)>, "vehicle-license-plate-detector");
using GVehiclesPlates = std::tuple<cv::GArray<cv::Rect>, cv::GArray<cv::Rect>>;
G_API_OP_M(ProcessDetections, <GVehiclesPlates(cv::GMat, cv::GMat)>, "custom.security_barrier.detector.postproc") { ... };
```

SSD 输出是 `200×7` 的记录，逐条解析 `[image_id, label, confidence, x1,y1,x2,y2]`：`label==1` 归入车辆，`label==2` 外扩 `15px` 后归入车牌，`confidence<0.5` 跳过，`image_id<0` 表示列表结束。车牌字符通过输出向量索引 `labels::license_text` 得到；`LPR` 网络需要一个常量输入 `seq_ind`（`88×1`，首元素 0 其余 1），用 `constInput` 注入。

#### 关键 API

- `G_API_NET`、`G_API_OP_M`、`GAPI_OCV_KERNEL`：声明网络与多输出自定义操作；

- `cv::gapi::infer<VehicleLicenseDetector>(in)`、`infer<VehicleAttributes>(vehicles, in)`、`infer<LPR>(plates, in)`；

- `cv::gapi::ie::Params<T>{ir, weights, device}` 与 `cfgOutputLayers`（输出 `color`/`type`）、`constInput("seq_ind", mat)`；

- `compile`/`compileStreaming`、`setSource(GCaptureSource)`、`pull`/`try_pull`、`start/stop`。

#### 处理流程

`CommandLineParser` 读输入与九个模型/设备参数 → 定义三个网络与 `ProcessDetections` 图 → 配置 `det_net/attr_net/lpr_net` 三个 `ie::Params` → `kernels + networks` 编译 → 串行用 `compile` + `cap >> frame`；流式用 `compileStreaming` + `GCaptureSource` + `start` 后 `pull`（纯基准）或 `try_pull`（带 UI）循环 → `DrawResults` 画车辆（颜色/类型）与车牌（字符）→ `DrawFPS` → `imshow`，最后打印处理帧数与平均耗时。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `confidence` 阈值 | SSD 后处理过滤 | 硬编码 `0.5` | 越大框越少越准，越小召回越高但误检变多 |
| `MAX_PROPOSALS` | 最多解析的 SSD 输出条数 | `200` | 限制候选数，影响后处理耗时 |
| `detd/vehd/lprd` | 各模型执行设备 | `CPU`/`GPU` 等 | 决定推理吞吐与延迟 |
| `cfgOutputLayers` | 属性网络输出层 | `{"color","type"}` | 层名与模型不符会取不到正确输出 |
| `seq_ind` | LPR 网络常量输入 | `88×1` | 需与模型输入规格一致，否则推理失败 |
| `ser` | 串行/流水线 | `false`（流式） | 串行便于调试，流式吞吐更高 |
| `pure` | 纯基准模式 | `false` | 打开后不做 `imshow`，用阻塞 `pull` 最大化吞吐 |

#### 关联与对比

与 8.4.4 同属“检测 + 逐 ROI 属性”图结构，但这里一个检测器同时输出两类框，并演示 `G_API_OP_M` 多输出操作与 `constInput` 常量注入。对比命令式 DNN 循环（见[第 6 章目标检测](./ch06_objdetect_photo.md)），G-API 自动完成 ROI 裁剪、多模型调度与流水线重叠。

#### 注意事项

- 需要 G-API 的 IE 后端与三个模型的 IR/权重文件，设备参数需与模型匹配；

- `ProcessDetections::outMeta` 目前仅返回空描述，依赖编译期 `GMatDesc` 匹配；

- 车牌字符向量以 `-1` 表示结束，索引越界会导致崩溃。

#### 应用场景

智慧交通卡口、停车场车牌识别、违章抓拍、多模型边缘推理流水线基准测试。

---

## 8.5 GPU/CUDA

### 8.5.1 `gpu-basics-similarity.cpp` —— PSNR/MSSIM 的 CPU 与 CUDA 对比

> **源文件**：`samples/cpp/tutorial_code/gpu/gpu-basics-similarity/gpu-basics-similarity.cpp` ｜ **所属模块**：`cudaarithm`/`cudafilters` ｜ **示例类型**：`性能对比`

#### 功能概述

对同一对图像分别用 CPU、基础 CUDA、缓冲复用 CUDA 三种实现计算 PSNR 与 MSSIM（平均结构相似度），各跑 `TIMES` 次取平均耗时与数值结果，直观展示“显存分配昂贵 → 复用缓冲 + 异步流”对性能的影响。程序接受 `参考图 对比图 运行次数` 三个命令行参数。

#### 核心原理

**30 秒心智模型**：同一个公式、三种写法——CPU 用 `Mat` 逐算子算；基础 CUDA 用 `cuda::GpuMat` 但每次调用都新建显存；优化版把中间 `GpuMat` 提为可复用缓冲并挂到同一个 `cuda::Stream` 上，省掉每次调用的分配与同步。

PSNR 的定义（`MSE` 为通道平均平方误差）：

$$
\text{PSNR}=10\log_{10}\frac{255^2}{\text{MSE}},\qquad
\text{MSE}=\frac{\sum_i\|I_1(i)-I_2(i)\|^2}{C\cdot N}
$$

MSSIM 先用 11×11、σ=1.5 的高斯模糊估计局部均值 `μ` 与方差/协方差 `σ`，逐像素算 SSIM 后取平均：

$$
\text{SSIM}=\frac{(2\mu_1\mu_2+C_1)(2\sigma_{12}+C_2)}
{(\mu_1^2+\mu_2^2+C_1)(\sigma_1^2+\sigma_2^2+C_2)},\quad C_1=6.5025,\ C_2=58.5225
$$

CUDA 版把 `absdiff/multiply/sum`、`GaussianBlur/split/addWeighted` 换成 `cuda::absdiff`、`cuda::multiply`、`cuda::sum`、`cuda::createGaussianFilter` 等；优化版复用 `BufferPSNR`/`BufferMSSIM` 中的 `GpuMat`，仅在求和前 `stream.waitForCompletion()`。

#### 关键 API

- `cuda::GpuMat::upload/download`：CPU↔显存传输；

- `cuda::absdiff`、`cuda::multiply`、`cuda::sum`、`cuda::addWeighted`、`cuda::divide`、`cuda::split`、`cuda::subtract`：逐算子 CUDA 实现；

- `cuda::createGaussianFilter(type, -1, Size(11,11), 1.5)`：创建可复用高斯滤波器；

- `cuda::Stream` + 各算子尾参 `stream`：异步流水线；

- `cv::getTickCount/getTickFrequency`：高精度计时。

#### 处理流程

读入两张图 → 分别对 PSNR 与 MSSIM 跑四组：CPU 版、基础 CUDA 版、优化版首次调用、优化版平均 `TIMES` 次 → 每组用 `getTickCount` 计时并打印毫秒与结果 → 对比三种实现的耗时。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `TIMES` | 重复运行次数 | 第 3 个命令行参数，默认 `10` | 越大平均耗时越稳，运行越久 |
| `C1`/`C2` | SSIM 稳定常数 | `6.5025`/`58.5225`（即 `(0.01·255)²`、`(0.03·255)²`） | 防除零；取值影响 SSIM 灵敏度 |
| 高斯核尺寸/σ | 均值与方差估计窗口 | `Size(11,11)`、σ=`1.5` | 窗口越大平滑越强，细节损失越多 |
| `BufferPSNR/MSSIM` | 显存复用缓冲 | 结构体成员 | 复用避免每次分配，是性能提升的关键 |

#### 关联与对比

把 8.3.16 的 PSNR/MSSIM 从视频质量评估搬到显存执行，是“同一算法 CPU → 基础 CUDA → 优化 CUDA”的移植范本；测速手段与 8.2.6 的 `TickMeter`、8.3.5 的采集 FPS 一脉相承。G-API 的异构后端（8.4）在框架层做类似调度，此处是手工级显存管理。

#### 注意事项

- 需要编译 OpenCV 的 CUDA 模块（`cudaarithm`、`cudafilters`）并有可用 GPU；

- `cuda::GpuMat` 与 `Mat` 不能混用，必须显式 `upload/download`；

- 优化版所有算子挂同一 `cuda::Stream`，取值前需 `waitForCompletion` 保证同步。

#### 应用场景

GPU 加速的图像质量评估、把 CPU 管线移植到 CUDA 的起步范本、显存管理与异步流优化学习。

---

## 8.6 本章小结

本章的四条主线可压缩为：

```text
HighGUI 窗口 + 滑动条 + 绘图原语：把算法参数变成“拖滑块看结果”
  → VideoIO：VideoCapture/VideoWriter 统一摄像头、文件、序列与音轨
  → G-API：声明式计算图，GMat 建图、compile/compileStreaming 执行、G_API_NET 接 DNN
  → GPU/CUDA：GpuMat 显存执行，复用缓冲 + Stream 异步化加速
```

验收时不要只看“跑没跑起来”：HighGUI 看回调是否按参数重算，VideoIO 看流属性与时间戳是否对齐，G-API 看图的算子与输出描述是否正确、流式是否吞吐更高，GPU 看三版实现的耗时差与结果是否一致。

