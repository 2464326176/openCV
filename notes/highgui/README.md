# HighGUI 交互（highGui）

本节讲解 OpenCV 的 **HighGUI 模块**：窗口管理、图像显示、滑条（Trackbar）与回调机制，并以"对比度/亮度实时调节"为例。对应官方示例 [BasicLinearTransformsTrackbar.cpp](../../mingw-build/samples/cpp/tutorial_code/HighGUI/BasicLinearTransformsTrackbar.cpp) 与 [AddingImagesTrackbar.cpp](../../mingw-build/samples/cpp/tutorial_code/HighGUI/AddingImagesTrackbar.cpp)。

本目录源码：[basic_linear_transforms_trackbar.cpp](basic_linear_transforms_trackbar.cpp)。

## 1. 核心功能

输入：一张图像 + 两个滑条（contrast / brightness）。
输出：实时调节后的图像 $g(x) = \alpha \cdot f(x) + \beta$。

## 2. HighGUI API 速查

| API | 作用 |
|-----|------|
| `namedWindow(name, flags)` | 创建窗口；`WINDOW_AUTOSIZE` 固定尺寸、`WINDOW_NORMAL` 可缩放 |
| `imshow(name, mat)` | 在指定窗口显示图像 |
| `waitKey(ms)` | 等待按键；`0` 表示无限等待，返回值是按键 ASCII |
| `destroyWindow/destroyAllWindows` | 销毁窗口释放资源 |
| `moveWindow(name, x, y)` | 移动窗口位置 |
| `resizeWindow(name, w, h)` | 调整窗口大小 |
| `createTrackbar(...)` | 创建滑条并绑定回调 |
| `getTrackbarPos/setTrackbarPos` | 读取/设置滑条位置 |
| `setMouseCallback` | 绑定鼠标事件回调 |

## 3. 线性变换原理

对比度与亮度调节是最基础的像素级运算。
$$
g(x) = \alpha \cdot f(x) + \beta
$$

- $\alpha > 0$ 为**对比度增益**：$\alpha > 1$ 增强对比度，$0 < \alpha < 1$ 降低。
- $\beta$ 为**亮度偏置**：正值变亮，负值变暗。
- 结果用 `saturate_cast<uchar>` 截断到 [0, 255]，防止溢出回绕。

## 4. 代码逐段解读

来自 [basic_linear_transforms_trackbar.cpp](basic_linear_transforms_trackbar.cpp)。

### 4.1 回调函数：逐像素线性变换
```cpp
static void onTrackbar(int, void *) {
    Mat new_image = Mat::zeros(srcImage.size(), srcImage.type());

    for (int y = 0; y < srcImage.rows; ++y) {
        for (int x = 0; x < srcImage.cols; ++x) {
            for (int c = 0; c < 3; ++c) {
                new_image.at<Vec3b>(y,x)[c] =
                    saturate_cast<uchar>( alpha * (srcImage.at<Vec3b>(y,x)[c]) + beta );
            }
        }
    }
    imshow("New Image", new_image);
}
```

`saturate_cast<uchar>` 是关键：当 $\alpha \cdot f + \beta > 255$ 时截断为 255，而不是 `uchar` 回绕（如 256→0）导致的花屏。

### 4.2 注册滑条

```cpp
namedWindow("new image", 1);
createTrackbar("contrast",   "new image", &alpha, alpha_max, onTrackbar);
createTrackbar("brightness", "new image", &beta,  beta_max,  onTrackbar);
imshow("new image", srcImage);
waitKey();
```

`createTrackbar` 签名：
```cpp
int createTrackbar(const String& trackbarname, const String& winname,
                   int* value,          // 滑条当前值（双向绑定）
                   int count,           // 最大值
                   TrackbarCallback onChange = 0,   // 回调
                   void* userdata = 0);             // 透传给回调的用户数据
```

回调原型 `void (*)(int pos, void* userdata)`：`pos` 是滑条新值，`userdata` 用于传递上下文（避免全局变量）。

## 5. 官方改进版：convertTo 一行实现

官方 [BasicLinearTransformsTrackbar.cpp](../../mingw-build/samples/cpp/tutorial_code/HighGUI/BasicLinearTransformsTrackbar.cpp) 用 `convertTo` 替代三重循环，更快更简洁：

```cpp
new_image = Mat::zeros(srcImage.size(), srcImage.type());
srcImage.convertTo(new_image, -1, alpha, beta);   // dst = src*alpha + beta
```

`convertTo(dst, rtype, alpha, beta)` 内部做了 SIMD 优化，批量像素运算应优先使用。

## 6. 回调机制设计模式

HighGUI 回调（滑条、鼠标）都是 C 风格函数指针，常用两种模式传上下文：

1. **全局变量**（简单 demo）：本目录源码的做法。
2. **userdata 指针**（工程推荐）：把参数打包成结构体传入，回调内 `static_cast` 还原，参见 [morph_op_process.cpp](../image_process/process/morph_op_process.cpp) 的 `MorphDemo` 写法。

鼠标回调示例骨架：
```cpp
void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) { /* 处理点击 */ }
}
setMouseCallback("win", onMouse, nullptr);
```

## 7. 典型应用场景

- **算法调参面板**：任何带超参数的算法（阈值、核大小、alpha）都可用滑条实时预览。
- **标注工具**：鼠标回调实现点选、框选、多边形标注。
- **视频播放控制**：滑条绑定 `CAP_PROP_POS_FRAMES` 实现进度条拖动。

## 8. 相关官方示例

- [BasicLinearTransformsTrackbar.cpp](../../mingw-build/samples/cpp/tutorial_code/HighGUI/BasicLinearTransformsTrackbar.cpp)：对比度亮度滑条官方实现
- [AddingImagesTrackbar.cpp](../../mingw-build/samples/cpp/tutorial_code/HighGUI/AddingImagesTrackbar.cpp)：双图融合比例滑条
- [changing_contrast_brightness_image.cpp](../../mingw-build/samples/cpp/tutorial_code/ImgProc/changing_contrast_brightness_image/changing_contrast_brightness_image.cpp)：带过曝检测的增强版
