# 图像读写与像素操作（image）

本节覆盖图像**加载、显示、保存**、**颜色空间转换**、**通道分离与合并**、**ROI 与掩膜拷贝**、**加权融合**以及**颜色量化（color reduction）**的六种实现与性能对比。对应官方教程 [Load, Modify, and Save an Image](../../../mingw-build/samples/cpp/tutorial_code/introduction/display_image/display_image.cpp) 与 [how_to_scan_images](../../../mingw-build/samples/cpp/tutorial_code/core/how_to_scan_images/how_to_scan_images.cpp)。

## 1. 核心功能

| 源文件                       | 演示内容                              |
|------------------------------|---------------------------------------|
| [read_image.cpp](read_image.cpp)               | `imread` 加载 + 空图检查 + `imshow` 显示     |
| [display_image.cpp](display_image.cpp)         | `cvtColor` 转灰度 + `imwrite` 保存           |
| [split_merge.cpp](split_merge.cpp)             | `split` 分离 BGR 三通道                       |
| [add_image.cpp](add_image.cpp)                 | `addWeighted` 两图线性融合                    |
| [image.cpp](image.cpp)                         | ROI + `copyTo(mask)` 掩膜贴图                 |
| [region_of_interest.cpp](region_of_interest.cpp) | `Rect`/`Range` 两种 ROI 构造方式            |
| [color_reduce.cpp](color_reduce.cpp)           | 颜色量化、6 种像素遍历方式 + 计时对比         |

## 2. 图像读写与显示

### 2.1 加载与健壮性检查

来自 [read_image.cpp](read_image.cpp)：
```cpp
Mat image;
image = imread(imageName, IMREAD_COLOR);
if(image.empty())          // 路径错误或解码失败时返回空 Mat
{
    return -1;
}
namedWindow("display window", WINDOW_AUTOSIZE);
imshow("display window", image);
waitKey(0);                // 必须调用，否则窗口不刷新
```

`imread` 第二个参数常用取值：
| 标志                 | 含义                                       |
|----------------------|--------------------------------------------|
| `IMREAD_COLOR`       | 解码为 3 通道 BGR（默认）                   |
| `IMREAD_GRAYSCALE`   | 解码为单通道灰度                            |
| `IMREAD_UNCHANGED`   | 保持原样（含 alpha 通道、16 位深度）        |

### 2.2 颜色空间转换与保存

来自 [display_image.cpp](display_image.cpp)：
```cpp
Mat gray_image;
cvtColor(image, gray_image, COLOR_BGR2GRAY);
imwrite("./image/gray_image.jpg", gray_image);
```

BGR→Gray 采用 ITU-R BT.601 加权公式：
$$
Gray = 0.299R + 0.587G + 0.114B
$$

权重符合人眼对三色的敏感度（绿 > 红 > 蓝）。`imwrite` 按扩展名选择编码器（`.jpg` 有损、`.png` 无损）。

## 3. 通道操作

### 3.1 通道分离

来自 [split_merge.cpp](split_merge.cpp)：
```cpp
Mat src = imread("../static/data/fruits.jpg");
std::vector<Mat> dest;
split(src, dest);          // dest[0]=B, dest[1]=G, dest[2]=R

imshow("blue",  dest[0]);
imshow("green", dest[1]);
imshow("red",   dest[2]);
```

注意：OpenCV 默认通道顺序是 **BGR 而非 RGB**。`split` 会分离出 3 个独立 `Mat`；若只需单通道，可用 `extractChannel` 或直接 `mixChannels` 更高效。逆操作为 `merge`。

### 3.2 加权融合（线性混合）

来自 [add_image.cpp](add_image.cpp)：
```cpp
double alpha = 0.5;
double beta  = 1.0 - alpha;
addWeighted(windowsLogo, alpha, linuxLogo, beta, 0.0, dst);
```

数学形式：
$$
dst = \alpha \cdot src_1 + \beta \cdot src_2 + \gamma
$$

要求两图**尺寸与类型完全相同**。`alpha` 从 0 到 1 渐变即可实现淡入淡出转场。

## 4. ROI 与掩膜拷贝

### 4.1 两种 ROI 构造方式

来自 [region_of_interest.cpp](region_of_interest.cpp)：
```cpp
// 方式一：Rect(x, y, width, height)
Mat imageROI = img(Rect(p2i.x, p2i.y, mask.rows, mask.cols));

// 方式二：Range(行范围, 列范围)
Mat imageROI = img(Range(p2i.x, p2i.x + mask.rows),
                   Range(p2i.y, p2i.y + mask.cols));
```

ROI 是**浅引用**：不复制像素，只共享原图数据并记录偏移（`step`），对 ROI 的写入直接反映到原图。

### 4.2 带掩膜的贴图

来自 [image.cpp](image.cpp) 与 [region_of_interest.cpp](region_of_interest.cpp)：
```cpp
Mat mask0 = imread("mask.png", IMREAD_GRAYSCALE);
mask.copyTo(imageROI, mask0);   // 仅掩膜非零处复制
```

`copyTo(dst, mask)` 只在 `mask != 0` 的像素处复制，可实现**不规则形状贴图**（如 Logo 的白色背景被跳过）。对比 `addWeighted` 只能做矩形区域融合，掩膜拷贝支持任意形状。

## 5. 颜色量化：像素遍历的六种写法

来自 [color_reduce.cpp](color_reduce.cpp)，把每通道 256 级压缩为 `256/div` 级，映射规则：
$$
I_{new} = \left\lfloor \frac{I_{old}}{div} \right\rfloor \cdot div + \frac{div}{2}
$$

### 5.1 LUT 查表法（推荐）

```cpp
uchar table[256];
fillReduceTable(table, div);          // 预计算 256 个映射值
Mat lut(1, 256, CV_8U, table);
LUT(srcImage, lut, destImage);        // OpenCV 内部 SIMD 加速
```

核心思想：**把逐像素的除法换成一次查表**。`cv::LUT` 有 SIMD/IPP 优化，通常是通用方法中最快的。

### 5.2 指针扫描（连续内存）

```cpp
size_t n = (size_t)rows * cols * channels;
if (srcImage.isContinuous()) {        // 内存无行填充，可一维遍历
    const uchar* src = srcImage.data;
    uchar* dst = destImage.data;
    for (size_t j = 0; j < n; ++j)
        dst[j] = (uchar)(src[j] / div * div + div / 2);
}
```

`isContinuous()` 判断行末是否有对齐填充；非连续时须逐行用 `ptr<uchar>(i)` 遍历。

### 5.3 位掩码法（div 为 2 的幂）

```cpp
const uchar invMask = (uchar)~(div - 1);   // div=8 时为 0xF8
const uchar add = (uchar)(div / 2);
*dst++ = (uchar)((*src++ & invMask) + add); // 位与代替除法
```

用位运算替代除法，编译器更易向量化。

### 5.4 迭代器与 at 访问

```cpp
// 迭代器：类型安全，自动处理边界
auto itS = srcImage.begin<Vec3b>();
auto itE = srcImage.end<Vec3b>();

// at<T>()：带边界检查语义，最灵活但最慢
Vec3b& d = destImage.at<Vec3b>(i, j);
```

### 5.5 性能对比结论

计时框架（`getTickCount` / `getTickFrequency`）实测排序一般为：
$$
LUT \approx 位掩码 < 指针扫描 < 迭代器 < at
$$

`at()` 每次访问需计算 `row * step + col * elemSize` 偏移，适合随机访问；批量遍历优先 `LUT` 或指针。

## 6. 典型应用场景

- **视频转场特效**：`addWeighted` 按帧渐变 alpha 实现淡入淡出。
- **水印/Logo 叠加**：灰度掩膜 + `copyTo` 把不规则 Logo 贴到图像任意位置。
- **图像压缩预处理**：颜色量化减少色阶，降低后续编码码率或用于风格化海报效果。
- **通道级算法**：如仅对 G 通道做增强、HSV 空间对 H 通道分割目标。

## 7. 相关官方示例

- [display_image.cpp](../../../mingw-build/samples/cpp/tutorial_code/introduction/display_image/display_image.cpp)：图像加载显示入门。
- [how_to_scan_images.cpp](../../../mingw-build/samples/cpp/tutorial_code/core/how_to_scan_images/how_to_scan_images.cpp)：像素遍历三种方式官方计时对比。
- [AddingImages.cpp](../../../mingw-build/samples/cpp/tutorial_code/core/AddingImages/AddingImages.cpp)：`addWeighted` 官方演示。
