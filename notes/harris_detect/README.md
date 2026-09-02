# Harris 角点检测（detectHarris）

本节讲解 **Harris 角点检测算法**的数学原理与 OpenCV 实现。对应官方示例 [cornerHarris_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/TrackingMotion/cornerHarris_Demo.cpp)、[goodFeaturesToTrack_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/TrackingMotion/goodFeaturesToTrack_Demo.cpp)、[cornerSubPix_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/TrackingMotion/cornerSubPix_Demo.cpp)。

本目录源码：[corner_harris.cpp](corner_harris.cpp)（最简流程）、[harris.cpp](harris.cpp)（交互式阈值 + 归一化可视化）。

## 1. 核心功能

输入：灰度图像。
输出：标记了角点的彩色图像（红色圆圈标注）。

## 2. Harris 算法原理

### 2.1 角点的直觉

把一个小窗口在图像上滑动，观察窗口内灰度变化：
- **平坦区域**：任意方向移动，灰度几乎不变。
- **边缘**：沿边缘方向移动变化小，垂直边缘方向变化大。
- **角点**：任意方向移动，灰度都剧烈变化。

### 2.2 自相关矩阵（结构张量）

窗口平移 $(u, v)$ 引起的灰度变化用泰勒展开近似：
$$
E(u, v) = \sum_{x, y} w(x, y) \left[ I(x+u, y+v) - I(x, y) \right]^2
\approx
\begin{bmatrix} u & v \end{bmatrix} M \begin{bmatrix} u \\ v \end{bmatrix}
$$

其中窗口函数 $w$ 通常取高斯，结构张量 $M$ 为：
$$
M = \sum_{x, y} w(x, y)
\begin{bmatrix}
I_x^2 & I_x I_y \\
I_x I_y & I_y^2
\end{bmatrix}
$$

$I_x, I_y$ 是图像梯度（Sobel 求得）。

### 2.3 角点响应函数

设 $M$ 的特征值为 $\lambda_1, \lambda_2$：
- 两个特征值都小 → 平坦区；
- 一大一小 → 边缘；
- 两个都大 → 角点。

Harris 用响应值 $R$ 避免显式求特征值：

$$
R = \det(M) - k \cdot \mathrm{trace}(M)^2 = \lambda_1 \lambda_2 - k(\lambda_1 + \lambda_2)^2
$$

其中 $k$ 为经验常数（0.04~0.06）。$R$ 大于阈值且是局部极大值的点即为角点。

## 3. cornerHarris API

```cpp
void cornerHarris(InputArray src, OutputArray dst,
                  int blockSize,    // 计算协方差矩阵的邻域大小
                  int ksize,        // Sobel 求导的核大小
                  double k,         // Harris 自由参数（0.04~0.06）
                  int borderType = BORDER_DEFAULT);
```

输出 `dst` 是 `CV_32FC1` 的响应图，值为 $R$，需要归一化后才能显示。

## 4. 代码逐段解读

### 4.1 最简流程

来自 [corner_harris.cpp](corner_harris.cpp)：
```cpp
Mat srcImage = imread("../static/gril/0.jpg", 0);   // 灰度加载

Mat cornerStrength, harrisConner;
cornerHarris(srcImage, cornerStrength, 2, 3, 0.01);
// blockSize=2, ksize=3, k=0.01

threshold(cornerStrength, harrisConner, 0.00001, 255, THRESH_BINARY);
imshow("cornerHarris", harrisConner);
```

直接对响应图阈值化得到角点二值图。阈值极小（0.00001）是因为未归一化的 $R$ 值量级很小。

### 4.2 交互式完整流程

来自 [harris.cpp](harris.cpp)——官方推荐写法，含归一化与可视化：

```cpp
void onCornerHarris(int, void *) {
    Mat dstImage, normImage, scaledImage;
    dstImage = Mat::zeros(g_srcImage.size(), CV_32FC1);
    g_dstImage = g_srcImage.clone();

    // 1. Harris 检测：blockSize=2, ksize=3, k=0.04
    cornerHarris(g_grayImage, dstImage, 2, 3, 0.04, BORDER_DEFAULT);

    // 2. 归一化到 [0, 255]，便于显示与阈值化
    normalize(dstImage, normImage, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    convertScaleAbs(normImage, scaledImage);   // 转 8 位显示
    // 3. 遍历找强角点并画圈标注
    for (int j = 0; j < normImage.rows; ++j) {
        for (int i = 0; i < normImage.cols; ++i) {
            if ((int)normImage.at<float>(j, i) > thresh + 80) {
                circle(g_dstImage, Point(i, j), 5, Scalar(10, 10, 255), 2, 8, 0);
                circle(scaledImage, Point(i, j), 5, Scalar(10, 10, 255), 2, 8, 0);
            }
        }
    }
    imshow("src", g_dstImage);
    imshow("dst", scaledImage);
}
```

流程总结：*cornerHarris → normalize → 阈值筛选 → circle 标注*。滑条 `thresh` 实时调节角点数量。

## 5. 参数调优指南

| 参数 | 影响 |
|------|------|
| `blockSize` | 越大越稳定，但角点定位越粗 |
| `ksize` | 梯度核大小，3 最常用 |
| `k` | 越小检出的角点越多（更敏感） |
| 阈值 | 控制角点数量，通常取归一化后 100~200 |

## 6. 典型应用场景

- **图像配准/拼接**：角点作为稳定特征点做帧间对齐。
- **运动跟踪**：光流法（Lucas-Kanade）以角点为跟踪点，见 [lkdemo.cpp](../../mingw-build/samples/cpp/lkdemo.cpp)。
- **相机标定**：棋盘格角点提取（`findChessboardCorners` 内部类似思想）。
- **三维重建**：SfM 流程的特征点初始化。

## 7. 相关官方示例

- [cornerHarris_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/TrackingMotion/cornerHarris_Demo.cpp)：Harris 交互演示（本目录 harris.cpp 改编自它）
- [goodFeaturesToTrack_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/TrackingMotion/goodFeaturesToTrack_Demo.cpp)：Shi-Tomasi 角点（最小特征值准则）
- [cornerSubPix_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/TrackingMotion/cornerSubPix_Demo.cpp)：角点亚像素精化
- [cornerDetector_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/TrackingMotion/cornerDetector_Demo.cpp)：Harris 与 Shi-Tomasi 对比
