# 图像拼接与全景（imageStitching）

本节讲解**图像拼接**的两条路线：① 手工流程（特征匹配 + RANSAC 单应 + `warpPerspective`），理解每一步原理；② 官方 `Stitcher` 一键全景拼接。对应官方示例 [stitching.cpp](../../mingw-build/samples/cpp/stitching.cpp)、[stitching_detailed.cpp](../../mingw-build/samples/cpp/stitching_detailed.cpp)、[SURF_FLANN_matching_homography_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp)。

本目录源码：[stitch.cpp](stitch.cpp)（手工特征匹配拼接）、[stitcher_panorama.cpp](stitcher_panorama.cpp)（Stitcher 一键全景）。

## 1. 章节文件索引

| 文件 | 主题 |
|------|------|
| [stitch.cpp](stitch.cpp) | 手工流程：SIFT + BF 匹配 + RANSAC 单应 + 拼接 |
| [stitcher_panorama.cpp](stitcher_panorama.cpp) | 官方 `Stitcher` 全景拼接 |

## 2. 拼接四步原理

```
特征检测 → 特征匹配 → 求单应性矩阵 H → 透视变换 + 融合
```

### 2.1 特征检测与描述

两幅有重叠的图像各自提取特征点与描述子（SIFT/SURF/ORB，见 [features2d](../features2d/README.md)）。本目录 [stitch.cpp](stitch.cpp) 默认用 **ORB**（本机 OpenCV 未编 `xfeatures2d`）；若你的构建含 contrib，可换成 `xfeatures2d::SIFT::create()` 并用 `NORM_L2`。

### 2.2 特征匹配 + Lowe's ratio test

用 `knnMatch` 取最近邻与次近邻，比率 < 0.75 才保留，剔除大部分误匹配：

```cpp
matcher.knnMatch(desc1, desc2, knnMatches, 2);
for (size_t i = 0; i < knnMatches.size(); i++) {
    if (knnMatches[i][0].distance < 0.75 * knnMatches[i][1].distance)
        goodMatches.push_back(knnMatches[i][0]);
}
```

### 2.3 RANSAC 求单应性矩阵

两幅图之间存在**单应性（射影）变换**，用 3×3 矩阵 $H$ 表示（齐次坐标）：

$$
\begin{bmatrix} x' \\ y' \\ 1 \end{bmatrix}
= H
\begin{bmatrix} x \\ y \\ 1 \end{bmatrix},
\quad
H =
\begin{bmatrix}
h_{11} & h_{12} & h_{13} \\
h_{21} & h_{22} & h_{23} \\
h_{31} & h_{32} & 1
\end{bmatrix}
$$

`findHomography` 用 **RANSAC** 迭代，在含噪声的匹配点对中鲁棒地估计 $H$（默认剔除离群点）：

```cpp
Mat H = findHomography(pts2, pts1, RANSAC, 3.0);
// pts2（图2特征点）→ pts1（图1特征点），即把图2变换到图1坐标系
```

RANSAC 原理：随机选 4 对点求 $H$，统计满足该 $H$ 的内点数量，迭代多次取内点最多的解。`3.0` 是重投影误差阈值，小于该值的点视为内点。

### 2.4 透视变换 + 拼接

```cpp
warpPerspective(img2, result, H, Size(img1.cols + img2.cols, img1.rows));
Mat roi(result, Rect(0, 0, img1.cols, img1.rows));
img1.copyTo(roi);   // 左半部分直接覆盖图1，重叠区由图1覆盖
```

`warpPerspective` 把图2按 $H$ 投影到目标画布，画布宽度取两图之和；再把图1贴到左侧。重叠区的融合可进一步用 `detail::MultiBandBlender` 消除接缝。

## 3. 官方 Stitcher 一键拼接

来自 [stitcher_panorama.cpp](stitcher_panorama.cpp)——`Stitcher` 把上述流程封装为黑盒：

```cpp
Ptr<Stitcher> stitcher = Stitcher::create(Stitcher::PANORAMA);
Mat pano;
Stitcher::Status status = stitcher->stitch(images, pano);
if (status != Stitcher::OK) { ... }
```

内部自动完成：特征匹配 → 相机参数估计 → 曝光补偿 → 多频段融合（Multi-Band Blending）。进阶调参版本见官方 [stitching_detailed.cpp](../../mingw-build/samples/cpp/stitching_detailed.cpp)。

## 4. 参数调优指南

| 参数/环节 | 影响 |
|-----------|------|
| 特征检测器 | SIFT/SURF 精度高、慢（需 contrib）；ORB 快、抗旋转稍弱 |
| ratio test 阈值 | 0.75~0.8，越小匹配越严格 |
| RANSAC 阈值 | 越小内点判定越严，单应越准，但点太少会失败 |
| 输入图像顺序 | 应保证相邻图有足够重叠（30%+） |

## 5. 常见问题与易错点

| 现象 | 原因 | 解决 |
|------|------|------|
| `findHomography` 返回空 | 匹配点太少 | 增大特征点数 / 放宽 ratio test |
| 拼接结果歪斜错位 | 单应估计不准 | 提高 RANSAC 阈值内的内点质量、检查重叠 |
| 拼接处明显接缝 | 未做曝光/融合 | 用 `Stitcher` 或 `MultiBandBlender` |
| 全景变形严重 | 相机纯旋转假设不成立 | 用圆柱/球面投影（`Stitcher` 内部处理） |

## 6. 典型应用场景

- **全景拍照**：手机/相机的 Panorama 模式（`Stitcher`）。
- **卫星/航拍图拼接**：多帧影像合成大场景地图。
- **文档扫描拼接**：分段扫描的 A4 文档拼回整页。
- **监控大视野**：多路摄像头画面融合为广角视野。

## 7. 相关官方示例

- [stitching.cpp](../../mingw-build/samples/cpp/stitching.cpp)：一键拼接（本目录 stitcher_panorama.cpp 改编自它）
- [stitching_detailed.cpp](../../mingw-build/samples/cpp/stitching_detailed.cpp)：完整可调参拼接流水线
- [feature_homography/SURF_FLANN_matching_homography_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp)：匹配 + 单应框定位（本目录 stitch.cpp 前半段同思路）
- [Homography/panorama_stitching_rotating_camera.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/Homography/panorama_stitching_rotating_camera.cpp)：旋转相机全景
