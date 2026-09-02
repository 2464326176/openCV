# 滤波与形态学处理流程（process）

本节是 [morphology](../morphology/README.md) 的**工程实践**：以 Trackbar 交互方式实时调节核大小，对比五种滤波与七种形态学运算的效果。对应官方示例 [Smoothing.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Smoothing/Smoothing.cpp) 与 [Morphology_2.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Morphology_2.cpp)。

本目录源码：[filter_process.cpp](filter_process.cpp)（五种滤波交互演示）、[morph_op_process.cpp](morph_op_process.cpp)（七种形态学交互演示）。

## 1. 核心功能

输入：一张图像（本例 `OIP.png`）。

输出：
- 五个独立窗口分别显示方框/均值/高斯/中值/双边滤波结果，各自带 `ksize` 滑条。
- 七个独立窗口分别显示七种形态学运算结果，各自带 `ksize` 滑条。

## 2. API 速查表

| API | 类型 | 关键参数 | 说明 |
|-----|------|----------|------|
| `boxFilter` | 线性滤波 | `ddepth`, `ksize`, `normalize` | 方框滤波，`ddepth=-1` 同输入深度 |
| `blur` | 线性滤波 | `ksize`, `anchor` | 均值滤波，`anchor=(-1,-1)` 为中心 |
| `GaussianBlur` | 线性滤波 | `ksize`(奇数), `sigmaX`, `sigmaY` | 高斯滤波，`sigma=0` 自动推算 |
| `medianBlur` | 非线性滤波 | `ksize`(奇数) | 中值滤波，去椒盐噪声 |
| `bilateralFilter` | 非线性滤波 | `d`, `sigmaColor`, `sigmaSpace` | 双边滤波，保边去噪 |
| `dilate` / `erode` | 形态学 | `kernel`, `anchor`, `iterations` | 膨胀/腐蚀 |
| `morphologyEx` | 形态学 | `op`, `kernel` | 七种形态学统一入口 |
| `getStructuringElement` | 形态学 | `shape`, `ksize` | 生成结构元 |
| `pyrUp` / `pyrDown` | 金字塔 | `dstsize` | 上下采样 |
| `threshold` | 阈值 | `thresh`, `maxval`, `type` | 固定阈值，可叠加 `THRESH_OTSU` |
| `adaptiveThreshold` | 阈值 | `maxValue`, `method`, `blockSize`, `C` | 自适应局部阈值 |

## 3. 代码逐段解析

### 3.1 五种滤波的 Trackbar 回调

来自 [filter_process.cpp](filter_process.cpp)——每个滤波器一个回调，滑条值直接作为核尺寸。

```cpp
int boxFilterValue = 6;
int meanFilterValue = 10;
int gaussianFilterValue = 6;
int medianFilterValue = 10;
int bilateralFilterValue = 10;

void on_boxFilterTrackbar(int, void*) {
    boxFilter(srcImage, boxFilterImage, -1, Size(boxFilterValue, boxFilterValue));
    imshow("box filter", boxFilterImage);
}

void on_gaussianFilterTrackbar(int, void*) {
    // 核尺寸必须为奇数：value*2+1 保证 1,3,5,...
    GaussianBlur(srcImage, gaussianFilterImage,
                 Size(gaussianFilterValue * 2 + 1, gaussianFilterValue * 2 + 1), 0, 0);
    imshow("gaussian filter", gaussianFilterImage);
}

void on_medianFilterTrackbar(int, void*) {
    medianBlur(srcImage, medianFilterImage, medianFilterValue * 2 + 1);  // ksize 必须为奇数
    imshow("median filter", medianFilterImage);
}

void on_bilateralFilterTrackbar(int, void*) {
    bilateralFilter(srcImage, bilateralFilterImage,
                    bilateralFilterValue,             // d
                    bilateralFilterValue * 2.0,       // sigmaColor
                    bilateralFilterValue / 2.0);      // sigmaSpace
    imshow("bilateral filter", bilateralFilterImage);
}
```

**参数技巧**：高斯/中值滤波的核尺寸必须为奇数，采用 `value * 2 + 1` 把滑条整数映射为奇数序列，避免非法参数。

### 3.2 窗口与滑条注册

```cpp
namedWindow("gaussian filter", WINDOW_AUTOSIZE);
createTrackbar("ksize", "gaussian filter", &gaussianFilterValue, 50, on_gaussianFilterTrackbar);
on_gaussianFilterTrackbar(0, nullptr);   // 手动触发一次，显示初始结果
```

`createTrackbar` 只在滑条变化时回调，因此注册后要**手动调用一次回调**完成首次渲染。

### 3.3 七种形态学的统一演示框架

来自 [morph_op_process.cpp](morph_op_process.cpp)——用结构体数组 + 回调 `userdata` 实现一套回调服务七个窗口。

```cpp
struct MorphDemo {
    const char* winName;
    int op;        // MORPH_DILATE / MORPH_ERODE / ...
    int ksize;
};

void on_morphTrackbar(int value, void* userdata) {
    auto* demo = static_cast<MorphDemo*>(userdata);
    demo->ksize = value;
    Mat kernel = getStructuringElement(MORPH_RECT,
                                       Size(kernelSize(value), kernelSize(value)));
    Mat dst;
    morphologyEx(srcImage, dst, demo->op, kernel);
    imshow(demo->winName, dst);
}

MorphDemo demos[] = {
    {"dilate image",   MORPH_DILATE,    15},
    {"erode image",    MORPH_ERODE,     15},
    {"open image",     MORPH_OPEN,      15},
    {"close image",    MORPH_CLOSE,     15},
    {"gradient image", MORPH_GRADIENT,  15},
    {"tophat image",   MORPH_TOPHAT,    15},
    {"blackhat image", MORPH_BLACKHAT,  15},
};

for (auto& demo : demos) {
    namedWindow(demo.winName, WINDOW_AUTOSIZE);
    createTrackbar("ksize", demo.winName, nullptr, 50, on_morphTrackbar, &demo);
    setTrackbarPos("ksize", demo.winName, demo.ksize);  // 设置滑条初始位置
}
```

**设计要点**：
1. `createTrackbar` 的最后一个参数 `userdata` 传入各窗口自己的 `MorphDemo*`，回调内转型后区分是哪个窗口。
2. `setTrackbarPos` 把滑条拨到结构体预设的初始值（如 15）。
3. `kernelSize(value) = max(value, 1)` 防止核尺寸退化为 0 导致崩溃。

## 4. 滤波选型指南

| 噪声类型 | 推荐滤波 | 理由 |
|----------|----------|------|
| 高斯噪声 | `GaussianBlur` | 与噪声模型匹配，保边较好 |
| 椒盐噪声 | `medianBlur` | 中位数对极端值鲁棒 |
| 需要保边的平滑 | `bilateralFilter` | 值域权重抑制跨边缘平滑 |
| 追求速度 | `blur` / `boxFilter` | 可分离、可积分图加速 |

## 5. 典型应用场景

- **参数调优工作台**：本节的交互框架可直接复用于任何需要实时调参的算法原型。
- **预处理流水线对比**：同一输入并行跑多种滤波，肉眼 + 指标（PSNR）双重评估。
- **缺陷检测预处理**：顶帽/黑帽运算突出与背景亮度差异微小的缺陷区域。
- **文本图像增强**：高斯降噪 + 黑帽提取笔画骨架，提升 OCR 识别率。

## 6. 相关官方示例

- [Smoothing.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Smoothing/Smoothing.cpp)：官方滤波演示（含每 0.5 秒自动切换核大小的效果展示）
- [Morphology_1.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Morphology_1.cpp)：腐蚀膨胀基础
- [Morphology_2.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Morphology_2.cpp)：高级形态学交互演示
- [Morphology_3.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/morph_lines_detection/Morphology_3.cpp)：形态学提取水平/垂直线
