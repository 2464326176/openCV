# SURF 特征点检测（feature_detection）

本节剖析 **SURF（Speeded-Up Robust Features）** 算法原理与检测代码。对应官方示例 [SURF_detection_Demo.cpp](../../../mingw-build/samples/cpp/tutorial_code/features2D/feature_detection/SURF_detection_Demo.cpp)。

本目录源码：[surf_detection.cpp](surf_detection.cpp)。

> 注意：SURF 受专利保护，位于 `opencv_contrib` 的 `xfeatures2d` 模块，需编译 contrib 版本才能使用。

## 1. 核心功能

输入：灰度图像

输出：检测到的关键点，以圆圈（半径反映尺度）绘制在原图上

## 2. SURF 算法原理

### 2.1 关键点检测：Hessian 矩阵

SURF 用 Hessian 矩阵行列式检测斑点（blob）。图像 $I$ 在点 $(x, y)$、尺度 $\sigma$ 处的 Hessian 矩阵：

$$
H(x, y, \sigma) =
\begin{bmatrix}
L_{xx}(x, y, \sigma) & L_{xy}(x, y, \sigma) \\
L_{xy}(x, y, \sigma) & L_{yy}(x, y, \sigma)
\end{bmatrix}
$$

其中 $L_{xx}$ 是图像与高斯二阶导的卷积。行列式作为斑点响应：

$$
\det(H) = L_{xx} L_{yy} - L_{xy}^2
$$

### 2.2 盒式滤波近似（加速核心）

高斯二阶导计算昂贵，SURF 用**盒式滤波**（方框模板）近似，配合**积分图**使任意尺度计算量恒定：

$$
D_{xx}, D_{yy}, D_{xy} \approx L_{xx}, L_{yy}, L_{xy}
$$

近似响应：

$$
\det(H_{approx}) = D_{xx} D_{yy} - (0.9 \, D_{xy})^2
$$

### 2.3 尺度空间与主方向

- **尺度空间**：不降采样，而是逐层增大盒式滤波尺寸（9×9 → 15×15 → 21×21...），在 3×3×3 邻域内找行列式局部极值；
- **主方向**：统计关键点邻域 60° 扇形内的 Haar 小波响应和，取最大方向，保证旋转不变性；
- **描述子**：沿主方向取 20s×20s 窗口，分成 4×4 子区域，每个子区域统计 $dx, dy, |dx|, |dy|$ 四项 → **64 维向量**。

## 3. 代码逐段解析

来自 [surf_detection.cpp](surf_detection.cpp)：

```cpp
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
using namespace cv::xfeatures2d;

Mat src = imread("../static/gril/0.jpg", IMREAD_GRAYSCALE);   // 灰度加载

//-- Step 1: Detect the keypoints using SURF Detector
int minHessian = 400;                          // 响应阈值，越小检出越多
Ptr<SURF> detector = SURF::create(minHessian);
std::vector<KeyPoint> keypoints;
detector->detect(src, keypoints);

//-- Draw keypoints
Mat img_keypoints;
drawKeypoints(src, keypoints, img_keypoints);  // 圆圈半径 = 关键点尺度

//-- Show detected (drawn) keypoints
imshow("SURF Keypoints", img_keypoints);
```

### 3.1 关键参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `hessianThreshold` | 100 | 行列式响应阈值，典型 400~1000；越小点越多 |
| `nOctaves` | 4 | 尺度空间组数 |
| `nOctaveLayers` | 3 | 每组层数 |
| `extended` | false | true 时描述子扩展为 128 维 |
| `upright` | false | true 时不计算主方向（更快，无旋转不变性） |

### 3.2 KeyPoint 结构

```cpp
struct KeyPoint {
    Point2f pt;        // 位置
    float   size;      // 尺度（直径）
    float   angle;     // 主方向（弧度）
    float   response;  // 响应强度
    int     octave;    // 所在金字塔层
    int     class_id;
};
```

`drawKeypoints` 默认按尺度画圆；加 `DrawMatchesFlags::DRAW_RICH_KEYPOINTS` 还会画出方向线。

## 4. 从检测到匹配的完整链路

检测只是第一步，完整匹配流程（见 [SURF_FLANN_matching_Demo.cpp](../../../mingw-build/samples/cpp/tutorial_code/features2D/feature_flann_matcher/SURF_FLANN_matching_Demo.cpp)）：

```cpp
Ptr<SURF> detector = SURF::create(minHessian);
detector->detectAndCompute(img1, noArray(), kp1, desc1);   // 检测+描述一步完成
detector->detectAndCompute(img2, noArray(), kp2, desc2);

FlannBasedMatcher matcher;
matcher.knnMatch(desc1, desc2, knn, 2);      // KNN 找最近 2 个

// ratio test 过滤
for (auto& m : knn)
    if (m[0].distance < 0.75 * m[1].distance) good.push_back(m[0]);

drawMatches(img1, kp1, img2, kp2, good, out);
```

## 5. 典型应用场景

- **全景拼接**：SURF 匹配 + 单应性估计。
- **AR 标记跟踪**：平面特征跟踪（[planar_tracking.cpp](../../../mingw-build/samples/cpp/tutorial_code/features2D/AKAZE_tracking/planar_tracking.cpp) 用 AKAZE 同理）。
- **图像检索**：描述子建索引（FLANN/BoW）做相似度搜索。

## 6. 相关官方示例

- [SURF_detection_Demo.cpp](../../../mingw-build/samples/cpp/tutorial_code/features2D/feature_detection/SURF_detection_Demo.cpp)：官方检测演示（本目录源码改编自它）
- [SURF_matching_Demo.cpp](../../../mingw-build/samples/cpp/tutorial_code/features2D/feature_description/SURF_matching_Demo.cpp)：BFMatcher 匹配
- [SURF_FLANN_matching_homography_Demo.cpp](../../../mingw-build/samples/cpp/tutorial_code/features2D/feature_homography/SURF_FLANN_matching_homography_Demo.cpp)：匹配 + 单应性目标定位
