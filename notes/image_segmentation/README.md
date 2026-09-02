# 图像分割与轮廓检测（imageSegmentation）

本节讲解 **Canny + findContours + drawContours** 的经典轮廓提取流程，以及轮廓检索模式与层级结构。对应官方示例 [findContours_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ShapeDescriptors/findContours_demo.cpp) 与 [imageSegmentation.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/imageSegmentation.cpp)（分水岭分割）。

本目录源码：[segmentation.cpp](segmentation.cpp)（交互式轮廓提取）、[find_outline.cpp](find_outline.cpp)（阈值化 + 轮廓填充）。

## 1. 核心功能

输入：一张彩色图像。

输出：
- Canny 边缘图（阈值可调）
- 随机彩色着色的轮廓图

## 2. 轮廓提取标准流程

```
原图 → 灰度化 → 模糊降噪 → Canny/阈值二值化 → findContours → drawContours/分析
```

## 3. 代码逐段解析

### 3.1 交互式轮廓提取

来自 [segmentation.cpp](segmentation.cpp)。

```cpp
g_srcImage = imread(imageName);
cvtColor(g_srcImage, g_grayImgae, COLOR_RGB2GRAY);
blur(g_grayImgae, g_grayImgae, Size(3, 3));        // 降噪，减少碎边缘

createTrackbar("canny 阈值", "blur", &g_nThresh, g_nThresh_max, onThreshChange);
```

回调中完成边缘 → 轮廓 → 绘制三步：

```cpp
void onThreshChange(int, void *)
{
    // 高低阈值取 1:2
    Canny(g_grayImgae, g_cannyMat_output, g_nThresh, g_nThresh * 2, 3);
    findContours(g_cannyMat_output, g_vContours, g_vHierarchy,
                 RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    Mat drawing = Mat::zeros(g_cannyMat_output.size(), CV_8UC3);
    for (size_t i = 0; i < g_vContours.size(); ++i) {
        Scalar color = Scalar(g_rng.uniform(0, 255),
                              g_rng.uniform(0, 255),
                              g_rng.uniform(0, 255));   // 随机颜色
        drawContours(drawing, g_vContours, i, color, 2, 8, g_vHierarchy, 0, Point());
    }
    imshow("segmentation", drawing);
}
```

### 3.2 findContours 参数详解

```cpp
void findContours(InputOutputArray image,
                  OutputArrayOfArrays contours,   // vector<vector<Point>>
                  OutputArray hierarchy,          // vector<Vec4i>
                  int mode,                       // 检索模式
                  int method,                     // 逼近方法
                  Point offset = Point());
```

**检索模式 mode**：

| 模式 | 含义 |
|------|------|
| `RETR_EXTERNAL` | 只取最外层轮廓 |
| `RETR_LIST` | 所有轮廓，无层级关系 |
| `RETR_CCOMP` | 两层结构：外轮廓 + 内孔洞 |
| `RETR_TREE` | 完整嵌套层级 |

**逼近方法 method**：

| 方法 | 含义 |
|------|------|
| `CHAIN_APPROX_NONE` | 保存所有轮廓点 |
| `CHAIN_APPROX_SIMPLE` | 压缩水平/垂直/对角线段，只留端点 |
| `CHAIN_APPROX_TC89_L1/KCOS` | Teh-Chin 逼近算法 |

### 3.3 hierarchy 层级结构

每个轮廓 `i` 对应 `hierarchy[i] = [next, previous, child, parent]`：

- `next/previous`：同级下一个/上一个轮廓的索引
- `child`：第一个子轮廓（内孔洞）
- `parent`：父轮廓索引
- 不存在时取 `-1`

### 3.4 阈值化 + 轮廓填充

来自 [find_outline.cpp](find_outline.cpp)——用比较运算符二值化，再按层级遍历填充。

```cpp
Mat srcImage = imread(imageName, 0);              // 灰度加载
Mat dstImage = Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC3);
srcImage = srcImage > 119;                        // Mat 比较运算直接生成二值图
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
findContours(srcImage, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

// 按 hierarchy 的 next 链遍历所有顶层轮廓
for (int index = 0; index >= 0; index = hierarchy[index][0]) {
    Scalar color(rand()&255, rand()&255, rand()&255);
    drawContours(dstImage, contours, index, color, FILLED, 8, hierarchy);
}
```

要点：
1. `srcImage > 119` 是 Mat 表达式，返回 0/255 的二值图，等价于 `threshold(..., THRESH_BINARY)`。
2. `RETR_CCOMP` 下顶层轮廓通过 `hierarchy[index][0]`（next）串成链表。
3. `drawContours` 的 `thickness=FILLED` 表示填充轮廓内部。

## 4. 轮廓分析常用函数（延伸）

| 函数 | 作用 |
|------|------|
| `contourArea(c)` | 轮廓面积 |
| `arcLength(c, true)` | 轮廓周长 |
| `boundingRect(c)` | 正外接矩形 |
| `minAreaRect(c)` | 最小旋转矩形 |
| `minEnclosingCircle(c)` | 最小外接圆 |
| `fitEllipse(c)` | 拟合椭圆（≥ 6 点） |
| `approxPolyDP(c, eps, true)` | 多边形逼近 |
| `moments(c)` | 矩（可求质心） |
| `pointPolygonTest(c, pt, true)` | 点到轮廓距离 |

参考官方 [ShapeDescriptors](../../mingw-build/samples/cpp/tutorial_code/ShapeDescriptors/) 目录全部示例。

## 5. 典型应用场景

- **零件计数**：二值化 → 轮廓 → `contourArea` 过滤 → 计数。
- **形状识别**：`approxPolyDP` 逼近后按顶点数区分三角形/矩形/多边形。
- **目标定位**：`boundingRect`/`minAreaRect` 输出检测框供后续处理。
- **图像分割后处理**：分水岭/GrabCut 分割结果转轮廓，便于矢量化存储。
- **OCR 文本检测**：阈值化 + 轮廓填充形成字符连通域，再喂给识别引擎。

## 6. 相关官方示例

- [findContours_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ShapeDescriptors/findContours_demo.cpp)：Canny+轮廓官方演示（本目录 segmentation.cpp 即改编自它）
- [imageSegmentation.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgTrans/imageSegmentation.cpp)：分水岭算法分割
- [moments_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ShapeDescriptors/moments_demo.cpp)：矩与质心
- [hull_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/ShapeDescriptors/hull_demo.cpp)：凸包检测
- [watershed.cpp](../../mingw-build/samples/cpp/watershed.cpp)：交互式分水岭分割
