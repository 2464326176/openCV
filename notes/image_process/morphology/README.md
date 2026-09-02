# 滤波、形态学与阈值化（morphology）

本节系统讲解**三种线性滤波、两种非线性滤波、七种形态学运算、漫水填充、图像缩放、金字塔与阈值化**。对应官方示例 [Smoothing.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Smoothing/Smoothing.cpp)、[Morphology_1.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Morphology_1.cpp)、[Morphology_2.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Morphology_2.cpp)、[Threshold.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Threshold.cpp)、[Pyramids.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Pyramids/Pyramids.cpp)。

知识点清单：
- 三种线性滤波：方框滤波、均值滤波、高斯滤波
- 两种非线性滤波：中值滤波、双边滤波
- 七种形态学运算：腐蚀、膨胀、开运算、闭运算、形态学梯度、顶帽、黑帽
- 漫水填充、图像缩放、图像金字塔、阈值化

## 1. 线性滤波（卷积）

所有线性滤波本质是**卷积（相关）**运算：
$$
dst(x, y) = \sum_{i, j} src(x+i, y+j) \cdot K(i, j)
$$

其中 $K$ 为卷积核。核越大越模糊，但细节损失越多。

### 1.1 方框滤波 boxFilter

核内系数全为 $\frac{1}{rows \cdot cols}$（`normalize=true` 时），等价于均值滤波：

```cpp
boxFilter(srcImage, boxFilterImage, -1, Size(boxFilterValue, boxFilterValue));
// ddepth=-1 表示与输入同深度
```

### 1.2 均值滤波 blur

来自 [filter_process.cpp](../process/filter_process.cpp)：
```cpp
blur(srcImage, meanFilterImage, Size(meanFilterValue, meanFilterValue), Point(-1, -1));
// Point(-1,-1) 表示锚点位于核中心
```

$3 \times 3$ 均值核：
$$
K = \frac{1}{9}
\begin{bmatrix}
1 & 1 & 1 \\
1 & 1 & 1 \\
1 & 1 & 1
\end{bmatrix}
$$

特点：简单快速，但**模糊边缘**，且对椒盐噪声效果差（异常值参与平均）。

### 1.3 高斯滤波 GaussianBlur

核系数按二维高斯分布加权，中心权重最大：
$$
G(x, y) = A \cdot e^{-\left(\frac{(x-x_0)^2}{2\sigma_x^2} + \frac{(y-y_0)^2}{2\sigma_y^2}\right)}
$$

来自 [filter_process.cpp](../process/filter_process.cpp)：
```cpp
GaussianBlur(srcImage, gaussianFilterImage,
             Size(gaussianFilterValue * 2 + 1, gaussianFilterValue * 2 + 1),  // 核必须为奇数
             0, 0);   // sigmaX=0 时由核尺寸自动推算
```

特点：保边性优于均值滤波，是 Canny 等算法的标准预处理。

## 2. 非线性滤波

### 2.1 中值滤波 medianBlur

取邻域像素**中位数**替代中心像素：
$$
dst(x, y) = \underset{(i,j) \in window}{median}\ src(x+i, y+j)
$$

```cpp
medianBlur(srcImage, medianFilterImage, medianFilterValue * 2 + 1);  // ksize 为奇数
```

特点：**椒盐噪声的克星**——极端值不会拉偏结果；对细节、尖角有一定损伤。

### 2.2 双边滤波 bilateralFilter

同时考虑**空间距离**与**灰度差**的加权平均：
$$
dst(x, y) = \frac{1}{W_p} \sum_{i,j} src(i,j) \cdot
\underbrace{e^{-\frac{\|p-q\|^2}{2\sigma_s^2}}}_{空间项}\cdot
\underbrace{e^{-\frac{|I_p - I_q|^2}{2\sigma_c^2}}}_{值域}
$$

灰度差大的像素权重趋于 0，因此**去噪同时能保住边缘**。
```cpp
bilateralFilter(srcImage, bilateralFilterImage,
                bilateralFilterValue,            // d：邻域直径
                bilateralFilterValue * 2.0,      // sigmaColor：值域标准差
                bilateralFilterValue / 2.0);     // sigmaSpace：空间标准差
```

## 3. 形态学运算

形态学基于**结构元（kernel）**在二值/灰度图上滑动。先用 `getStructuringElement` 构造结构元：
```cpp
Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
// 形状可选：MORPH_RECT / MORPH_CROSS / MORPH_ELLIPSE
```

### 3.1 腐蚀与膨胀（基础运算）

**腐蚀**（取邻域最小值）——收缩亮区、去除小亮斑：
$$
(A \ominus B)(x,y) = \min_{(i,j) \in B} A(x+i, y+j)
$$

**膨胀**（取邻域最大值）——扩张亮区、填补小孔洞：
$$
(A \oplus B)(x,y) = \max_{(i,j) \in B} A(x+i, y+j)
$$

来自 [image_operation1.cpp](image_operation1.cpp) 的腐蚀实例：
```cpp
Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
erode(srcImage, dstImage, element);
```

### 3.2 七种形态学运算一览

来自 [morph_op_process.cpp](../process/morph_op_process.cpp)，用 `morphologyEx` 统一接口演示全部七种：
```cpp
MorphDemo demos[] = {
    {"dilate image",     MORPH_DILATE,    15},
    {"erode image",      MORPH_ERODE,     15},
    {"open image",       MORPH_OPEN,      15},
    {"close image",      MORPH_CLOSE,     15},
    {"gradient image",   MORPH_GRADIENT,  15},
    {"tophat image",     MORPH_TOPHAT,    15},
    {"blackhat image",   MORPH_BLACKHAT,  15},
};

Mat kernel = getStructuringElement(MORPH_RECT, Size(k, k));
morphologyEx(srcImage, dst, demo.op, kernel);
```

| 运算                 | 定义             | 效果                          |
|----------------------|------------------|-------------------------------|
| 开运算 `MORPH_OPEN`    | 先腐蚀后膨胀     | 去除细小亮斑、断开细桥        |
| 闭运算 `MORPH_CLOSE`  | 先膨胀后腐蚀     | 填补小孔洞、连接断裂          |
| 形态学梯度 `MORPH_GRADIENT` | 膨胀减去腐蚀 | 提取物体轮廓                  |
| 顶帽 `MORPH_TOPHAT`   | 原图减去开运算   | 提取比背景亮的细节            |
| 黑帽 `MORPH_BLACKHAT` | 闭运算减去原图   | 提取比背景暗的细节            |

## 4. 漫水填充 floodFill

来自 [floodfill.cpp](floodfill.cpp)：
```cpp
Rect ccomp;
floodFill(image, Point(0, 0), Scalar(155, 255, 55), &ccomp,
          Scalar(20, 20, 20), Scalar(20, 20, 20));
// 种子点 (0,0)，填充色，返回连通域外接矩形
// loDiff/upDiff 控制颜色容差
```

原理：从种子点出发，把与种子点颜色差在 `[seed-loDiff, seed+upDiff]` 范围内的连通像素全部染成新颜色。`FLOODFILL_FIXED_RANGE` 标志表示以种子点为基准而非逐像素比较。常用于背景替换、区域分割（如 `grabcut` 内部就用到）。

## 5. 图像缩放 resize

来自 [resize.cpp](resize.cpp)：
```cpp
resize(src, dst,  Size(src.cols / 2, src.rows / 2), 0, 0, INTER_LINEAR);  // 缩小
resize(src, dst2, Size(src.cols * 2, src.rows * 2), 0, 0, INTER_LINEAR);  // 放大
```

常用插值方法：
| 标志             | 原理                  | 特点                            |
|------------------|-----------------------|---------------------------------|
| `INTER_NEAREST`  | 取最近邻像素          | 最快，有锯齿                    |
| `INTER_LINEAR`   | 双线性：2×2 邻域加权  | 默认，速度与质量均衡            |
| `INTER_CUBIC`    | 双三次（4×4 邻域）    | 放大更平滑，较慢                |
| `INTER_AREA`     | 像素区域重采样        | **缩小**时抗混叠最佳            |

双线性插值公式（对目标点映射回源图的浮点坐标做两次线性插值）：
$$
f(x, y) \approx (1-\alpha)(1-\beta) f_{00} + \alpha(1-\beta) f_{10} + (1-\alpha)\beta f_{01} + \alpha\beta f_{11}
$$

## 6. 图像金字塔

来自 [resize.cpp](resize.cpp)：
```cpp
pyrDown(src, dst, Size(src.cols / 2, src.rows / 2));  // 下采样：先高斯模糊再隔行采样
pyrUp(src, dst, Size(src.cols * 2, src.rows * 2));    // 上采样：插值放大（比原图模糊）
```

- `pyrDown`：先高斯卷积后去掉偶数行/列，尺寸减半。
- `pyrUp`：行列各补零插值放大 2 倍，再乘 4 归一化。
- 注意：`pyrUp(pyrDown(img))` **不等于**原图，信息在下采样时已丢失。

应用：多尺度检测（如金字塔加速的滑动窗口）、图像融合（拉普拉斯金字塔）。

## 7. 阈值化

### 7.1 固定阈值 threshold

$$
dst(x,y) =
\begin{cases}
maxval, & src(x,y) > thresh \\
0, & \text{其他}
\end{cases}
\quad (\text{THRESH\_BINARY})
$$

五种类型：`THRESH_BINARY`、`THRESH_BINARY_INV`、`THRESH_TRUNC`、`THRESH_TOZERO`、`THRESH_TOZERO_INV`，可叠加 `THRESH_OTSU`（大津法自动求阈值）。

### 7.2 自适应阈值 adaptiveThreshold

对光照不均的图像，用**每个像素邻域的加权均值**作为局部阈值：
$$
T(x,y) = \text{mean/gaussian}(邻域) - C
$$

```cpp
adaptiveThreshold(src, dst, 255,
                  ADAPTIVE_THRESH_MEAN_C,      // 或 ADAPTIVE_THRESH_GAUSSIAN_C
                  THRESH_BINARY, 11, 2);       // blockSize=11, C=2
```

## 8. 典型应用场景

- **工业缺陷检测**：高斯滤波去噪 → 顶帽运算突出亮缺陷 → 阈值化分割。
- **文档扫描增强**：自适应阈值克服纸张阴影，得到清晰黑白文档。
- **背景建模**：`floodFill` 从四角种子填充去除纯色背景。
- **多尺度目标检测**：金字塔逐层下采样，在不同尺度上检测目标。

## 9. 相关官方示例

- [Smoothing.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Smoothing/Smoothing.cpp)：五种滤波官方演示。
- [Morphology_2.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Morphology_2.cpp)：七种形态学运算交互演示。
- [Threshold.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Threshold.cpp) / [Threshold_inRange.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/Threshold_inRange.cpp)：阈值化与颜色范围过滤。
- [ffilldemo.cpp](../../../mingw-build/samples/cpp/ffilldemo.cpp)：漫水填充交互示例。
