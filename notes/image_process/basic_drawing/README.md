# 基本图形绘制（basicDrawing）

本节对应官方教程 [Basic Drawing](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/basic_drawing/Drawing_1.cpp)，演示如何用 OpenCV 的绘图函数在 `Mat` 上绘制椭圆、圆、多边形、矩形和直线。本目录实现见 [draw.cpp](draw.cpp) 与 [draw.h](draw.h)。

## 1. 核心功能

输入：无（程序内部创建空白画布）

输出：

- `atomImage`：由 4 个不同角度的椭圆 + 1 个填充圆组成的"原子"图案
- `rookImage`：由填充多边形 + 矩形 + 多条直线组成的"城堡"图案

## 2. 绘图函数总览

| 函数 | 作用 | 关键参数 |
|------|------|----------|
| `ellipse()` | 绘制椭圆/椭圆弧 | 中心、轴长 `Size`、旋转角、起止角 |
| `circle()` | 绘制圆 | 圆心、半径、`FILLED` 表示填充 |
| `fillPoly()` | 填充多边形 | 点数组指针、每多边形点数 |
| `rectangle()` | 绘制矩形 | 两对角点、`FILLED` 表示填充 |
| `line()` | 绘制直线 | 起点、终点、线宽 |

所有绘图函数共享一组通用参数：

- `color`：`Scalar(B, G, R)`，注意是 **BGR 顺序**
- `thickness`：线宽，`FILLED`（即 -1）表示填充封闭图形
- `lineType`：`LINE_8`（8 连通）、`LINE_4`（4 连通）、`LINE_AA`（抗锯齿，最平滑）

## 3. 代码逐段解析

### 3.1 创建画布

来自 [draw.cpp](draw.cpp)：

```cpp
#define w 400

Mat atomImage = Mat::zeros(w, w, CV_8UC3);
Mat rookImage = Mat::zeros(w, w, CV_8UC3);
```

用 `Mat::zeros` 创建 400×400 的三通道黑色画布。`CV_8UC3` 表示 8 位无符号、3 通道。

### 3.2 绘制椭圆（不同旋转角）

```cpp
drawEllipse(atomImage, 90);
drawEllipse(atomImage, 0);
drawEllipse(atomImage, 45);
drawEllipse(atomImage, -45);

void drawEllipse(Mat img, double angle) {
    ellipse(img,
            Point(w / 2, w / 2),      // 椭圆中心
            Size(w / 4, w / 16),      // 长轴/短轴半径
            angle,                    // 旋转角度
            0,                        // 起始角
            360,                      // 结束角（完整椭圆）
            Scalar(255, 0, 0),        // 蓝色（BGR）
            2,                        // 线宽
            LINE_8);
}
```

同一中心、同一轴长，只改变 `angle` 参数，即可得到旋转对称的"原子轨道"效果。起止角设为 `0~360` 画完整椭圆；若设为 `0~180` 则只画半个椭圆弧。

### 3.3 绘制填充圆

```cpp
void drawFilledCircle(Mat img, Point center) {
    circle(img,
           center,          // 圆心
           w / 32,          // 半径
           Scalar(0, 0, 255), // 红色
           FILLED,          // thickness = -1，填充
           LINE_8);
}
```

`FILLED` 是 `thickness = -1` 的语义化常量，对圆、矩形、多边形等封闭图形生效。

### 3.4 绘制填充多边形

```cpp
Point rook_points[1][20];
rook_points[0][0]  = Point(    w/4,   7*w/8 );
rook_points[0][1]  = Point(  3*w/4,   7*w/8 );
// ... 共 20 个顶点，勾勒城堡轮廓

const Point* ppt[1] = { rook_points[0] };  // 指向每个多边形顶点数组的指针数组
int npt[] = { 20 };                        // 每个多边形的顶点数

fillPoly( img,
          ppt,                  // 顶点指针数组
          npt,                  // 各多边形顶点数
          1,                    // 多边形个数
          Scalar( 255, 255, 255 ),
          lineType );
```

`fillPoly` 的参数设计支持**一次填充多个多边形**：`ppt` 是"指针的数组"，`npt` 记录每个多边形的顶点数，第三个参数是多边形数量。本例只画 1 个 20 顶点的城堡轮廓。

### 3.5 矩形与直线组合

```cpp
rectangle( rookImage,
           Point( 0, 7*w/8 ),
           Point( w, w),
           Scalar( 0, 255, 255 ),
           FILLED,
           LINE_8 );
drawLine( rookImage, Point( 0, 15*w/16 ), Point( w, 15*w/16 ) );
drawLine( rookImage, Point( w/4, 7*w/8 ), Point( w/4, w ) );
```

先用填充矩形画城堡底座，再用多条竖线/横线分割出"城垛"纹理。

### 3.6 显示与窗口管理

```cpp
imshow("atomImage", atomImage);
moveWindow("atomImage", 0, 200);
imshow("rookImage", rookImage);
moveWindow("rookImage", w, 200);
waitKey(0);
```

`moveWindow` 把两个窗口并排摆放，便于对比观察。

## 4. 原理要点

### 4.1 坐标系

OpenCV 图像坐标系：原点在**左上角**，x 向右、y 向下。所有 `Point(x, y)` 参数均遵循此约定，与数学坐标系 y 轴相反。

### 4.2 抗锯齿

`LINE_8` 按 8 连通逐像素绘制，斜线会有锯齿；改用 `LINE_AA` 会对边缘像素做亚像素加权，视觉上更平滑，但速度略慢：

```cpp
line(img, start, end, Scalar(0,0,0), 2, LINE_AA);
```

### 4.3 绘制即修改像素

绘图函数直接写入 `Mat` 的像素数据（原地操作），不返回新图像。若需保留原图，先 `clone()` 再绘制。

## 5. 典型应用场景

- **检测结果可视化**：目标检测后在图上画 `rectangle` 框、`circle` 标关键点、`putText` 写置信度。
- **标定板/测试图生成**：程序化生成棋盘格、同心圆等图案，用于相机标定或算法测试。
- **数据标注工具**：交互式标注中用 `line`/`fillPoly` 绘制多边形标注区域。

## 6. 相关官方示例

- [Drawing_1.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/basic_drawing/Drawing_1.cpp)：基础绘图完整演示
- [Drawing_2.cpp](../../../mingw-build/samples/cpp/tutorial_code/ImgProc/basic_drawing/Drawing_2.cpp)：随机图形 + `putText` 文本绘制
- [drawing.cpp](../../../mingw-build/samples/cpp/drawing.cpp)：顶层综合绘图示例
