# 霍夫变换（Hough Transform）

本节讲解 **霍夫直线检测**（标准 `HoughLines` 与概率 `HoughLinesP`）与 **霍夫圆检测**（`HoughCircles`）。霍夫变换是经典的**参数空间投票**算法，把图像空间的几何形状检测转化为参数空间峰值查找。对应官方示例 [HoughLines\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/HoughLines_Demo.cpp)、[HoughCircle\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/HoughCircle_Demo.cpp)、[houghlines.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/houghlines.cpp)。

本目录源码：[hough\_lines.cpp](hough_lines.cpp)（直线，标准 + 概率双窗口）、[hough\_circles.cpp](hough_circles.cpp)（圆，滑动条调参）。

## 1. 章节文件索引

| 文件                                      | 主题            |
| --------------------------------------- | ------------- |
| [hough\_lines.cpp](hough_lines.cpp)     | 标准/概率霍夫直线检测对比 |
| [hough\_circles.cpp](hough_circles.cpp) | 霍夫梯度法圆检测      |

## 2. 直线检测原理

### 2.1 图像空间 → 参数空间

图像空间中一条直线可用极坐标表示：

$$
\rho = x\cos\theta + y\sin\theta
$$

- $\rho$：原点到直线的垂直距离；

- $\theta$：该垂线与 $x$ 轴的夹角。

**核心思想**：图像空间中经过某点的所有直线对应参数空间 $(\theta, \rho)$ 中的一条正弦曲线；多个共线点在参数空间的多条曲线**交于同一点**。于是「找直线」变成「找参数空间累加器的峰值」。

### 2.2 标准霍夫 HoughLines

```cpp
void HoughLines(InputArray image,   // 二值边缘图（常为 Canny 输出）
                OutputArray lines,  // vector<Vec2f>：每行 (rho, theta)
                double rho,         // 距离分辨率，单位像素，取 1
                double theta,       // 角度分辨率，单位弧度，取 CV_PI/180
                int threshold,      // 累加器阈值，低于该值的峰值忽略
                double srn = 0, double stn = 0);
```

输出是**参数对**而非端点，需要自行换算回直线上的两点再绘制（见源码 `onHoughLines`）：

```cpp
double a = cos(theta), b = sin(theta);
double x0 = a * rho, y0 = b * rho;
Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
```

### 2.3 概率霍夫 HoughLinesP

```cpp
void HoughLinesP(InputArray image,
                 OutputArray lines,  // vector<Vec4i>：每行 (x1, y1, x2, y2)
                 double rho, double theta, int threshold,
                 double minLineLength = 0,   // 最短线段长度，滤除短噪声
                 double maxLineGap = 0);     // 同一直线上点最大间隔
```

**与标准版区别**：概率版只对随机采样的边缘点投票，且直接返回**线段端点**，输出更简洁，检测长直线效果更好。官方示例见 [HoughLines\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/HoughLines_Demo.cpp)。

### 2.4 参数调优指南

| 参数              | 含义     | 调大/调小                      |
| --------------- | ------ | -------------------------- |
| `threshold`     | 累加器阈值  | 越大直线越少越"干净"；越小漏检越多         |
| `minLineLength` | 最小线段长度 | 调大滤除短线噪声                   |
| `maxLineGap`    | 线段最大间隙 | 调大可连接断开的直线                 |
| `rho / theta`   | 分辨率    | 越小越精细但越慢，常用 `1, CV_PI/180` |

## 3. 圆检测原理

圆方程有三个未知数 $(a, b, r)$，直接三维投票开销大。`HoughCircles` 用**霍夫梯度法**简化：

1. 对图像做 Canny 边缘检测与 Sobel 梯度求取，获得每个边缘点的梯度方向；
2. 沿梯度方向累加"可能的圆心"，形成圆心累加器；
3. 由累加器峰值确定圆心，再由圆心到边缘点的距离统计出半径。

```cpp
void HoughCircles(InputArray image,     // 8 位灰度图
                  OutputArray circles,  // vector<Vec3f>：每行 (x, y, radius)
                  int method,           // 目前只有 HOUGH_GRADIENT
                  double dp,            // 累加器分辨率（1=与输入同分辨率）
                  double minDist,       // 圆心间最小距离，过滤近邻重复
                  double param1,        // Canny 高阈值
                  double param2,        // 圆心累加器阈值
                  int minRadius = 0, int maxRadius = 0);
```

**使用要点**（源码 `hough_circles.cpp` 已体现）：

1. 输入必须**先转灰度**；
2. 检测前先 `GaussianBlur` 平滑，噪声会产生大量假圆；
3. `minDist` 设为 `rows/16` 左右，避免同心重复；
4. `minRadius/maxRadius` 限定半径范围可显著减少误检。

## 4. 代码逐段解读

标准霍夫检测来自 [hough\_lines.cpp](hough_lines.cpp)：

```cpp
cvtColor(g_src, g_gray, COLOR_BGR2GRAY);
Canny(g_gray, g_canny, 50, 150, 3);   // 霍夫输入必须是二值边缘图
HoughLines(g_canny, lines, 1, CV_PI / 180, 100, 0, 0);
```

圆检测核心来自 [hough\_circles.cpp](hough_circles.cpp)：

```cpp
HoughCircles(g_gray, circles, HOUGH_GRADIENT, 1,
             g_gray.rows / 16, g_param1, g_param2, 20, 80);
for (size_t i = 0; i < circles.size(); i++) {
    Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
    int radius = cvRound(circles[i][2]);
    circle(dst, center, 3, Scalar(0, 255, 0), -1);     // 圆心
    circle(dst, center, radius, Scalar(255, 0, 0), 2); // 圆周
}
```

## 5. 常见问题与易错点

| 现象                 | 原因                | 解决                                                |
| ------------------ | ----------------- | ------------------------------------------------- |
| 直线过密               | `threshold` 太小    | 调大累加器阈值                                           |
| 圆检测到一堆乱圆           | 未平滑 / `param2` 太小 | 先 `GaussianBlur`；调大 `param2`                      |
| 想检测椭圆              | 标准霍夫只支持圆          | 用 `fitEllipse` 或广义霍夫（`generalizedHoughTransform`） |
| HoughLines 画的线穿过整图 | 返回的是参数对           | 按公式换算端点（见 §2.2）                                   |

## 6. 典型应用场景

- **车道线检测**：高斯滤波 → Canny → Hough 直线，是驾驶辅助最经典流程。

- **文档表格线检测**：提取直线后做版面分析、表格结构还原。

- **硬币/血细胞/圆孔计数**：HoughCircles 检测圆形目标并计数。

- **红绿灯定位**：结合颜色阈值与 Hough 圆检测锁定圆形信号灯。

## 7. 相关官方示例

- [HoughLines\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/HoughLines_Demo.cpp)：标准/概率/加权三窗口对比

- [houghlines.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/houghlines.cpp)：简化版直线检测

- [HoughCircle\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/HoughCircle_Demo.cpp)：圆检测交互调参

- [houghcircles.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/houghcircles.cpp)：圆检测简化版

- [generalizedHoughTransform.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/generalizedHoughTransform.cpp)：任意形状广义霍夫

