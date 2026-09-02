# Mat 容器与像素访问（mat）

`cv::Mat` 是 OpenCV 的核心数据结构，既是图像容器，也是通用矩阵。本节对应官方教程 [Mat - The Basic Image Container](../../../mingw-build/samples/cpp/tutorial_code/core/mat_the_basic_image_container/mat_the_basic_image_container.cpp) 与 [Operations with images](../../../mingw-build/samples/cpp/tutorial_code/core/mat_operations/mat_operations.cpp)。本目录源码：[mat.cpp](mat.cpp)、[mat1.cpp](mat1.cpp)、[mat_operate.cpp](mat_operate.cpp)。

## 1. Mat 的内存模型

Mat 由两部分组成：
```
┌─────────────────────────┐       ┌──────────────────────┐
│  矩阵头（固定大小）      │       │ 像素数据（动态大小）  │
│   - 尺寸、通道数、类型    │──指针──▶│ uchar/float/...      │
│   - data 指针            │       │ 引用计数 refcount     │
│   - step[] 步长          │       └──────────────────────┘
└─────────────────────────┘
```

- **矩阵头**：拷贝代价恒定（几十字节），因此函数按值传递 `Mat` 很廉价。
- **数据块**：多个 Mat 头可共享同一块数据，由**引用计数**管理生命周期。
- **step[]**：描述内存布局。`step[0]` 是行字节数（可能含对齐填充），`step[1]` 是元素字节数。像素 $(i,j)$ 的地址为：

$$
addr = data + i \cdot step[0] + j \cdot step[1]
$$

## 2. 创建 Mat 的多种方式

来自 [mat.cpp](mat.cpp)：
```cpp
// 1. 指定尺寸+类型+初始值：2x2 三通道，红色 (B=0,G=0,R=255)
Mat M(2, 2, CV_8UC3, Scalar(0, 0, 255));

// 2. 多维矩阵：2x2x2 单通道全零
int matSize[3] = {2, 2, 2};
Mat L(3, matSize, CV_8UC1, Scalar::all(0));

// 3. 从图像深拷贝
Mat mtx = img.clone();        // 深拷贝；Mat mtx(img) 是浅拷贝

// 4. create：按需重新分配（尺寸类型匹配则复用旧内存）
M.create(4, 4, CV_8UC2);

// 5. 特殊矩阵
Mat E = Mat::eye(4, 4, CV_64F);    // 单位阵
Mat O = Mat::ones(4, 4, CV_64F);   // 全 1
Mat Z = Mat::zeros(4, 4, CV_64F);  // 全 0

// 6. 逗号初始化器（小矩阵/卷积核常用）
Mat C = (Mat_<double>(3, 3) << 0, -1, 0,
                               -1,  5, -1,
                                0, -1, 0);

// 7. 克隆某一行
Mat RowClone = C.row(1).clone();   // 注意：必须 clone，row() 是引用
```

### 2.1 类型命名规则

`CV_<深度><U/S/C><通道数>`，例如 `CV_8UC3` = 8 位无符号、3 通道。`Mat_<T>` 模板类与深度的对应（见 [mat1.cpp](mat1.cpp) 注释）：

| 模板类      | 深度常量   |
|-------------|------------|
| `Mat_<uchar>`   | `CV_8U`  |
| `Mat_<char>`    | `CV_8S`  |
| `Mat_<int>`     | `CV_32S` |
| `Mat_<float>`   | `CV_32F` |
| `Mat_<double>`  | `CV_64F` |

## 3. 像素访问方式

来自 [mat_operate.cpp](mat_operate.cpp)（官方 mat_operations 教程片段）：
```cpp
// 单通道灰度图
Scalar intensity = img.at<uchar>(y, x);
Scalar intensity = img.at<uchar>(Point(x, y));

// 3 通道 BGR 图
Vec3b intensity = img.at<Vec3b>(y, x);
uchar blue  = intensity.val[0];
uchar green = intensity.val[1];
uchar red   = intensity.val[2];

// 浮点图（如 Sobel 结果）
Vec3f intensity = img.at<Vec3f>(y, x);

// 写入像素
img.at<uchar>(y, x) = 128;
```

四种访问方式对比：
| 方式          | 语法                       | 特点                                  |
|---------------|----------------------------|---------------------------------------|
| `at<T>()`     | `img.at<Vec3b>(y,x)`       | 可读性最好，有类型检查，逐像素最慢     |
| `ptr<T>()`    | `img.ptr<uchar>(i)`        | 返回行首指针，批量遍历快               |
| 迭代器        | `img.begin<Vec3b>()`       | 类型安全，自动跨行                     |
| `data` 指针   | `img.data`                 | 最快，需自行处理 step 与连续性         |

## 4. 深拷贝与浅拷贝

```cpp
Mat img1 = img.clone();   // 深拷贝：独立数据
Mat img2 = img;           // 浅拷贝：共享数据，引用计数 +1
Mat roi  = img(Rect(10, 10, 100, 100));  // ROI 也是浅拷贝
```

浅拷贝的矩阵在析构时递减引用计数，计数归零才释放数据。`reshape`、`row`、`col`、`operator()` 等 ROI 均不复制数据。

## 5. 常用操作速查

来自 [mat_operate.cpp](mat_operate.cpp)：
```cpp
img = Scalar(0);                        // 整图置零
Rect r(10, 10, 100, 100);
Mat smallImg = img(r);                  // 取 ROI
cvtColor(img, grey, COLOR_BGR2GRAY);    // BGR→灰度
src.convertTo(dst, CV_32F);             // 深度转换
```

### 5.1 浮点结果可视化

梯度图（`CV_32F`）值域超出 0~255，显示前需归一化：

```cpp
Sobel(grey, sobelx, CV_32F, 1, 0);
double minVal, maxVal;
minMaxLoc(sobelx, &minVal, &maxVal);
// 线性映射 [minVal, maxVal] → [0, 255]
sobelx.convertTo(draw, CV_8U,
                 255.0/(maxVal - minVal),
                 -minVal * 255.0/(maxVal - minVal));
```

`convertTo(dst, rtype, alpha, beta)` 执行 $dst = src \cdot \alpha + \beta$，一行完成归一化。

## 6. 典型应用场景

- **算法原型验证**：用逗号初始化器手写 3×3 卷积核，配合 `filter2D` 快速试验。
- **内存敏感场景**：理解引用计数后，可用 ROI/`reshape` 零拷贝地重组数据，避免大图复制。
- **性能优化**：批量像素处理从 `at` 切换到 `ptr`/`LUT`，通常可获得数倍提速。

## 7. 相关官方示例

- [mat_the_basic_image_container.cpp](../../../mingw-build/samples/cpp/tutorial_code/core/mat_the_basic_image_container/mat_the_basic_image_container.cpp)：Mat 创建方式全集。
- [mat_operations.cpp](../../../mingw-build/samples/cpp/tutorial_code/core/mat_operations/mat_operations.cpp)：图像操作官方代码片段（本目录 mat_operate.cpp 即来源于此）。
- [how_to_scan_images.cpp](../../../mingw-build/samples/cpp/tutorial_code/core/how_to_scan_images/how_to_scan_images.cpp)：四种像素遍历性能对比。
- [cout_mat.cpp](../../../mingw-build/samples/cpp/cout_mat.cpp)：Mat 打印输出示例。
