# L1 Mat 与像素

**目标**：吃透 `cv::Mat` 的引用计数、深浅拷贝、ROI、像素遍历，并掌握 core 的高频能力（LUT、掩膜卷积、`addWeighted`、`FileStorage`、DFT、ECC、JPEG 编解码）。

**建议顺序**：01 → 02 → 03 → 04 → 05 → 08 → 06 → 07 → 09 → 10 →（选做 11/12）

**验收**：能解释「改 ROI 为什么原图也变了」；能用三种方式遍历像素并比较性能；能写出 DFT 幅度谱与 ECC 配准骨架。

## 核心概念速览

| 主题 | 关键词 | 心智模型 |
| --- | --- | --- |
| Mat 容器 | 矩阵头/数据区/引用计数 | 多个头共享一块数据，clone 才独立 |
| 类型系统 | CV_8UC3/Mat_\<T\> | 深度+通道数+符号编码 |
| 像素访问 | at/ptr/iterator/LUT | 从慢到快的四种遍历 |
| 深浅拷贝 | clone/copyTo/ROI=() | ROI 与赋值是浅拷贝，写 ROI 污染原图 |
| LUT | 查表/SIMD | 预计算 256 映射，避免逐像素分支 |
| 卷积 | filter2D/掩膜 | 邻域加权求和，可分离核更快 |
| 线性混合 | addWeighted/convertTo | $dst=\alpha src_1+\beta src_2+\gamma$ |
| 持久化 | FileStorage/XML/YAML | 序列化 Mat/参数到磁盘 |
| DFT | dft/频谱/象限交换 | 二维可分离，幅度谱取 log 后可视化 |
| 频谱配准 | findTransformECC/warpAffine | 亮度恒定下亚像素对齐 |
| 编解码 | imwrite params/imencode | JPEG 质量/ PNG 压缩级参数控制 |
| 并行 | parallel_for_/Range | 自动分块多线程遍历 |
| SIMD | v_ universal intrinsics | 跨平台向量指令抽象 |

## 官方对照表

| 练习文件 | 官方 sample | docs 锚点 | 必会 API |
| - | - | - | - |
| [01_mat_create_type.cpp](01_mat_create_type.cpp) | `tutorial_code/core/mat_the_basic_image_container/mat_the_basic_image_container.cpp`、`mat_operations.cpp` | ch01 §2.1–2.4 | `Mat(type,size)` / `clone` / `copyTo` / `ROI` / `isContinuous` |
| [02_pixel_scan.cpp](02_pixel_scan.cpp) | `tutorial_code/core/how_to_scan_images/how_to_scan_images.cpp` | ch01 §2.5 | `at` / `ptr` / `iterator` / `LUT` |
| [03_lut_color_reduce.cpp](03_lut_color_reduce.cpp) | `colorReduce.cpp`（根）、`how_to_use_OpenCV_parallel_for_.cpp` | ch01 §LUT | `LUT` / `applyColorMap` |
| [04_mask_convolution.cpp](04_mask_convolution.cpp) | `tutorial_code/core/mat_mask_operations/mat_mask_operations.cpp` | ch01 §2.6 | `filter2D` / 手写卷积 |
| [05_add_weighted.cpp](05_add_weighted.cpp) | `tutorial_code/core/AddingImages/AddingImages.cpp`、`BasicLinearTransforms.cpp` | ch01 §线性混合 | `addWeighted` / `convertScaleAbs` |
| [06_file_storage.cpp](06_file_storage.cpp) | `tutorial_code/core/file_input_output/file_input_output.cpp`、`imagelist_reader.cpp` | ch01 §FileStorage | `FileStorage` 读写 / `<<` / `>>` |
| [07_dft_spectrum.cpp](07_dft_spectrum.cpp) | `tutorial_code/core/discrete_fourier_transform/discrete_fourier_transform.cpp`、`dft.cpp` | ch01 §2.7 | `dft` / `copyMakeBorder` / `magnitude` / `log` |
| [08_create_mask.cpp](08_create_mask.cpp) | `create_mask.cpp`（根） | ch01 §create_mask | `Mat::setTo` / `bitwise_and` / `bitwise_or` |
| [09_ecc_align.cpp](09_ecc_align.cpp) | `image_alignment.cpp`（根） | ch01 §ECC | `findTransformECC` / `warpAffine` |
| [10_jpeg_codec.cpp](10_jpeg_codec.cpp) | `imgcodecs_jpeg.cpp`（根）、`imgcodecs_imwrite.cpp` | ch01 §3 | `imwrite` params / `imdecode` / `imencode` |
| [11_parallel_for.cpp](11_parallel_for.cpp) | `how_to_use_OpenCV_parallel_for_.cpp`、`how_to_use_OpenCV_parallel_for_new.cpp` | ch01 §2.8 | `parallel_for_` / `Range`（选做） |
| [12_simd_basic.cpp](12_simd_basic.cpp) | `univ_intrin.cpp`、`simd_basic.cpp`（根） | ch01 §2.9 | `v_` universal intrinsics（选做） |

## 关键易错点

| 练习 | 易错点 / 关键参数 |
| --- | --- |
| 01 Mat | `Mat b = a` 是浅拷贝；`row()/col()` 返回引用，写它会动原图，需 `clone()` |
| 02 遍历 | `at<>(y,x)` 先行后列；非连续 Mat 当一维扫会越界 |
| 03 LUT | `LUT` 只支持 `CV_8U`；表大小必须 256 |
| 04 卷积 | `ddepth=-1` 同输入；负结果会被截断，梯度图应用 `CV_16S` |
| 05 加权 | 两图尺寸/类型必须一致；结果用 `saturate_cast` 截断 |
| 06 FileStorage | 写用 `<<`、读用 `>>`；`FileStorage::READ` 必须匹配节点名 |
| 07 DFT | 输入需 `copyMakeBorder` 补零到最优尺寸；幅度谱要 `log(1+|F|)` |
| 08 掩膜 | `setTo` 带 mask 只改非零处；`bitwise_and` 做区域裁剪 |
| 09 ECC | 迭代次数/终止条件；`findTransformECC` 对齐前需灰度+归一化 |
| 10 编码 | `IMWRITE_JPEG_QUALITY` 0~100；`IMWRITE_PNG_COMPRESSION` 0~9 |

## 说明

- `03_lut_color_reduce`：用查表把 256 级压成 32 级，对比手写循环与 `LUT` 的性能。
- `07_dft_spectrum`：输出灰度幅度谱图，理解四象限交换。
- `09_ecc_align`：无配对图时用同一张图 + 人工仿射变换做对齐验证，避免缺数据。
- 选做项（11/12）允许只跑通最小示例，不深入性能专题，避免 L1 拖成性能课。
