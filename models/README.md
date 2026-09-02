# models/ — DNN 模型库

本目录存放供 `learn/`、`algorithms/`、`notes/` 各模块使用的预训练 DNN 模型
（ONNX 格式为主，部分为 OpenCV 兼容的 `.caffemodel` / `.pb`）。

> 与 `data/` 和 `mingw-build/` 不同：**新增模型文件可以写入本目录**，但需同步
> 更新下方 §1 模型清单，写清下载来源、版本、输入尺寸、license。

## 1. 模型清单

| 文件名 | 任务 | 输入尺寸 | 来源 / 版本 | 参考文档 | 使用者 |
|--------|------|----------|-------------|----------|--------|
| `face_detection_yunet_2023mar.onnx` | 人脸检测 (YuNet) | 动态 (推荐 320×320 或 640×640) | OpenCV Zoo 2023-03 版 | [opencv_zoo/models/face_detection_yunet](https://github.com/opencv/opencv_zoo/tree/main/models/face_detection_yunet) | `beauty` (待接入)、`L4_detect_calib` / `L5_ml_gapi` 练习 |
| `face_recognition_sface_2021dec.onnx` | 人脸识别 (SFace, 128-d 特征) | 112×112 (RGB BGR 均可，代码内部转) | OpenCV Zoo 2021-12 版 | [opencv_zoo/models/face_recognition_sface](https://github.com/opencv/opencv_zoo/tree/main/models/face_recognition_sface) | `L5_ml_gapi/13_digits_dnn.cpp`；未来 face_recognition 小流水线 |

## 2. 使用约定

### 2.1 从代码定位模型

代码统一通过 `getModelPath(name)` 或 `ALGO_MODELS_ROOT` 宏访问：

```cpp
// learn/ 与 common 单元 (见 common/opencv_utils.h)
std::string yunet = getModelPath("face_detection_yunet_2023mar.onnx");

// algorithms/ 子项目 (注入 ALGO_MODELS_ROOT 宏)
cv::String sface = cv::String(ALGO_MODELS_ROOT) + "/face_recognition_sface_2021dec.onnx";
```

无论 CWD 是什么，这两个入口都会自动回退到 `../models/` / `../../models/` 等相对路径
寻找，优先找编译期注入的 `*_ROOT` 宏。

### 2.2 YuNet 使用片段（备忘）

```cpp
cv::Ptr<cv::FaceDetectorYN> detector = cv::FaceDetectorYN::create(
    yunetPath, "", cv::Size(320, 320), 0.9f, 0.3f, 5000);
// setInputSize 之后再 detect:
// detector->setInputSize(frame.size());
// cv::Mat faces; // N×15: [x,y,w,h, score, x1,y1,..x5,y5]
// detector->detect(frame, faces);
```

### 2.3 SFace 使用片段（备忘）

```cpp
cv::Ptr<cv::FaceRecognizerSF> recognizer = cv::FaceRecognizerSF::create(sfacePath, "");
// 1. 先 alignCrop 把检测框内人脸对齐到 112×112:
//    cv::Mat aligned; recognizer->alignCrop(frame, faces.row(i), aligned);
// 2. 再 feature 提取 128 维:
//    cv::Mat feat; recognizer->feature(aligned, feat);
// 3. match 两张特征：recognizer->match(feat1, feat2, cv::FaceRecognizerSF::FR_COSINE)
//    > 0.363 一般认为是同一人。
```

## 3. License 说明

所有模型来自官方 OpenCV Zoo，遵循 Apache License 2.0：
- 原文见 <https://github.com/opencv/opencv_zoo/blob/main/LICENSE>
- 如需在商业产品中使用，请自行核对各子模型的具体 license。

## 4. 新增模型 Checklist

在本目录下新增 `.onnx` / `.caffemodel` / `.pb` 前，务必：

1. [ ] 在上方表格新增一行：文件名 / 任务 / 输入尺寸 / 来源 / 参考文档 / 使用者
2. [ ] 在 [根 README §2.1 目录角色与入口](../README.md#21-目录角色与入口) 同步说明
3. [ ] 若是大文件（> 50MB），优先用 Git LFS 或写 `_placeholder.url`，避免仓库膨胀
4. [ ] 在 `common/opencv_utils.h` 的 `getModelPath` 相关逻辑里（如需要）补充路径别名
5. [ ] 确认训练集来源与 license 合规
