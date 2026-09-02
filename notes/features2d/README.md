# 特征点检测（features2D）

本节讲解**局部特征点（关键点 + 描述子）**的概念与主流算法对比，重点剖析 SURF。对应官方示例 [SURF_detection_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/feature_detection/SURF_detection_Demo.cpp)、[SURF_matching_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/feature_description/SURF_matching_Demo.cpp)、[SURF_FLANN_matching_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/feature_flann_matcher/SURF_FLANN_matching_Demo.cpp)。

## 1. 章节结构

| 子目录 | 主题 |
|--------|------|
| [feature_detection](feature_detection/README.md) | SURF 关键点检测（本章节重点） |

## 2. 特征点 = 关键点 + 描述子

局部特征由两部分组成：

1. **关键点（KeyPoint）**：位置 $(x, y)$、尺度 $\sigma$、主方向 $\theta$、响应强度；
2. **描述子（Descriptor）**：关键点邻域的数值向量（如 SURF 为 64 维），用于跨图像匹配。

```
检测（detect）→ 描述（compute）→ 匹配（match）
```

## 3. 主流算法对比

| 算法 | 检测原理 | 描述子 | 速度 | 精度 | 专利/开源 |
|------|----------|--------|------|------|-----------|
| SIFT | DoG 尺度空间极值 | 128 维梯度直方图（浮点） | 慢 | 高 | 已过期 |
| SURF | Hessian 行列式 + 盒式滤波 | 64 维 Haar 响应（浮点） | 中 | 高 | 有专利（xfeatures2d） |
| ORB | FAST + 金字塔 | 256 位二进制（BRIEF 改进） | 快 | 中 | 开源 |
| BRISK | 多尺度 FAST | 512 位二进制 | 快 | 中 | 开源 |
| AKAZE | 非线性尺度空间 | M-LDB 二进制 | 中 | 较高 | 开源 |

**选型建议**：

- 追求精度、无实时约束 → SIFT/SURF；
- 移动端/嵌入式实时 → ORB（配 `BFMatcher` + `NORM_HAMMING`）；
- 需要抗模糊且开源 → AKAZE，见 [AKAZE_match.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/AKAZE_match.cpp)。

## 4. 匹配器简介

| 匹配器 | 原理 | 适用描述子 |
|--------|------|------------|
| `BFMatcher` | 暴力遍历 + 距离度量（L2/汉明） | 任意 |
| `FlannBasedMatcher` | KD 树/LSH 近似最近邻 | 浮点描述子（SIFT/SURF） |

经典技巧 **Lowe's ratio test**：最近邻距离 / 次近邻距离 < 0.75 才接受，可剔除绝大多数误匹配：

```cpp
if (m.distance < 0.75 * n.distance) good_matches.push_back(m);
```

## 5. 典型应用场景

- **图像拼接（全景）**：特征匹配 → `findHomography` → `stitching`，见 [stitching.cpp](../../mingw-build/samples/cpp/stitching.cpp)。
- **目标识别定位**：匹配 + 单应性矩阵把模板框映射到场景图。
- **视觉里程计/SLAM**：帧间特征跟踪估计相机运动。
- **三维重建**：SfM 以特征点为观测建立点云。

## 6. 相关官方示例

- [feature_homography/SURF_FLANN_matching_homography_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp)：匹配 + 单应性定位
- [Homography/perspective_correction.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/Homography/perspective_correction.cpp)：单应性透视校正
- [matchmethod_orb_akaze_brisk.cpp](../../mingw-build/samples/cpp/matchmethod_orb_akaze_brisk.cpp)：三种二进制特征匹配方法对比
- [AKAZE_tracking/planar_tracking.cpp](../../mingw-build/samples/cpp/tutorial_code/features2D/AKAZE_tracking/planar_tracking.cpp)：平面目标实时跟踪
