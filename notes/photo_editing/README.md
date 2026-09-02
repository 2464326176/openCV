# 图像编辑与照片特效（photoEditing）

本节讲解 `opencv_photo` 模块：**无缝克隆**（Poisson 图像编辑）、**保边平滑**、**细节增强**、**素描**、**风格化**、**高质量去色**。这类技术在修图软件、美颜、影视后期中广泛使用。对应官方示例 [cloning_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/photo/seamless_cloning/cloning_demo.cpp)、[decolor.cpp](../../mingw-build/samples/cpp/tutorial_code/photo/decolorization/decolor.cpp)、[npr_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/photo/non_photorealistic_rendering/npr_demo.cpp)。

本目录源码：[seamless_clone.cpp](seamless_clone.cpp)、[photo_effects.cpp](photo_effects.cpp)。

## 1. 章节文件索引

| 文件 | 主题 |
|------|------|
| [seamless_clone.cpp](seamless_clone.cpp) | 无缝克隆：普通/混合/单色迁移 |
| [photo_effects.cpp](photo_effects.cpp) | 去色、保边平滑、细节增强、素描、风格化 |

## 2. 无缝克隆 seamlessClone

### 2.1 原理

普通 `copyTo` 拼接会在边界留下明显接缝。**Poisson 图像编辑**不直接拷贝像素值，而是拷贝目标区域的**梯度场**（局部相对变化），再在边界约束下解 Poisson 方程恢复像素——目标区域保持源图的纹理细节，边界与背景无缝过渡。

```cpp
seamlessClone(source, target, mask, p, result, flag);
```

| 参数 | 含义 |
|------|------|
| `source` | 要粘贴的源图 |
| `target` | 背景目标图 |
| `mask` | 源图中的粘贴区域（非零=有效） |
| `p` | 源图中心粘贴到目标图的锚点 |
| `flag` | 1=普通克隆 2=混合克隆 3=单色迁移 |

- **普通克隆**（NORMAL_CLONE）：保留源图完整纹理。
- **混合克隆**（MIXED_CLONE）：对源图与目标图梯度取**较大者**，适合透明物体、文字贴图。
- **单色迁移**（MONOCHROME_TRANSFER）：只迁移源图纹理而不迁颜色。

## 3. 非真实感渲染与去色

代码来自 [photo_effects.cpp](photo_effects.cpp)：

```cpp
decolor(src, gray, color_boost);            // ① 高质量去色：灰度 + 彩色增强图
edgePreservingFilter(src, img, 1);          // ② 保边平滑(1=归一化卷积, 2=递归滤波)
detailEnhance(src, img);                    // ③ 细节增强
pencilSketch(src, sketch, color_sketch, 10, 0.1f, 0.03f);  // ④ 素描/彩色素描
stylization(src, img);                      // ⑤ 风格化(卡通)
```

### 3.1 decolor 与普通 cvtColor 的区别

普通 `cvtColor(src, gray, COLOR_BGR2GRAY)` 是固定加权：`gray = 0.299R + 0.587G + 0.114B`，会损失色差相近的细节。`decolor` 通过**优化灰度映射**保留原始局部对比度，同时额外输出一张**色彩增强图**（boost），让本应醒目的颜色更突出。

### 3.2 保边平滑 edgePreservingFilter

与高斯模糊不同，保边滤波在平滑的同时保留强边缘，用于磨皮、降噪、美颜。实现分两种：
- **归一化卷积**（RECURS_FILTER=1）：迭代加权平均。
- **递归滤波**（NORMCONV_FILTER=2）：多尺度递归，更平滑。

### 3.3 参数

| 函数 | 关键参数 | 说明 |
|------|----------|------|
| `edgePreservingFilter` | `sigma_s`/`sigma_r` | 空间/颜色方差，越大越模糊 |
| `pencilSketch` | `sigma_s, sigma_r, shade_factor` | `shade_factor` 越小线条越深 |
| `detailEnhance` | `sigma_s, sigma_r` | 增强局部对比度 |

## 4. 应用场景

- **美颜 App**：`edgePreservingFilter` 磨皮 + `detailEnhance` 提亮五官。
- **摄影后期**：`decolor` 做高级黑白，`seamlessClone` 换天空/加元素。
- **UI/素材**：文字、Logo 无缝贴图（MIXED_CLONE）。
- **风格迁移预览**：`stylization` 快速生成卡通/油画效果。

## 5. 相关官方示例

- [cloning_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/photo/seamless_cloning/cloning_demo.cpp)：6 种克隆/编辑模式
- [decolor.cpp](../../mingw-build/samples/cpp/tutorial_code/photo/decolorization/decolor.cpp)：去色
- [npr_demo.cpp](../../mingw-build/samples/cpp/tutorial_code/photo/non_photorealistic_rendering/npr_demo.cpp)：非真实感渲染
- [hdr_imaging.cpp](../../mingw-build/samples/cpp/tutorial_code/photo/hdr_imaging/hdr_imaging.cpp)：HDR 合成
