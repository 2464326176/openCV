# 直方图与模板匹配（histogramsMatch）

本节覆盖**直方图计算、直方图对比、直方图均衡化、H-S 二维直方图、模板匹配**。对应官方示例 [calcHist\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/Histograms_Matching/calcHist_Demo.cpp)、[compareHist\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/Histograms_Matching/compareHist_Demo.cpp)、[EqualizeHist\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/Histograms_Matching/EqualizeHist_Demo.cpp)、[MatchTemplate\_Demo.cpp](../../mingw-build/samples/cpp/tutorial_code/Histograms_Matching/MatchTemplate_Demo.cpp)。

## 1. 章节文件索引

| 文件                                        | 主题                 |
| ----------------------------------------- | ------------------ |
| [calc\_hist.cpp](calc_hist.cpp)           | 计算并绘制 BGR 三通道直方图   |
| [compare\_hist.cpp](compare_hist.cpp)     | H-S 二维直方图 + 四种对比方法 |
| [equalize\_hist.cpp](equalize_hist.cpp)   | 直方图均衡化             |
| [hs\_hist.cpp](hs_hist.cpp)               | H-S 二维直方图可视化       |
| [match\_template.cpp](match_template.cpp) | 六种模板匹配方法交互演示       |

## 2. 直方图计算 calcHist

来自 [calc\_hist.cpp](calc_hist.cpp)：

```cpp
vector<Mat> bgr_planes;
split(srcImage, bgr_planes);          // 分离三通道

int histSize = 256;                   // 256 个 bin
float range[] = {0, 256};             // 灰度范围 [0, 256)
const float *histRange = {range};

Mat b_hist, g_hist, r_hist;
calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
```

`calcHist` 参数解析：

| 参数                   | 含义                       |
| -------------------- | ------------------------ |
| `images` / `nimages` | 输入图像数组及数量                |
| `channels`           | 参与统计的通道编号                |
| `mask`               | 掩膜，只统计非零区域（`Mat()` 表示全图） |
| `dims`               | 直方图维度（1=灰度，2=H-S 联合）     |
| `histSize`           | 每个维度的 bin 数              |
| `ranges`             | 每个维度的取值范围                |
| `uniform`            | 是否等宽分组                   |
| `accumulate`         | 是否累加（多次统计用）              |

### 2.1 直方图可视化

```cpp
normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());  // 归一到画布高度
for (int i = 1; i < histSize; ++i) {
    line(histImage,
         Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
         Point(bin_w * (i),     hist_h - cvRound(b_hist.at<float>(i))),
         Scalar(255, 0, 0), 2, 8, 0);
}
```

用折线连接相邻 bin 的高度。三通道分别用 `b_hist`/`g_hist`/`r_hist` 对应蓝/绿/红折线。

## 3. H-S 二维直方图

来自 [hs\_hist.cpp](hs_hist.cpp)——在 HSV 空间统计 Hue×Saturation 联合分布。

```cpp
cvtColor(src, dst, COLOR_BGR2HSV);

int hueBinNum = 30, saturationBinNum = 32;
int histSize[] = {hueBinNum, saturationBinNum};

float hueRanges[] = {0, 180};          // OpenCV 中 H 范围为 0-179
float saturationRanges[] = {0, 256};
const float *ranges[] = {hueRanges, saturationRanges};

int channels[] = {0, 1};               // H 与 S 通道
MatND dstHist;
calcHist(&dst, 1, channels, Mat(), dstHist, 2, histSize, ranges, true, false);
```

二维直方图可视化为热力图（灰度越亮表示该 H-S 组合像素越多）：

```cpp
double maxValue = 0;
minMaxLoc(dstHist, 0, &maxValue, 0, 0);
int scale = 10;   // 每个 bin 放大 10 倍显示
for (int hue = 0; hue < hueBinNum; hue++) {
    for (int saturation = 0; saturation < saturationBinNum; saturation++) {
        float binValue = dstHist.at<float>(hue, saturation);
        int intensity = cvRound(binValue * 255 / maxValue);
        rectangle(histImg, Point(hue * scale, saturation * scale),
                  Point((hue + 1) * scale - 1, (saturation + 1) * scale - 1),
                  Scalar::all(intensity), FILLED);
    }
}
```

**为什么用 HSV**：H（色调）对光照变化不敏感，适合颜色目标跟踪（配合 `calcBackProject` 反向投影）。

## 4. 直方图对比 compareHist

来自 [compare\_hist.cpp](compare_hist.cpp)——同一场景不同图像做相似度对比。

```cpp
cvtColor(src_base, hsv_base, COLOR_BGR2HSV);

int h_bins = 50, s_bins = 60;
int histSize[] = { h_bins, s_bins };
int channels[] = { 0, 1 };

calcHist(&hsv_base, 1, channels, Mat(), hist_base, 2, histSize, ranges, true, false);
normalize(hist_base, hist_base, 0, 1, NORM_MINMAX, -1, Mat());   // 归一化到 [0,1]

for (int compare_method = 0; compare_method < 4; compare_method++) {
    double base_base  = compareHist(hist_base, hist_base,  compare_method);
    double base_half  = compareHist(hist_base, hist_half_down, compare_method);
    double base_test1 = compareHist(hist_base, hist_test1, compare_method);
    double base_test2 = compareHist(hist_base, hist_test2, compare_method);
}
```

四种对比方法（$H\_1, H\_2$ 为两个归一化直方图，$N$ 为 bin 数）：

| 方法                         | 公式                                                                                                   | 判据          |
| -------------------------- | ---------------------------------------------------------------------------------------------------- | ----------- |
| `HISTCMP_CORREL`(0)        | $\frac{\sum (H\_1-\bar H\_1)(H\_2-\bar H\_2)}{\sqrt{\sum(H\_1-\bar H\_1)^2 \sum(H\_2-\bar H\_2)^2}}$ | 越接近 1 越相似   |
| `HISTCMP_CHISQR`(1)        | $\sum \frac{(H\_1-H\_2)^2}{H\_1+H\_2}$                                                               | 越小越相似       |
| `HISTCMP_INTERSECT`(2)     | $\sum \min(H\_1, H\_2)$                                                                              | 越大越相似       |
| `HISTCMP_BHATTACHARYYA`(3) | $\sqrt{1 - \frac{1}{\bar H\_1 \bar H\_2 N^2}\sum \sqrt{H\_1 H\_2}}$                                  | 越小越相似（巴氏距离） |

## 5. 直方图均衡化

来自 [equalize\_hist.cpp](equalize_hist.cpp)：

```cpp
cvtColor(srcIMage, srcIMage, COLOR_BGR2GRAY);
equalizeHist(srcIMage, dstImage);
```

原理见 [image\_transformation/README.md](../image_transformation/README.md) 第 8 节：用 CDF 映射拉平直方图，增强整体对比度。仅适用于单通道 8 位图。

## 6. 模板匹配 matchTemplate

来自 [match\_template.cpp](match_template.cpp)——滑条切换六种匹配方法：

```cpp
// 结果矩阵尺寸 = 可滑动的窗口位置数
int result_cols = img.cols - templ.cols + 1;
int result_rows = img.rows - templ.rows + 1;
result.create(result_rows, result_cols, CV_32FC1);

matchTemplate(img, templ, result, match_method);   // 滑动窗口逐位置打分
normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

// 定位最佳匹配
double minVal, maxVal; Point minLoc, maxLoc, matchLoc;
minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

// SQDIFF 系列：值越小越匹配；其余方法：值越大越匹配
if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
    matchLoc = minLoc;
else
    matchLoc = maxLoc;

rectangle(img_display, matchLoc,
          Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2);
```

六种方法（$T$ 模板，$I$ 图像，$R$ 结果）：

| 方法                               | 特性                     |
| -------------------------------- | ---------------------- |
| `TM_SQDIFF` / `TM_SQDIFF_NORMED` | 平方差匹配，**最小值**为最佳       |
| `TM_CCORR` / `TM_CCORR_NORMED`   | 相关性匹配，最大值最佳；NORMED 更鲁棒 |
| `TM_CCOEFF` / `TM_CCOEFF_NORMED` | 相关系数匹配，最大值最佳；对亮度偏移鲁棒   |

**掩膜支持**：仅 `TM_SQDIFF` 和 `TM_CCORR_NORMED` 接受掩膜参数。

## 7. 典型应用场景

- **图像检索**：直方图对比实现"以图搜图"的粗筛。

- **颜色目标跟踪**：H-S 直方图 + `calcBackProject` 反向投影是 CamShift 的基础（见 video 章节）。

- **模板定位**：固定相机下定位固定外观的零件/标志，配合金字塔加速。

- **曝光校正**：均衡化改善过暗/过亮图像的可视性。

## 8. 相关官方示例

- [calcBackProject\_Demo1.cpp](../../mingw-build/samples/cpp/tutorial_code/Histograms_Matching/calcBackProject_Demo1.cpp)：直方图反向投影

- [calcBackProject\_Demo2.cpp](../../mingw-build/samples/cpp/tutorial_code/Histograms_Matching/calcBackProject_Demo2.cpp)：反向投影分析

- [demhist.cpp](../../mingw-build/samples/cpp/demhist.cpp)：直方图均衡交互演示

- [mask\_tmpl.cpp](../../mingw-build/samples/cpp/mask_tmpl.cpp)：带掩膜的模板匹配

