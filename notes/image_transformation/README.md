# 图像变换与边缘检测（imageTransformation）

本节覆盖**图像梯度与边缘检测**（Sobel、Scharr、Laplacian、Canny）、**直方图均衡化**、**重映射**与**线性变换**。对应官方示例 [Sobel_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/Sobel_Demo.cpp)、[Laplace_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/Laplace_Demo.cpp)、[CannyDetector_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/CannyDetector_Demo.cpp)、[Remap_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/Remap_Demo.cpp)。

## 1. 章节文件索引

| 文件 | 主题 |
|------|------|
| [sobel.cpp](sobel.cpp) | Sobel 算子求 x/y 方向梯度 |
| [scharr.cpp](scharr.cpp) | Scharr 算子（小核下比 Sobel 更精确） |
| [laplacian.cpp](laplacian.cpp) | Laplacian 二阶导数边缘 |
| [canny.cpp](canny.cpp) | Canny 边缘检测完整流程 |
| [trans.cpp](trans.cpp) | Canny + Sobel + Scharr 三合一交互演示 |
| [equalizehist.cpp](equalizehist.cpp) | 直方图均衡化增强对比度 |
| [remap.cpp](remap.cpp) | 重映射实现图像翻转 |
| [alpha.cpp](alpha.cpp) | 生成 RGBA 渐变透明图 |

## 2. 梯度理论基础

图像梯度是灰度函数的偏导数，边缘处梯度幅值大：

$$
\nabla f = \left( \frac{\partial f}{\partial x}, \frac{\partial f}{\partial y} \right), \quad
|\nabla f| = \sqrt{G_x^2 + G_y^2}, \quad
\theta = \arctan\frac{G_y}{G_x}
$$

实际计算用卷积近似导数。注意输出要用 `CV_16S` 深度（导数可能为负），再 `convertScaleAbs` 转回 8 位显示。

## 3. Sobel 算子

来自 [sobel.cpp](sobel.cpp)：

```cpp
Mat srcImage = imread(imageName, IMREAD_GRAYSCALE);
// x, y 方向的梯度（一阶导数）
Sobel(srcImage, grad_x, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);
Sobel(srcImage, grad_y, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT);
convertScaleAbs(grad_x, abs_grad_x);   // 取绝对值并转 8 位
convertScaleAbs(grad_y, abs_grad_y);
// 合并梯度
addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);
```

`Sobel(src, dst, ddepth, dx, dy, ksize, scale, delta, borderType)` 关键参数：

- `dx=1, dy=0`：求 x 方向梯度；`dx=0, dy=1`：求 y 方向
- `ksize`：核大小（1/3/5/7），`ksize=1` 时退化为 3×1 差分
- `ddepth=CV_16S`：防止负梯度被截断

3×3 Sobel 核（x 方向为平滑×差分的可分离核）：

$$
G_x =
\begin{bmatrix}
-1 & 0 & +1 \\
-2 & 0 & +2 \\
-1 & 0 & +1
\end{bmatrix}, \quad
G_y =
\begin{bmatrix}
-1 & -2 & -1 \\
0 & 0 & 0 \\
+1 & +2 & +1
\end{bmatrix}
$$

## 4. Scharr 算子

来自 [scharr.cpp](scharr.cpp)：

```cpp
Scharr(srcImage, grad_x, CV_16S, 1, 0, 1, 0, BORDER_DEFAULT);
Scharr(srcImage, grad_y, CV_16S, 0, 1, 1, 0, BORDER_DEFAULT);
```

当 `ksize=3` 时，Sobel 的旋转误差较大，Scharr 用优化系数提高精度：

$$
G_x =
\begin{bmatrix}
-3 & 0 & +3 \\
-10 & 0 & +10 \\
-3 & 0 & +3
\end{bmatrix}
$$

**选型**：3×3 场景优先 Scharr；更大核用 Sobel。

## 5. Laplacian 算子

来自 [laplacian.cpp](laplacian.cpp)：

```cpp
GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);  // 先降噪
cvtColor(src, src_gray, COLOR_RGB2GRAY);
Laplacian(src_gray, dst, CV_16S, 3, 1, 0, BORDER_DEFAULT);
convertScaleAbs(dst, abs_dst);
```

Laplacian 是**二阶导数**（散度的梯度），对噪声敏感，所以官方流程总是先高斯平滑：

$$
\Delta f = \frac{\partial^2 f}{\partial x^2} + \frac{\partial^2 f}{\partial y^2}
$$

$$
K =
\begin{bmatrix}
0 & 1 & 0 \\
1 & -4 & 1 \\
0 & 1 & 0
\end{bmatrix}
$$

特点：各向同性（无方向偏好），边缘呈"双边缘"（过零点两侧一明一暗），常用于锐化（原图减去拉普拉斯）。

## 6. Canny 边缘检测

来自 [canny.cpp](canny.cpp)：

```cpp
cvtColor(image, edges, COLOR_BGR2GRAY);
blur(edges, edges, Size(3, 3));   // 3x3 内核降噪
/*
 * canny
 * 第三个参数 第一个滞后性阈值
 * 第四个参数 第二个滞后性阈值
 * 较小的用于边缘连接 较大的控制强边缘的初始段一般在（2:1~3:1）
 * 第五个参数 Sobel算子的孔径大小
 */
Canny(edges, edges, 3, 9, 3);
```

Canny 五步算法：

1. **高斯滤波**去噪；
2. **Sobel 求梯度**幅值与方向；
3. **非极大值抑制（NMS）**：沿梯度方向比较，只保留局部最大值，细化边缘；
4. **双阈值**：梯度 > 高阈值为强边缘；介于两者之间为弱边缘；
5. **滞后连接**：与强边缘相连的弱边缘被保留，孤立的弱边缘被丢弃。

经验法则：高低阈值比取 **2:1 ~ 3:1**（本例 3:9）。

## 7. 三合一交互演示

来自 [trans.cpp](trans.cpp)：同一窗口用两个滑条分别控制 Canny 低阈值与 Sobel 核大小，并固定展示 Scharr 结果：

```cpp
createTrackbar("value：", "canny", &g_cannyLowThreshold, 120, onCanny);
createTrackbar("value：", "canny", &g_sobelKernelSize, 3, onSobel);
onCanny(0, 0);
onSobel(0, 0);
scharr();
```

## 8. 直方图均衡化

来自 [equalizehist.cpp](equalizehist.cpp)：

```cpp
cvtColor(srcImage, srcImage, COLOR_BGR2GRAY);
equalizeHist(srcImage, dstImage);
```

原理：用累积分布函数（CDF）做灰度映射，把输出直方图拉平，增强对比度：

$$
s_k = (L-1) \sum_{j=0}^{k} p_r(j), \quad k = 0, 1, \dots, L-1
$$

其中 $p_r(j)$ 是灰度 $j$ 出现的归一化频率。仅适用于**单通道 8 位**图像；彩色图应转 HSV 后对 V 通道均衡，或用 `createCLAHE` 做局部自适应均衡。

## 9. 重映射 remap

来自 [remap.cpp](remap.cpp)——构造映射表实现垂直翻转：

```cpp
map_x.create(srcImage.size(), CV_32FC1);
map_y.create(srcImage.size(), CV_32FC1);

for (int i = 0; i < srcImage.rows; ++i) {
    for (int j = 0; j < srcImage.cols; ++j) {
        map_x.at<float>(i, j) = static_cast<float>(j);                    // x 不变
        map_y.at<float>(i, j) = static_cast<float>(srcImage.rows - i);    // y 翻转
    }
}

remap(srcImage, dstImage, map_x, map_y, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
```

原理：对目标图每个像素 $(i, j)$，按映射表到源图取像素：

$$
dst(i, j) = src\big( map\_y(i, j),\ map\_x(i, j) \big)
$$

`map_x/map_y` 是浮点坐标，`INTER_LINEAR` 做亚像素插值。翻转、波浪、鱼眼等几何特效都可用两张映射表实现。官方 [Remap_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/Remap_Demo.cpp) 演示了四种映射（上下翻转、左右翻转、组合翻转、缩放）。

## 10. RGBA 透明渐变图

来自 [alpha.cpp](alpha.cpp)：

```cpp
Mat mat(480, 640, CV_8UC4);          // 4 通道：BGRA
createAlphaMat(mat);                  // 逐像素写入渐变透明度
vector<int> compression_params;
compression_params.push_back(IMWRITE_PNG_COMPRESSION);
compression_params.push_back(9);      // PNG 压缩级别 0-9
imwrite("alpha.png", mat, compression_params);
```

只有 PNG 格式支持 alpha 通道保存；`IMWRITE_PNG_COMPRESSION` 是无损压缩级别，不影响画质只影响文件大小。

## 11. 典型应用场景

- **车道线检测**：高斯滤波 → Canny → Hough 变换。
- **文档锐化**：原图减去 Laplacian（USM 锐化）提升文字清晰度。
- **低照度图像增强**：CLAHE/equalizeHist 提升监控夜视画面。
- **图像特效**：remap 实现鱼眼、波浪、镜像等几何变形。

## 12. 相关官方示例

- [Sobel_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/Sobel_Demo.cpp) / [Scharr_Demo](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/Sobel_Demo.cpp)：梯度计算官方演示
- [CannyDetector_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/CannyDetector_Demo.cpp)：Canny 交互调参
- [Geometric_Transforms_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/Geometric_Transforms_Demo.cpp)：仿射/透视几何变换
