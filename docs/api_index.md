# API 与资源交叉索引（四维映射）

> 把四类资源按**主题**对齐，方便"想做 X → 从哪学 → 哪个 demo 直接跑 → 官方源码怎么写"一条龙定位。
>
> 四类资源：
> 1. **learn/** —— 分层逐题练习（L0~L5，每题独立 `main`）→ [learn/README](../learn/README.md)
> 2. **notes/** —— 毛星云体例笔记（按主题子目录）→ [notes/README](../notes/README.md)
> 3. **algorithms/** —— 17 个端到端算法模块 → [algorithms/README](../algorithms/README.md)
> 4. **mingw-build/samples/cpp/** —— OpenCV 官方 233 个 C++ 示例（含 `tutorial_code/` 136 个）

> 官方 demo 文件名以 `mingw-build/samples/cpp/...` 为基准，部分示例在 `tutorial_code/` 子目录下，
> 行末标注 `*` 的用文件名前缀，可在 `samples/cpp` 目录内搜索。

---

## 1. 基础：IO / Mat / 色彩

| 主题 | learn | notes | algorithms | 官方 demo (samples/cpp) |
|------|-------|-------|-----------|------------------------|
| 读写 / 窗口 | L0: `01_hello_imread` `02_named_window_trackbar` `03_mouse_roi` | `highgui/` | — | `opencv_version.cpp` `image.cpp` |
| Mat 类型 / 像素 | L1: `01_mat_create_type` `02_pixel_scan` `03_lut_color_reduce` `04_mask_convolution` | `image_process/mat/` `image_process/image/` | — | `mat_mask_operations.cpp`* |
| 视频捕获 | L0: `05_videocapture_camera` `08_videowriter` | `video_tracking/` | `optical_flow` | `videocapture_basic.cpp` `videowriter.cpp` |

---

## 2. 滤波 / 形态学 / 边缘

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| 平滑滤波 | L2: `01_smoothing` `02_filter2d` `03_linear_transform_colormap` | `image_process/morphology/` `image_transformation/` | `denoise_single` `deblur` | `smoothing.cpp` `filter2D_demo.cpp` |
| 形态学 | L2: `04_erode_dilate` `05_morphology_ex` `06_hit_miss` | `image_process/morphology/` | `morphology` | `morphology2.cpp` |
| 边缘检测 | L2: `10_sobel` `11_scharr` `12_laplacian` `13_canny` | `image_transformation/` | `edge_detection` | `edge.cpp` |

---

## 3. 阈值 / 分割 / 轮廓

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| 阈值化 | L2: `08_threshold` `09_inrange_hsv` `27_equalize_clahe` | `histogram_match/` | `segmentation` | `threshold.cpp` `clahe.cpp` |
| 直方图 | L2: `26_calc_hist` `28_compare_hist` `29_backproject` | `histogram_match/` | — | `histogram.cpp` `histogram_calculation.cpp` |
| 轮廓 / 连通域 | L3: `10_find_contours` `11_convex_hull` `12_bounding_shapes` `13_moments_hu` `14_point_polygon_test` `15_general_contours` | `image_segmentation/` | `segmentation` | `contours2.cpp` `convexhull.cpp` |
| 分割算法 | L4: `07_grabcut` `06_inpaint` | `image_segmentation/` | `segmentation` `inpaint` | `grabcut.cpp` `watershed.cpp` `segment_objects.cpp` |
| 距离变换 / 分水岭 | — | `image_segmentation/` | `segmentation`(Watershed) | `distrans.cpp` `watershed.cpp` |

---

## 4. 几何变换 / 霍夫

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| 仿射 / 透视 / 重映射 | L2: `17_warp_affine` `18_warp_perspective` `19_remap` `20_copy_make_border` `21_polar_transform` | `image_transformation/` | `template_matching`(仿射合成) | `geometric_transformations.cpp` `remap.cpp` |
| 霍夫直线 / 圆 | L2: `14_hough_lines` `15_hough_lines_p` `16_hough_circles` `36_generalized_hough` | `hough_transform/` | `hough_transform` | `houghlines.cpp` `houghcircles.cpp` `generalized_hough.cpp` |

---

## 5. 特征 / 匹配 / 立体

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| 角点检测 | L3: `01_corner_harris` `02_good_features` `03_corner_subpix` `04_corner_detector` | `harris_detect/` `features2d/` | `feature_detection` | `cornerHarris_demo.cpp` `goodFeaturesToTrack_Demo.cpp` |
| 描述符 / 匹配 | L3: `05_orb_detect_match` `06_akaze_match` `07_bf_match_lowe` `08_flann_match` `09_homography` `25_homography_decompose` | `features2d/` `features2d/feature_detection/` | `feature_detection` | `matchmethod_orb_akaze_brisk.cpp` `homography_from_camera_displacement.cpp` |
| MSER / 线特征 | L3: `23_mser` `24_blob_lsd` | `features2d/` | `edge_detection`(DoG) | `mser.cpp` `lsd_lines.cpp` |
| 模板匹配 | L2: `30_match_template` `32_phase_correlate` | `image_process/image_algo/` | `template_matching` | `template_matching.cpp` `phase_corr.cpp` |
| 立体匹配 | L4: `14_stereo_match` `13_epipolar` `19_essential_mat` `20_pnp_pose` | `camera_calibration/` | `stereo` | `stereo_match.cpp` `stereo_calib.cpp` `epipolar_lines.cpp` |
| 相机标定 | L4: `12_camera_calib` `21_charuco_detect` `04_aruco_*` | `camera_calibration/` | — | `calibration.cpp` `charuco_calibration.cpp` |

---

## 6. 视频 / 光流 / 运动

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| 背景建模 | L3: `16_bg_subtract_mog2` | `video_tracking/` | — | `bgfg_segm.cpp` |
| 稀疏光流 LK | L3: `17_lk_optical_flow` `26_lk_stepwise` | `video_tracking/` | `optical_flow` | `lkdemo.cpp` `optical_flow.cpp` |
| 稠密光流 | L3: `18_farneback_dense` `19_dis_opticalflow` | `video_tracking/` | `optical_flow` | `fback.cpp` `simpleflow_demo.cpp` |
| 均值漂移 / CamShift | L3: `20_camshift` `21_meanshift` | `video_tracking/` | — | `camshiftdemo.cpp` |
| 卡尔曼 | L3: `22_kalman` | `video_tracking/` | — | `kalman.cpp` |

---

## 7. ISP 算法族（algorithms 专属，learn/notes 偏原理）

| 主题 | learn / notes（原理侧） | algorithms 模块 | 官方 demo |
|------|----------------------|---------------|-----------|
| 单帧降噪 | L2: `01_smoothing`；`algorithms/watermark`(DFT 域) | `denoise_single` `frequency_domain` | `denoise.cpp`* `non_linear_svms.cpp`* |
| 多帧降噪 / 配准 | L3: `09_homography`；`algorithms/common/nv21_io` | `denoise_multi` | `image_alignment.cpp` |
| HDR / 曝光融合 | L4: `11_hdr` | `hdr` | `hdr_imaging.cpp` `tonemap.cpp` |
| 夜景增强 | — | `night_scene` | `decolor.cpp` `seamless_cloning.cpp` |
| 美颜 | `notes/photo_editing/` | `beauty` | `face_beautification.cpp`(tutorial_code/gapi) |
| 水印 / 修复 | L4: `06_inpaint` | `watermark` `inpaint` | `inpaint.cpp` `cloning_demo.cpp` |
| 频域滤波 | L1: `07_dft_spectrum` | `frequency_domain` | `fft.cpp` `frequency_filtering.cpp` |

---

## 8. 人脸 / 检测 / DNN

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| 人脸检测 | L4: `01_cascade_face` `17_facial_features` `18_smile_detect` `16_dbt_face_detection` | `face_detect/` | `beauty`(接 YuNet) | `facedetect.cpp` `facerec_demo.cpp` |
| 二维码 / ArUco | L4: `03_qrcode` `04_aruco_create` `05_aruco_detect` `21_charuco_detect` | — | — | `qrcode_example.cpp` `aruco_detect.cpp` |
| 目标检测 / HOG | L4: `02_hog_pedestrian` `14_stereo_match` | `face_detect/` | — | `hog.cpp` `pedestrian_detection.cpp` |
| DNN 推理 | L5: `13_digits_dnn` `16_hog_svm_train` | — | `feature_detection`(可扩) | `classification.cpp` `object_detection.cpp` |
| 人脸识别 (ONNX) | — | — | `beauty`/`common`(接 SFace) | `face_recognition.cpp`* |

> **模型位置**：`models/face_detection_yunet_2023mar.onnx` + `face_recognition_sface_2021dec.onnx`，
> 调用速查见 [models/README](../models/README.md)。

---

## 9. 机器学习 / GPU

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| KMeans | L5: `01_kmeans` | `image_segmentation/`(KMeans 量化) | `segmentation` | `kmeans.cpp` |
| SVM / PCA | L5: `02_svm_intro` `03_svm_nonlinear` `04_pca` `07_em` `08_boosting` `09_logistic_reg` `10_svmsgd` | `ml/` | — | `pca.cpp` `svm.cpp` `train_HOG.cpp` |
| ANN / 决策树 | L5: `05_em` `06_ann_mlp` `07_decision_tree` | `ml/` | — | `letter_recog.cpp` |
| GAPI / GPU | L5: `11_gapi_blur_canny` `12_gpu_basic` `15_gapi_pipeline` | — | — | `gapi_example.cpp` `gpu_example.cpp`(tutorial_code/gapi) |

---

## 10. 拼接 / 标定（综合）

| 主题 | learn | notes | algorithms | 官方 demo |
|------|-------|-------|-----------|-----------|
| 图像拼接 | L4: `15_stitching` | `image_stitching/` | — | `stitching.cpp` `stitching_detailed.cpp` |
| 传统照片处理 | L4: `09_decolor` `10_npr` | `photo_editing/` | `night_scene` `beauty` | `decolor.cpp` `seamless_cloning.cpp` `npr_demo.cpp` |

---

## 用法建议

- **入门某个主题**：先看 `learn/<Lx>/NN_*.cpp`（最小可运行）→ 再翻 `notes/<主题>/`（系统讲解）→ 要跑工业级对比看 `algorithms/<module>/`。
- **调 API 参数**：`docs/cheatsheet.md` 一页速查；`docs/ch0x_*.md` 深读。
- **排障**：`docs/faq_troubleshooting.md` 25+ 条。
- **想看官方怎么写**：按上表"官方 demo"列的文件名在 `mingw-build/samples/cpp/` 里直接搜。
- **原理深挖**：`docs/principles.md` + `docs/ch01_core.md` ~ `ch08_gui_gapi_gpu.md`。
