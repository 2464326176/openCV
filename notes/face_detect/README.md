# Haar 级联人脸检测（facedetect）

本节讲解 **Haar-like 特征 + AdaBoost 级联分类器**的经典人脸检测方案，以及相机原始数据（NV21）处理流程。对应官方示例 [face_detect.cpp](../../mingw-build/samples/cpp/face_detect.cpp)（本目录 [face_detect.cpp](face_detect.cpp) 即官方源码）。

## 1. 章节文件索引

| 文件 | 主题 |
|------|------|
| [face_detect.cpp](face_detect.cpp) | 官方人脸+眼睛嵌套检测完整示例 |
| [single_frame_process.cpp](single_frame_process.cpp) | NV21 原始数据转 BGR + 平滑/锐化 |
| [image_process.cpp](image_process.cpp) | 图像读写基础（imread/imshow/imwrite） |
| [test.cpp](test.cpp) | NV21 与 HDR 成像实验 |

## 2. Haar 级联检测原理

### 2.1 Haar-like 特征

Viola-Jones 框架使用矩形差分特征，每个特征值为白色矩形区域像素和减去黑色区域像素和。
$$
f = \sum_{(x,y) \in white} I(x,y) - \sum_{(x,y) \in black} I(x,y)
$$

常见特征形状：
```
┌───┬───┐    ┌───────┐    ┌──┬──┬──┐    ┌───┬───┐
│白│黑│    │   白   │    │白│黑│白│    │白│黑│
└───┴───┘    ├───────┤    └──┴──┴──┘    ├───┼───┤
边缘特征      │   黑   │       线性特征       │白│黑│
              └───────┘                    └───┴───┘
                              对角特征
```

这些特征能捕捉人脸的明暗结构（如眼睛比脸颊暗、鼻梁两侧比鼻梁亮）。

### 2.2 积分图加速

任意矩形区域像素和可用 4 次查表 $O(1)$ 算出，使特征计算与尺寸无关：

$$
S(x, y) = \sum_{i \le x, j \le y} I(i, j), \quad
RectSum = S(D) + S(A) - S(B) - S(C)
$$

### 2.3 AdaBoost 级联

- 单个弱分类器：一个特征 + 阈值，准确率略高于随机。
- AdaBoost 加权组合数百个弱分类器形成**强分类器**。
- **级联结构**：把强分类器拆成多级，前几级只用极少特征快速排除大部分背景窗口，越靠后越复杂。绝大多数背景在第一级就被拒绝，实现实时检测。

## 3. detectMultiScale API

```cpp
void detectMultiScale(InputArray image,
                      CV_OUT std::vector<Rect>& objects,
                      double scaleFactor = 1.1,  // 图像金字塔缩放比例
                      int minNeighbors = 3,      // 候选框最少邻居数（去误检）
                      int flags = 0,
                      Size minSize = Size(),     // 最小目标尺寸
                      Size maxSize = Size());    // 最大目标尺寸
```

| 参数 | 影响 |
|------|------|
| `scaleFactor` | 越小检测越精细越慢（1.05~1.3） |
| `minNeighbors` | 越大误检越少，但可能漏检（2~5） |
| `minSize` | 过滤小窗口，加速检测 |
| `flags` | 如 `CASCADE_SCALE_IMAGE`、`CASCADE_FIND_BIGGEST_OBJECT` |

## 4. 代码逐段解读

### 4.1 加载分类器

来自 [face_detect.cpp](face_detect.cpp)：
```cpp
CascadeClassifier cascade, nestedCascade;
cascadeName = "data/haarcascades/haarcascade_frontalface_alt.xml";       // 人脸
nestedCascadeName = "data/haarcascades/haarcascade_eye_tree_eyeglasses.xml"; // 眼睛
if (!cascade.load(samples::findFile(cascadeName))) {
    cerr << "ERROR: Could not load classifier cascade" << endl;
    return -1;
}
```

分类器是训练好的 XML 文件，OpenCV 自带人脸、眼睛、微笑、人体等十余种。

### 4.2 检测主流程

```cpp
cvtColor(img, gray, COLOR_BGR2GRAY);            // 1. 灰度化
double fx = 1 / scale;
resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);  // 2. 缩小加速
equalizeHist(smallImg, smallImg);               // 3. 直方图均衡化抗光照
cascade.detectMultiScale(smallImg, faces,
    1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));   // 4. 多尺度检测
```

**预处理三件套**：灰度化（Haar 特征只用灰度）→ 缩小（减少计算量）→ 均衡化（抑制光照差异）。

### 4.3 嵌套检测（人脸中找眼睛）
```cpp
for (size_t i = 0; i < faces.size(); i++) {
    Rect r = faces[i];
    // 宽高比接近 1 画圆，否则画矩形
    double aspect_ratio = (double)r.width / r.height;
    if (0.75 < aspect_ratio && aspect_ratio < 1.3) {
        circle(img, center, radius, color, 3, 8, 0);
    } else {
        rectangle(img, ...);
    }
    // 在人脸 ROI 内做二级检测
    smallImgROI = smallImg(r);
    nestedCascade.detectMultiScale(smallImgROI, nestedObjects,
        1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
}
```

**嵌套检测**思想：先粗后细，在人脸框内再跑眼睛分类器，既提速又降低误检。

### 4.4 翻转增强

```cpp
if (tryflip) {
    flip(smallImg, smallImg, 1);   // 水平翻转再检测一次
    // 把翻转图上的检测框映射回原图坐标
    faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
}
```

侧脸朝向不同时，翻转图像再检测可提高召回率。

## 5. NV21 原始数据处理

来自 [single_frame_process.cpp](single_frame_process.cpp)——相机 ISP 输出的 NV21 数据处理。
```cpp
// NV21 布局：平面 Y(w*h) + 交错 VU 平面(w*h/2)
cv::Mat nv21Mat(height + height / 2, width, CV_8UC1, nv21_data.data());

cv::Mat bgrMat;
cv::cvtColor(nv21Mat, bgrMat, cv::COLOR_YUV2BGR_NV21);   // 一行完成转换
// 平滑与锐化
cv::GaussianBlur(bgrMat, smoothed, cv::Size(5, 5), 0);
```

文件还实现了两种锐化：
```cpp
// 拉普拉斯锐化：原图 + alpha * 拉普拉斯
cv::Laplacian(input, laplacian, CV_32F);
cv::convertScaleAbs(input + alpha * laplacian, sharpened);

// 非锐化掩蔽（USM）：原图*(1+amount) - 模糊*amount
cv::addWeighted(input, 1.0 + amount, blurred, -amount, 0, sharpened);
```

## 6. 典型应用场景

- **门禁/考勤**：人脸检测 + 人脸识别（FaceRecognizerSF/DNN）两级流水线。
- **相机人脸对焦**：检测框驱动 3A 算法对人脸测光对焦。
- **驾驶员监控（DMS）**：嵌套检测人脸→眼睛→闭眼状态。
- **图像质量分析**：NV21 直取 + 锐度评估用于相机 tuning。

## 7. 局限与替代方案

Haar 级联速度快但精度有限（侧脸、遮挡、光照剧变易漏检）。现代替代方案：
- **DNN 人脸检测**：`opencv2/dnn` + SSD/YuNet 模型，见 [dbt_face_detection.cpp](../../mingw-build/samples/cpp/dbt_face_detection.cpp)
- **HOG + SVM 行人检测**：见 [peopledetect.cpp](../../mingw-build/samples/cpp/peopledetect.cpp)
- **微笑检测**：[smiledetect.cpp](../../mingw-build/samples/cpp/smiledetect.cpp)

## 8. 相关官方示例

- [face_detect.cpp](../../mingw-build/samples/cpp/face_detect.cpp)：官方完整示例（本目录同名文件）
- [facial_features.cpp](../../mingw-build/samples/cpp/facial_features.cpp)：面部特征点检测
- [train_HOG.cpp](../../mingw-build/samples/cpp/train_HOG.cpp)：训练 HOG 检测器
