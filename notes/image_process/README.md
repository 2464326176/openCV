# 图像处理基础（imageProcess）

本章是 OpenCV 学习的起点，覆盖了 **Mat 容器**、**图像读写、绘图、像素操作** 的全部基础内容，并延伸到滤波、形态学、金字塔等核心图像处理算法。所有示例均参考官方教程代码 [tutorial\_code/core](../../mingw-build/samples/cpp/tutorial_code/core) 与 [tutorial\_code/ImgProc](../../mingw-build/samples/cpp/tutorial_code/ImgProc)。

## 1. 章节结构

| 子目录                                       | 主题                 | 核心 API                                                         |
| ----------------------------------------- | ------------------ | -------------------------------------------------------------- |
| [mat](mat/README.md)                      | Mat 容器与像素访问        | `Mat`、`at<T>`、`clone`、`copyTo`                                 |
| [image](image/README.md)                  | 图像读写、ROI、通道操作、融合   | `imread`、`split`、`merge`、`addWeighted`、`LUT`                   |
| [basic\_drawing](basic_drawing/README.md) | 基本图形绘制             | `line`、`circle`、`ellipse`、`rectangle`、`fillPoly`               |
| [morphology](morphology/README.md)        | 滤波、形态学、漫水填充、金字塔、阈值 | `GaussianBlur`、`dilate`、`erode`、`floodFill`、`threshold`        |
| [process](process/README.md)              | 滤波与形态学处理流程         | `filter2D`、`morphologyEx`、`pyrUp/pyrDown`                      |
| [image\_algo](image_algo/README.md)       | HDR 成像算法           | `createMergeDebevec`、`createTonemapDrago`、`createMergeMertens` |

## 2. 核心知识脉络

```
Mat 容器（内存模型）
   ├── 图像读写：imread / imwrite / imshow
   ├── 像素访问：at<T>() / ptr<T>() / 迭代器 / LUT
   ├── 通道操作：split / merge / ROI / copyTo(mask)
   ├── 图形绘制：line / circle / ellipse / fillPoly
   └── 图像变换：滤波 / 形态学 / 金字塔 / 阈值 / HDR
```

## 3. 学习建议

1. 先读 [mat/README.md](mat/README.md)，理解 Mat 作为「矩阵头 + 数据指针 + 引用计数」的设计，这是后续所有章节的基础。
2. 再读 [image/README.md](image/README.md)，掌握图像加载、颜色空间转换与像素级操作的六种写法及性能对比。
3. [basic\_drawing/README.md](basic_drawing/README.md) 内容轻量，可作为 HighGUI 交互的入门练手。
4. 最后进入 [morphology/README.md](morphology/README.md) 与 [process/README.md](process/README.md)，系统学习滤波与形态学。

## 4. 编译说明

本目录提供 [CMakeLists.txt](CMakeLists.txt)（当前子目录源码收集被注释，按需启用）。官方构建方式参考 [example\_cmake](../../mingw-build/samples/cpp/example_cmake/CMakeLists.txt)。

```cmake
cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( DisplayImage DisplayImage.cpp )
target_link_libraries( DisplayImage ${OpenCV_LIBS} )
```

## 5. 典型应用场景

- **工业视觉检测**：先用 `imread` 读入工件图像，通过 ROI + `threshold` 定位缺陷区域，再用绘图 API 标注结果。

- **图像预处理流水线**：相机采集 → 颜色空间转换 → 滤波降噪 → 形态学修正 → 特征提取，本章内容覆盖流水线前半段。

- **文档/车牌识别预处理**：灰度化 + 双边滤波保边去噪 + 自适应阈值二值化，为 OCR 提供干净输入。

- **HDR 与多曝光融合**：基于 `image_algo` 子目录的 Debevec / Mertens 融合算法，将多张不同曝光的图像合成高动态范围图像。

