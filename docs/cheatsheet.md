# OpenCV C++ 速查表（Cheatsheet）

> 一页纸速查：把最常用的 API、参数取值、易混点集中在此，省去翻 `ch0x_*.md`。
> 详细原理与参数表见各章文档（`ch01_core.md` ~ `ch08_gui_gapi_gpu.md`）。
> 约定：**所有 `cv::` 命名空间省略前缀**；`src` 为输入 `Mat`，`dst` 为输出 `Mat`。

---

## 1. 读写与基础

```cpp
Mat img = imread("a.jpg", IMREAD_COLOR);     // 默认 BGR；IMREAD_GRAYSCALE / IMREAD_UNCHANGED
Mat g;  cvtColor(img, g, COLOR_BGR2GRAY);     // 最常用色彩转换
imwrite("out.png", dst);                      // 支持 png/jpg/bmp
Mat yuv = imread("b.png", IMREAD_UNCHANGED);  // 含 alpha 时要用 UNCHANGED
```

| 操作 | 代码 | 注意 |
|------|------|------|
| 克隆（深拷贝） | `Mat c = src.clone();` | **改 `c` 不影响 `src`** |
| 浅拷贝（共享数据） | `Mat s = src;` 或 `Mat s = src(Rect(...));` | ROI 改了原图也变，危险 |
| 取 ROI 安全写法 | `Mat roi; src(Rect(x,y,w,h)).copyTo(roi);` | 别写 `Mat r = src(Rect(...));` 然后改 r |
| 类型转换 | `src.convertTo(dst, CV_32F, 1/255.0);` | 第二个参数是**目标类型**，第三/四是缩放/偏移 |

---

## 2. 图像滤波（平滑去噪）

| 函数 | 典型调用 | 一句话区别 |
|------|---------|-----------|
| `GaussianBlur` | `(src, dst, Size(5,5), 1.4)` | 最通用低通，核大小**必须奇数**，σ 给 0 让函数按核大小自动算 |
| `medianBlur` | `(src, dst, 5)` | **椒盐噪声杀手**，核大小奇数 |
| `bilateralFilter` | `(src, dst, 9, 75, 75)` | 保边去噪（值域+空间双核），美颜磨皮基础；`d`/`σ_color`/`σ_space` 三个越大越平滑越慢 |
| `blur` | `(src, dst, Size(3,3))` | 均值滤波，简单糊边缘 |

>  滤波前如果是 `CV_8U`，结果也是 `CV_8U`；要防溢出先 `convertTo` 到 `CV_32F`。

---

## 3. 形态学（针对二值 mask）

```cpp
Mat k = getStructuringElement(MORPH_RECT, Size(3,3));  // 形状: RECT / ELLIPSE / CROSS
morphologyEx(bin, dst, MORPH_OPEN,  k);                // 开=先蚀后胀：去白噪点
morphologyEx(bin, dst, MORPH_CLOSE, k);                // 闭=先胀后蚀：填黑洞
// MORPH_GRADIENT 轮廓 / MORPH_TOPHAT 亮细节 / MORPH_BLACKHAT 暗细节 / MORPH_HITMISS 形状
```

- 核大小 `ksize` 必须奇数；去规则噪点用 `RECT`，不规则物体用 `ELLIPSE`。
- 空洞比核还大 → 闭运算填不上，调大 `ksize`。

---

## 4. 边缘检测

| 算子 | 调用 | 要点 |
|------|------|------|
| `Sobel` | `Sobel(g, gx, CV_16S, 1, 0, 3); convertScaleAbs(gx, gx8);` | **中间必须用 CV_16S/CV_32F**，最后才转回 8U，否则负值被截断 |
| `Scharr` | `Scharr(g, gx, CV_16S, 1, 0);` | 3×3 下比 Sobel 更准 |
| `Laplacian` | `Laplacian(g, lap, CV_32F, 3); abs(lap)` | 二阶，对噪声敏感，先 `GaussianBlur` |
| `Canny` | `Canny(g, c, 100, 200);` | **双阈值高低比 2:1~3:1**；先降噪 |

> 通用三步：滤波 → 梯度幅值 → 双阈值。Canny 对噪声极敏感，**必须**先 `GaussianBlur(Size(5,5),1.4)`。

---

## 5. 阈值化

| 函数 | 调用 | 注意 |
|------|------|------|
| `threshold` | `threshold(g, b, 127, 255, THRESH_BINARY \| THRESH_OTSU)` | 给了 `THRESH_OTSU` 就忽略手动阈值，自动算 |
| `adaptiveThreshold` | `(g, b, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 51, 3)` | `blockSize` 必须奇数；`C` 是从均值里减的常数 |

> HSV 里 **H 范围是 0~179**（不是 0~360），S/V 才是 0~255。`inRange` 分割失败常因 H 写成了 0~360。

---

## 6. 几何变换

| 函数 | 调用 | 用途 |
|------|------|------|
| `resize` | `resize(src, dst, Size(), 0.5, 0.5, INTER_AREA)` | 缩小用 `INTER_AREA`，放大用 `INTER_LINEAR`/`INTER_CUBIC` |
| `warpAffine` | `warpAffine(src, dst, M, dsize)` | 旋转/平移/缩放，`M` 是 2×3（`getRotationMatrix2D`） |
| `warpPerspective` | `warpPerspective(src, dst, H, dsize)` | 透视校正（发票/文档），`H` 是 3×3 单应矩阵 |
| `remap` | `remap(src, dst, mapx, mapy, INTER_LINEAR)` | 自定义形变（鱼眼校正、拼接） |
| `getRotationMatrix2D` | `getRotationMatrix2D(center, angle, scale)` | 生成旋转仿射矩阵 |

---

## 7. 直方图

```cpp
int histSize = 256; float range[] = {0, 256}; const float* ranges = range;
calcHist(&g, 1, 0, Mat(), hist, 1, &histSize, &ranges);   //  ranges 上界是 256 是 exclusive
normalize(hist, hist, 0, 255, NORM_MINMAX);
equalizeHist(g, g);          // 全局均衡，天空/人脸易过曝
CLAHE c; c = createCLAHE(2.0, Size(8,8)); c.apply(g, dst);  // 自适应，基本吊打原版
```

- `calcHist` 的 `ranges` 用 `[0, 256)` 不是 `[0, 255]`（上界独占）。
- 显示直方图为空/形状不对 → 八成是 `ranges` 写错。

---

## 8. 特征检测与匹配

| 检测/描述 | 调用要点 |
|----------|---------|
| `goodFeaturesToTrack` | Shi-Tomasi 角点；`(g, corners, maxCorners, qualityLevel, minDistance)` |
| `cornerHarris` | 响应值极小（浮点 0.x），`normalize` 到 0~255 再 `threshold` 才看得见 |
| `ORB::create(500)` | 免费+快+带方向，综合首选；`detectAndCompute` 一次出特征点与描述符 |
| `BFMatcher(NORM_HAMMING)` | 二进制描述符（ORB/BRISK/AKAZE）用 HAMMING 距离；SIFT/SURF 用 `NORM_L2` |
| Lowe 比例测试 | `m[0].distance < 0.75 * m[1].distance` → 保留，剔除歧义匹配 |
| `findHomography(pts1, pts2, RANSAC, 3.0)` | RANSAC 剔除离群点；`3.0` 是重投影误差阈值（像素） |

> SURF/SIFT 编译报错 "has no member" → 没编 `opencv_contrib`，需设 `OPENCV_EXTRA_MODULES_PATH`。

---

## 9. 短视频 / 光流

```cpp
VideoCapture cap("v.avi");  cap >> frame;        // 读帧
VideoWriter w("o.avi", VideoWriter::fourcc('M','J','P','G'), 25, Size(w,h));
calcOpticalFlowPyrLK(prevG, nextG, p0, p1, status, err, Size(21,21), 3);  // LK 稀疏
calcOpticalFlowFarneback(pG, nG, flow, 0.5, 3, 15, 3, 5, 1.2, 0);         // Farneback 稠密
```

- `waitKey(0)` 才会停住窗口；`waitKey(>0)` 是按毫秒等待。**没有 `waitKey` HighGUI 窗口一闪而过**。
- 光流 `status` 必须检查，失败点是垃圾值。

---

## 10. 配准与频域

| 函数 | 要点 |
|------|------|
| `findTransformECC` | 灰度配准，返回变换矩阵；大图先降采样 |
| `estimateAffinePartial2D` / `findHomography` | 多帧降噪对齐用 |
| `dft(src, dst, DFT_COMPLEX_OUTPUT)` | 输入先 `getOptimalDFTSize` 补零；低频在四角，看谱要象限交换 |
| 频域滤波 | 构造中心化掩膜 → `swapQuadrants` 转回未移位布局 → 逐元素乘 `F` → `idft` |

---

## 11. 评估指标（项目 `demo_algorithms_common` 提供）

| 指标 | 含义 | 越高越好？ |
|------|------|:---------:|
| `psnr(a,b)` | 峰值信噪比 dB | ✅ |
| `ssim(a,b)` | 结构相似性 (0,1] | ✅ |
| `mae(a,b)` / `mse(a,b)` | 平均/均方绝对误差 | ❌ |
| `loe(a,b)` | 光照顺序误差 | ❌ |
| `niqe(a)` | 无参考质量（仅灰度） | ❌（越小越好） |

---

## 12. 最容易踩的 10 个坑（速记版）

1. ROI 浅拷贝改了原图 → 用 `clone()` / `copyTo()`
2. 窗口一闪而过 → 末尾加 `waitKey(>0)`
3. Sobel 全花屏 → 中间没用 `CV_16S`/`CV_32F`，直接 `saturate_cast` 截 0
4. Canny 碎成片 → 没先 `GaussianBlur`
5. `calcHist` 形状不对 → `ranges` 用 `[0,256)` 不是 `[0,255]`
6. HSV `inRange` 不工作 → H 写成了 0~360，应为 0~179
7. `cornerHarris` 看不见 → R 值极小，需 `normalize` + `threshold`
8. SURF/SIFT 编译失败 → 没编 `opencv_contrib`
9. Debevec HDR 崩断言 → 用 **Mertens 曝光融合**绕开
10. `*.exe` 找不到 data → `cd` 到 exe 目录，或用 `getImagePath()`

> 更完整的 25+ 条排障清单见 [faq_troubleshooting.md](faq_troubleshooting.md)。
