# HDR 成像算法（imageAlgo）

本节对应官方示例 [hdr_imaging.cpp](../../../mingw-build/samples/cpp/tutorial_code/photo/hdr_imaging/hdr_imaging.cpp)，讲解如何从多张不同曝光的照片合成 HDR 图像。

## 1. 背景原理

如今，大多数数字图像和成像设备每通道使用 8 位，从而将设备的动态范围限制在两个数量级（实际上是 256 个级别），而人眼可以适应相差十个数量级的照明条件。当我们拍摄现实世界场景的照片时，明亮区域可能会曝光过度，而黑暗区域可能会曝光不足，因此我们无法使用单次曝光捕捉所有细节。HDR 成像适用于每通道使用超过 8 位（通常为 32 位浮点值）的图像，从而允许更宽的动态范围。

获取 HDR 图像的方法有很多种，但最常见的一种是使用不同曝光值拍摄的场景照片。要结合这些曝光，了解相机的响应函数很有用，并且有一些算法可以估计它。HDR 图像混合后，必须将其转换回 8 位才能在普通显示器上查看。此过程称为色调映射。当场景或相机的物体在镜头之间移动时，会出现额外的复杂性，因为应注册和对齐具有不同曝光的图像。

在本教程中，我们将展示如何从曝光序列生成和显示 HDR 图像。在我们的例子中，图像已经对齐并且没有移动的物体。我们还演示了一种称为曝光融合的替代方法，可以产生低动态范围图像。HDR 管线的每个步骤都可以使用不同的算法来实现，因此请查看参考手册以了解所有算法。

## 2. 核心功能

输入：一组不同曝光时间的图像（JPEG/PNG 等）及对应的曝光时间列表

输出：

- 合成的 HDR 图像（`.hdr` 格式）
- 色调映射后的 LDR 图像（`.png`）
- 多曝光融合图像（`.png`）

## 3. 代码流程

### 3.1 加载图像序列 (loadExposureSeq)

```cpp
void loadExposureSeq(String path, vector<Mat>& images, vector<float>& times) {
    ifstream list_file((path + "list.txt").c_str()); // 读取 list.txt
    string name; float val;
    while(list_file >> name >> val) {
        Mat img = imread(path + name);  // 读取图像
        images.push_back(img);
        times.push_back(1 / val);       // 存储曝光时间的倒数
    }
}
```

输入目录结构：

```text
input_folder/
    img1.jpg  [曝光时间=1/30秒]
    img2.jpg  [曝光时间=1/15秒]
    ...
    list.txt  [内容示例：img1.jpg 30 \n img2.jpg 15]
```

### 3.2 相机响应校准

```cpp
Mat response;
Ptr<CalibrateDebevec> calibrate = createCalibrateDebevec();
calibrate->process(images, response, times);  // 计算相机响应曲线
```

使用 Debevec 算法从不同曝光的图像中逆向求解相机的响应函数：

- 输出 `response` 是 256×1×3 的矩阵（对应 BGR 通道的响应曲线）

**Debevec 原理**：设传感器辐照度为 $E$，曝光时间为 $\Delta t$，像素观测值 $Z$ 满足：

$$
g(Z) = \ln E + \ln \Delta t
$$

其中 $g = \ln f^{-1}$ 是响应函数 $f$ 的对数逆。对所有像素、所有曝光建立超定线性方程组，加入平滑约束后以最小二乘求解 $g$ 的 256 个离散值。

### 3.3 HDR 合成

```cpp
Mat hdr;
Ptr<MergeDebevec> merge_debevec = createMergeDebevec();
merge_debevec->process(images, hdr, times, response);  // 合成 HDR 图像
```

利用上一步的响应曲线，将不同曝光的图像融合成 32 位浮点 HDR 图像：

- 像素值范围超过 0-255，包含真实场景的亮度信息

**加权融合公式**：每个像素的辐照度按三角权重函数加权平均：

$$
\ln E(x) = \frac{\sum_{i} w(Z_i(x)) \left[ g(Z_i(x)) - \ln \Delta t_i \right]}{\sum_{i} w(Z_i(x))}
$$

权重 $w$ 在中等灰度处最大、在过曝/欠曝处趋近 0，保证融合结果可靠。

### 3.4 色调映射 (Tone Mapping)

```cpp
Mat ldr;
Ptr<TonemapDrago> tonemap = createTonemapDrago(2.2f);
tonemap->process(hdr, ldr);  // 将 HDR 压缩到 LDR
```

Drago 算法将 HDR 的高动态范围压缩到显示器能显示的 0-255 范围：

- 参数 `2.2f` 是 gamma 值，控制对比度

**Drago 对数映射**核心公式（自适应对数压缩）：

$$
L_d = \frac{\ln(1 + L_w)}{\ln(1 + L_{max})} \cdot
\frac{\ln\left(\frac{b}{L_a}\right)}{\ln b}
$$

通过偏置参数 $b$ 在保持对比度与保留高光细节之间折中。

### 3.5 曝光融合 (Exposure Fusion)

```cpp
Mat fusion;
Ptr<MergeMertens> merge_mertens = createMergeMertens();
merge_mertens->process(images, fusion);  // 直接融合多曝光图像
```

Mertens 算法跳过 HDR 生成，直接融合多张不同曝光的图像：

- 优点：速度快，避免 HDR 计算的中间过程

**Mertens 原理**：为每张图计算三种质量图——对比度 $C$、饱和度 $S$、曝光良好度 $W$：

$$
W_{i}(x) = C_i(x)^{\omega_C} \cdot S_i(x)^{\omega_S} \cdot E_i(x)^{\omega_E}
$$

再经拉普拉斯金字塔多频带融合，避免拼接缝。

### 3.6 保存结果

```cpp
imwrite("fusion.png", fusion * 255);  // 融合结果需乘 255
imwrite("ldr.png", ldr * 255);        // 色调映射结果需乘 255
imwrite("hdr.hdr", hdr);              // 直接保存浮点 HDR
```

- `fusion` 和 `ldr` 是 0-1 范围的浮点图像，保存 PNG 前需乘以 255
- HDR 图像用 Radiance 格式（`.hdr`）保存浮点数据

## 4. 关键算法对比

| 步骤 | 算法 | 作用 | 输出特点 |
|------|------|------|----------|
| 校准 | Debevec | 求解相机响应曲线 | 256×1×3 矩阵 |
| HDR 合成 | Debevec | 合并多曝光图像 | 32 位浮点图像 |
| 色调映射 | Drago | HDR→LDR 转换 | 对比度优化的 8 位图像 |
| 融合 | Mertens | 多曝光直接融合 | 细节丰富的 8 位图像 |

## 5. 运行方式

```bash
./program --input=image_folder
```

其中 `image_folder` 包含：

- 多张曝光不同的图像（如 `img1.jpg`, `img2.jpg`...）
- `list.txt` 文件（每行格式：`文件名 曝光时间分母`）

## 6. 典型应用场景

- 逆光环境下的风景摄影
- 室内外混合光照场景
- 高对比度场景（如霓虹灯夜景）
- 需要保留阴影和高光细节的工业检测

> 注意：HDR 图像（`.hdr`）需用支持 HDR 的软件查看（如 Photoshop），而 `ldr.png` 和 `fusion.png` 可直接用普通图片查看器打开。

## 7. 相关官方示例

- [hdr_imaging.cpp](../../../mingw-build/samples/cpp/tutorial_code/photo/hdr_imaging/hdr_imaging.cpp)：HDR 完整流水线官方示例
- [decolor.cpp](../../../mingw-build/samples/cpp/tutorial_code/photo/decolorization/decolor.cpp)：彩色转灰度（保留对比度）
- [npr_demo.cpp](../../../mingw-build/samples/cpp/tutorial_code/photo/non_photorealistic_rendering/npr_demo.cpp)：非真实感渲染（卡通化等）
