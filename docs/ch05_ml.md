# 第 5 章 OpenCV `ml` 模块：原理与实战

> 本章基于 OpenCV C++ 官方示例源码（`samples/cpp` 根目录 11 个文件 + `tutorial_code/ml/` 3 个文件，共 14 个）重写并大幅扩写。目标是把"能跑通的示例"还原为"可迁移的原理"——对每个条目给出数学表达、关键 API 参数表、算法关联对比、常见错误与落地场景。正文为简体中文，API 与代码标识符保留英文。
>
> 约定：文件路径以 `samples/cpp/...` 相对前缀给出；公式用 LaTeX 行内（`$...$`）或伪代码块；所有示例均"先读源码再扩写"，未改动任何源文件。源码根为仓库内 `mingw-build/samples/cpp`。

---

## 5.0 章节导言

OpenCV 的 `ml` 模块定位非常清晰——它是**一组"经典机器学习算法"的纯消费者（consumer）**，自身**不负责图像特征的提取**。从所有示例的流程即可看出：

| 阶段 | 负责模块 | 典型 API |
| --- | --- | --- |
| 图像→像素特征/轮廓 | `imgproc`、`imgcodecs`、`video` | `Sobel`、`cartToPolar`、`findContours`、`connectedComponentsWithStats` |
| 像素→数值特征向量 | 用户代码 / `imgproc` | `preprocess_hog`、`formatImagesForPCA` |
| 特征向量→模型 | `ml` | `SVM::train`、`KNearest::train`、`PCA`、`EM::trainEM` |
| 模型→推断/可视化 | `ml` + `imgproc`/`highgui` | `predict`、`project`、`drawContours` |

典型证据：

- [digits_svm.cpp](#521-digit_svmcpp--hog--svm端到端数字识别) 的完整流水线：*`Sobel`+`cartToPolar` 产生 HOG 特征 → 才交给 `SVM`/`KNearest` 消费*；`ml` 模块从未接触过"图像"，只看到 `Mat samples` 这种 `CV_32F` 数值矩阵。
- [pca.cpp](#571-pcacpp--pca-人脸降维与保留方差-trackbar) 的 `formatImagesForPCA()` 先把图像 `reshape(1,1)` 摊平为行向量，再交给 `PCA`；`ml` 不知道那是人脸。
- [train_HOG.cpp](#522-train_hogcpp--自训-hog--svm-行人检测器) 的 `computeHOGs()` 先用 `HOGDescriptor` 计算直方图，再用 `convert_to_ml()` 转成 `CV_32FC1` 矩阵，最后才交给 `SVM`。

**因此，学习 `ml` 模块时务必破除幻觉：你要先学会用 `imgproc` 把图像变成"特征矩阵"，`ml` 才有意义。** 这也是为什么本系列教程把"特征工程"单独放在 [第 3 章局部特征](./ch03_features.md) 与 [第 2 章图像处理](./ch02_imgproc.md)。

### 5.0.1 `StatModel` 统一接口与 `TrainData` 设计哲学

所有 `ml` 模型都继承自基类 `cv::ml::StatModel`，因此它提供了一套*统一的训练/预测/保存/加载接口*：

```cpp
class StatModel {
public:
    bool train( const Ptr<TrainData>& trainData, int flags=0 );     // 通过 TrainData
    bool train( InputArray samples, int layout, InputArray responses ); // 简化重载
    float predict( InputArray samples, OutputArray results=noArray(), int flags=0 ) const;
    bool isTrained() const;  bool isClassifier() const;
    void save( const String& filename ) const;
    template<typename T> static Ptr<T> StatModel::load( const String& filename );
    float calcError( const Ptr<TrainData>& data, bool test, OutputArray resp ) const;
};
```

`TrainData` 是把"训练样本、标签、变量类型、缺失值、训练/测试切分"统一打包的容器。其设计要点：

- **`samples` 行存储**：`Mat` 每行一个样本（`ROW_SAMPLE`，最常见）或每列一个样本（`COL_SAMPLE`）。`CV_32F` 必备，分类标签可以是 `CV_32S`。
- **`responses`**：回归用 `CV_32F`；分类既可类别号（`CV_32S`，推荐），也可 one-hot。
- **`varType`**：长度 = 变量数 + 1，最后一位固定为响应列类型。`VAR_ORDERED`（数值）/`VAR_CATEGORICAL`（类别）。
- **`setTrainTestSplitRatio(r)`**：内置训练/测试自动切分，配合 `calcError(data, true, ...)` 一键算测试集误差。

> **30 秒心智模型**：`StatModel` 把所有模型抽象为"吃 `Mat` 出 `float` 的黑盒"——把图像、点云、文本特征统统拍平成 `CV_32F` 行矩阵，标签列单独给。模型间差异在 `train` 内部目标函数，不在数据接口。

```mermaid
flowchart LR
    A[原始图像/视频] --> B[imgproc: Sobel/Canny/直方图<br/>HOG/SURF 描述子]
    B --> C[Mat CV_32F<br/>行=样本 列=特征]
    C --> D[TrainData::create]
    D --> E[StatModel::train]
    E --> F[模型文件 .yml/.xml]
    F --> G[StatModel::predict]
    G --> H[类别/回归值/簇号]
    style E fill:#2c7,color:#fff
    style G fill:#2c7,color:#fff
```

**上下文依赖**：本章是 [第 2 章图像处理](./ch02_imgproc.md)（Sobel/Histogram/Hough）与 [第 3 章局部特征](./ch03_features.md)（HOG/SURF/ORB 描述子）的下游消费者；HOG 行人检测、级联分类器在 [第 6 章](./ch06_objdetect_photo.md) 展开；`dnn` 深度学习作为 `ml` 的现代化替代在 `digits_lenet.cpp` 处衔接；原理总纲见 [principles §13.4 传统机器学习](./principles.md#134-传统机器学习)。读不懂本章，上层的检测器训练、检索、姿态识别都会有理解断层。

**本章阅读建议**：按"SVM → KNN/Bayes/LogReg → k-means/EM → 决策树/森林/Boosting → ANN_MLP/dnn 桥梁 → PCA 降维"顺序读，每节按 "功能 → 原理 → 数学 → API 参数 → 对比 → 易错点 → 场景" 展开。重点吃透 **SVM 的最大间隔与软间隔约束、RBF 核的 $\gamma$ 含义、HOG+SVM 的检测器导出技巧、k-means 的 K-means++ 初始化与 compactness 评估、EM 的隐变量后验与协方差类型、决策树的不纯度下降与 RTrees 的变量重要性、ANN_MLP 的反向传播、PCA 的协方差特征分解与方差保留** 八处原理。

**概念阅读顺序**（重点看核心原理与参数说明，不写编译运行）：

- 先懂 `StatModel`/`TrainData` 抽象与行存储约定，再对照任意示例
- 先懂 SVM 的最大间隔与核映射，再对照 `introduction_to_svm.cpp` / `non_linear_svms.cpp`
- 先懂 HOG 特征 + 检测器导出（SV+ρ 拼接），再对照 `digits_svm.cpp` / `train_HOG.cpp`
- 先懂 k-means 与 EM 的"硬分配 vs 软分配"差异，再对照 `kmeans.cpp` / `em.cpp`
- 先懂决策树分裂准则与随机森林的 bagging，再对照 `tree_engine.cpp` / `letter_recog.cpp`
- 先懂 ANN_MLP 的前向/反向传播与激活函数，再对照 `neural_network.cpp`
- 先懂 PCA 的协方差特征分解与"保留方差=信息保留"的对应，再对照 `pca.cpp` / `introduction_to_pca.cpp`

---

## 5.1 SVM 系列：最大间隔分类

支持向量机是 `ml` 模块的核心监督分类器。OpenCV 提供 `SVM`（标准 SMO 求解）与 `SVMSGD`（在线随机梯度）两条线。四个示例从"线性可分入门"逐步推进到"端到端工程"。

### 5.1.1 `introduction_to_svm.cpp` —— 线性 SVM 入门

> **源文件**：`samples/cpp/tutorial_code/ml/introduction_to_svm/introduction_to_svm.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：完整流程

#### 功能概述

构造 4 个二维点（3 个负类 + 1 个正类），训练一个线性核 `C_SVC`，把整张 512×512 图像每个像素送入 `predict` 染色，得到决策区域可视化；再把训练点和**支持向量**画到同一张图上。

#### 核心原理

**30 秒心智模型**：SVM 在样本空间中寻找一个超平面 $w^\top x + b = 0$，让"两类离它最近的点（支持向量）"距离最大——间隔 $\gamma = 2/\lVert w\rVert$。最大化间隔等价于最小化 $\tfrac12\lVert w\rVert^2$，故线性可分 SVM 的原始问题：

$$
\min_{w,b}\ \tfrac12\lVert w\rVert^2 \quad \text{s.t.}\quad y_i(w^\top x_i + b)\ge 1,\ \forall i.
$$

数据线性不可分时引入松弛 $\xi_i\ge 0$ 与惩罚 $C$，得到**软间隔**：

$$
\min_{w,b,\xi}\ \tfrac12\lVert w\rVert^2 + C\sum_i \xi_i \quad \text{s.t.}\quad y_i(w^\top x_i + b)\ge 1-\xi_i,\ \xi_i\ge 0.
$$

$C$ 大→对违反零容忍→易过拟合；$C$ 小→容忍违反→易欠拟合。拉格朗日对偶给出：

$$
\max_\alpha\ \sum_i\alpha_i - \tfrac12\sum_{i,j}\alpha_i\alpha_j y_i y_j \langle x_i,x_j\rangle \quad \text{s.t.}\quad 0\le\alpha_i\le C,\ \sum_i\alpha_i y_i = 0.
$$

只有 $\alpha_i>0$ 的样本是**支持向量**——它们决定超平面，其余样本可以删掉模型不变。对偶形式中的内积 $\langle x_i,x_j\rangle$ 可替换为核函数 $K(x_i,x_j)$，从而把线性 SVM 推广到非线性。

#### 关键 API

- `SVM::create()` → `Ptr<SVM>`；`setType(SVM::C_SVC)` 设置多分类或二分类。
- `setKernel(SVM::LINEAR)`：核类型；可选 `POLY`/`RBF`/`SIGMOID`/`CHI2`/`INTER`。
- `setTermCriteria(TermCriteria(MAX_ITER, 100, 1e-6))`：SMO 迭代终止。
- `train(samples, ROW_SAMPLE, labels)`：训练。
- `predict(sample)`：返回类别号（`float`）。
- `getUncompressedSupportVectors()`：返回支持向量矩阵（线性核下可直接画出来）。

#### 处理流程

1. 构造 4 行 2 列 `trainingData`（`CV_32F`）+ 4 行 1 列 `labels`（`CV_32S`）。
2. 创建 `SVM`，`C_SVC` + `LINEAR` + 终止条件 100 次迭代。
3. `train` 后，外层两层 `for` 遍历像素 `(i,j)`，组装 `Mat_<float>(1,2) << j,i`，调 `predict` 染绿/蓝。
4. 把训练点画实心圆；把 `getUncompressedSupportVectors()` 的每行画大圈，灰边。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `SVM::C_SVC` | 多分类目标 | 默认 | 与 `NU_SVR`/`ONE_CLASS` 互斥，按任务选 |
| `SVM::LINEAR` | 核函数 $K(x,x')=x^\top x'$ | 默认 | 换 `RBF` 可分非线性但需调 $\gamma$ |
| `MAX_ITER=100` | SMO 最大迭代 | 100–1000 | 大→更精但慢；小→可能未收敛 |
| `eps=1e-6` | 收敛阈值 | 1e-6 | 小→精但慢 |

#### 关联与对比

- 与 [non_linear_svms.cpp](#512-non_linear_svmscpp--软间隔与不可分数据) 对比：本例线性可分，故 `C` 不起作用；下例用 `C=0.1` 控制软间隔违反。
- 与 [digits_svm.cpp](#523-digit_svmcpp--hog--svm端到端数字识别) 对比：本例的"可视化决策区域"在 digits 示例中被"图像分类准确率"取代，但内核一致。
- 原理对应 [principles §13.4 SVM](./principles.md#svm)。

#### 注意事项

- `trainingData` 必须 `CV_32F`，`labels` 多分类用 `CV_32S`。源码注释 `[setup2]` 写得很清楚，是工程实践的标准姿势。
- `getSupportVectors()` 在压缩存储时返回压缩向量；要画原坐标用 `getUncompressedSupportVectors()`，见源码 `[show_vectors]`。
- `predict` 返回 `float`，比较相等时不要用 `==1`，而是 `cvRound` 或设定阈值。

#### 应用场景

- 教学示例：理解 SVM 决策区域与支持向量的几何含义。
- 任何**特征数 ≤ 2** 的二分类可视化场景，可直接套用此模板。

---

### 5.1.2 `non_linear_svms.cpp` —— 软间隔与不可分数据

> **源文件**：`samples/cpp/tutorial_code/ml/non_linear_svms/non_linear_svms.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：完整流程

#### 功能概述

随机生成 200 个二维点（两堆高斯分布，但中间一段"重叠"），用 `LINEAR` 核 + `C=0.1` 的软间隔训练，可视化决策边界与支持向量。展示**当数据不可严格线性可分时，SVM 通过 $C$ 权衡间隔宽与违反量**。

#### 核心原理

**30 秒心智模型**：本例的 90% 点线性可分，10% 点故意散在中间形成不可分区域。`C` 小→允许少量点"穿过"间隔甚至越界→得到更宽、更平滑的间隔；`C` 大→几乎不许穿过→边界复杂、易过拟合。

软间隔对偶约束变为 $0\le\alpha_i\le C$：$\alpha_i=C$ 的样本**违反间隔**（在间隔内或在错误一侧），它们仍是支持向量但带惩罚。`C` 直接对应"对违反的容忍度"：

$$
\xi_i = \max(0,\, 1 - y_i(w^\top x_i + b)), \quad \text{hinge loss}.
$$

故 SVM 的目标等价于最小化 $\tfrac12\lVert w\rVert^2 + C\sum_i \max(0, 1-y_i f(x_i))$——这是"正则项 + 合页损失"的标准形式。

#### 关键 API

- `setC(0.1)`：软间隔惩罚；本例关键参数。
- `rng.fill(c, RNG::UNIFORM, Scalar(0), Scalar(0.4*WIDTH))`：批量生成均匀分布样本；`RNG::NORMAL` 用于高斯。
- `trainData.rowRange(...)` / `colRange(...)`：切片填充训练数据，源码 `[setup1]`/`[setup2]` 的标准用法。

#### 处理流程

1. 构造 `trainData(2N, 2, CV_32F)` + `labels(2N, 1, CV_32S)`。
2. 前 90 个点为类别 1，x∈[0, 0.4W]；后 90 个点为类别 2，x∈[0.6W, W]；中间 20 个点两类各占 10 个，x∈[0.4W, 0.6W] 随机分布——形成不可分区域。
3. `SVM::create()` + `C_SVC` + `LINEAR` + `C=0.1` + `MAX_ITER=1e7`。
4. `train` 后整图染色 + 训练点彩色圆 + 支持向量大灰圈。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `C` | 软间隔惩罚 | 0.1–100 | 大→严苛、边界复杂、过拟合；小→宽容、边界平、欠拟合 |
| `FRAC_LINEAR_SEP=0.9` | 线性可分比例 | 0.5–1.0 | 大→接近线性可分；小→越难分 |
| `MAX_ITER=1e7` | SMO 上限 | 1e5–1e7 | 大→更精但慢 |

#### 关联与对比

- 与 [introduction_to_svm.cpp](#511-introduction_to_svmcpp--线性-svm-入门) 对比：本例刻意制造"不可分"，凸显 `C` 的作用；上例完全可分故 `C` 无效。
- 想真正解非线性，把 `LINEAR` 改 `RBF` 并设 `gamma`——这是工程默认，下两例即用。

#### 注意事项

- $C$ 的"大小"无绝对量纲，与样本规模、特征尺度相关。**务必先标准化特征**再调 $C$。
- 中间一段重叠会让模型在 0.5W 附近边界剧烈摆动，可视化的彩色区域颜色会"混色"——这正是 `C` 太小的体现。
- 若换成 `RBF` 核不调 $\gamma$，默认 $\gamma = 1/\text{numFeatures}$，往往太小或太大，需要 `setGamma()` 手动设。

#### 应用场景

- 教学示例：理解软间隔与硬间隔的差异。
- 任何有噪声/重叠的二分类，验证 $C$ 调参方向。

---

### 5.1.3 `train_svmsgd.cpp` —— 在线 SGD-SVM 交互训练

> **源文件**：`samples/cpp/train_svmsgd.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：完整流程

#### 功能概述

交互式 GUI：左键点加正例（黄），右键点加负例（青），每加一个点立即调 `SVMSGD::train` 重训一次，绘制当前线性决策边界 $w^\top x + b = 0$。展示 SGD 求解 SVM 的在线学习能力。

#### 核心原理

**30 秒心智模型**：经典 SVM 用 SMO 算法一次性批处理求解 $\alpha_i$，新增样本须整体重训；`SVMSGD` 用随机梯度下降直接优化原始问题（hinge loss + L1/L2 正则），单步更新即可：

$$
w_{t+1} = w_t - \eta_t\, \nabla \big(\lambda r(w_t) + \ell(w_t; x_t, y_t)\big),\quad \ell = \max(0, 1 - y_t w_t^\top x_t).
$$

这正是 Pegasos 算法的雏形。每加一个点 → 增量训练 → 立刻给出新边界，无需重算 $\alpha$。`getWeights()` 返回 $w$（2 维），`getShift()` 返回 $b$，画线 $w_0 x + w_1 y + b = 0$ 与图像边界求交。

#### 关键 API

- `SVMSGD::create()` + `train(TrainData)`：在线训练。
- `getWeights()` / `getShift()`：取出 $w$ 与 $b$。
- `findCrossPointWithBorders`：把直线与图像四边求交，画分界线。

#### 处理流程

1. 初始化空白图 841×594。
2. 鼠标回调：左键 `(x,y,1)`，右键 `(x,y,-1)`。
3. `addPointRetrainAndRedraw`：push 到 `samples`/`responses`，调 `doTrain` 重训，求 $w$ 与 $b$。
4. `findPointsForLine`：与四条边界求交两点。
5. `redraw`：画所有训练点 + 决策线。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `SVMSGD::SGD`/`ASGD` | 梯度类型 | 默认 ASGD | ASGD 平均更稳 |
| `lambda=0.0001` | 正则系数 | 1e-5–1e-2 | 大→强正则、边界平 |
| `Weights` | 决策法向量 | 训练得来 | 可视化时画法线 |

#### 关联与对比

- 与 [introduction_to_svm.cpp](#511-introduction_to_svmcpp--线性-svm-入门) 对比：本例**每次加点立即更新**，`SVM` 则是批量重训。SGD 在数据流场景占优。
- 与 [points_classifier.cpp（在 ch04）](./ch04_video.md) 对比：后者用 SVM/KNN/RTrees/Boost/ANN/EM/NBayes 多模型对比，本例只演示 SVMSGD 一种。

#### 注意事项

- `SVMSGD` 默认只支持二分类线性；要 RBF 或多分类须回到标准 `SVM`。
- 样本数极少时边界会剧烈跳动，符合 SGD 在小样本下的预期。
- 求直线与边界交点时若 `weights[1]≈0`（接近水平线）要避免除零——源码用 `if (weights[1] != 0)` 守护。

#### 应用场景

- 教学示例：直观感受 SVM 的几何与在线学习。
- 流式数据场景下的二分类雏形（如点击日志、传感器流）。

---

## 5.2 SVM 工程化应用：从分类到检测

`digits_svm.cpp` 与 `train_HOG.cpp` 是把 SVM 用于真实视觉任务的两个示例。它们共同的关键技巧是**"把训练好的线性 SVM 导出成 `HOGDescriptor::detect` 可用的检测器向量"**——线性 SVM 的权重向量恰好就是 HOG 检测器的线性滤波器。

### 5.2.1 `digits_svm.cpp` —— HOG + SVM 端到端数字识别

> **源文件**：`samples/cpp/digits_svm.cpp` ｜ **所属模块**：`ml` + `imgproc` ｜ **示例类型**：完整流程

#### 功能概述

从 `digits.png` 切出 5000 个 20×20 手写数字（0–9 各 500 个），对每个数字做：**几何去斜（deskew）→ 4×4 cell HOG 描述子 → Hellinger 核归一化**。然后训练 `KNearest` 与 `SVM(RBF)`，输出错误率、混淆矩阵与错分样例拼图。

#### 核心原理

**30 秒心智模型**：直接用像素值做分类效果差——同一数字有平移、旋转、粗细变化。HOG 描述子把图像分成 4 个 10×10 cell，每 cell 统计梯度方向 16 bin 直方图，最终 64 维向量；这把"局部形状"编码进特征，比像素鲁棒得多。再做 Hellinger 归一化（L1 后开方）让特征度量变为 Hellinger 距离，对光照更稳。

deskew 基于图像矩：

$$
\text{skew} = \frac{\mu_{11}}{\mu_{02}},\quad
M = \begin{bmatrix}1 & \text{skew} & -\tfrac{SZ}{2}\,\text{skew}\\ 0 & 1 & 0\end{bmatrix},
$$

其中 $\mu_{pq}$ 是图像的 $(p+q)$ 阶中心矩。`warpAffine` 用 $M$ 把数字"摆正"。

HOG 计算用 `Sobel` 求 $g_x,g_y$，`cartToPolar` 转模长与角度，每 cell 累加 $b_n=16$ 个方向 bin：

$$
h_{i,b} = \sum_{p\in \text{cell}_i} \|\nabla I(p)\| \cdot \mathbb{1}\left[\theta(p)\in \text{bin}_b\right].
$$

随后整张图归一化 $h \leftarrow h / (\sum h + \epsilon)$，开方 $h \leftarrow \sqrt{h}$，再 L2——这一串组合把直方图变成"概率分布的平方根"，等价于 Hellinger 核的显式映射（参考源码注释里的 Arandjelovic & Zisserman *Three things everyone should know to improve object retrieval*）。

#### 关键 API

- `moments(img)`：计算图像矩，给 deskew 用。
- `warpAffine(img, dst, M, Size, WARP_INVERSE_MAP|INTER_LINEAR)`：去斜。
- `Sobel(img, gx, CV_32F, 1, 0)` + `cartToPolar(gx, gy, mag, ang)`：梯度幅值与角度。
- `KNearest::create()` + `train(samples, ROW_SAMPLE, labels)` + `findNearest(test, k, predictions)`：KNN 基线。
- `SVM::create()` + `setGamma(5.383)` + `setC(2.67)` + `setKernel(RBF)` + `setType(C_SVC)`：RBF SVM。
- `svm->predict(test, predictions)`：批量预测。

#### 处理流程

1. `load_digits`：`imread` 灰度读 `digits.png`，`split2d` 切成 20×20 小块，按行分配 0–9 标签。
2. `shuffle`：随机打乱，避免按类别顺序训练有偏。
3. 对每张 20×20 数字做 `deskew` → 64 维 HOG → Hellinger 归一化 → 拼成 `samples(N, 64, CV_32F)`。
4. 90% 训练 / 10% 测试切分。
5. KNN：`train` + `findNearest(test, 4, predictions)`，输出错误率与混淆矩阵。
6. SVM：`train` + `predict`，输出错误率与混淆矩阵；保存 `digits_svm.yml`。
7. `evaluate_model` 还会把错分数字底色染红做可视化拼图。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `SZ=20` | 数字图像边长 | 20, 28 | 与训练数据匹配，改了要重切 |
| `bin_n=16` | HOG 方向 bin 数 | 9（行人默认）, 16 | 大→更细方向，维数翻倍 |
| `K=4` (KNN) | KNearest 邻居数 | 1–10 | 大→稳但模糊；小→敏感 |
| `gamma=5.383` | RBF 核带宽 | 1e-3–10 | 大→边界复杂过拟合；小→近似线性 |
| `C=2.67` | SVM 软间隔 | 0.1–100 | 大→严过拟合；小→宽容欠拟合 |
| `train_n=0.9` | 训练比例 | 0.7–0.9 | 大→训练多，测试少 |

#### 关联与对比

- 与 [train_HOG.cpp](#522-train_hogcpp--自训-hog--svm-行人检测器) 对比：本例 SVM 做分类（predict 给类别），train_HOG 做检测（HOG 滑窗 + SVM 输出原始距离）。
- 与 [introduction_to_pca.cpp](#572-introduction_to_pcacpp--pca-求主方向) 对比：两者都从 `imgproc` 取特征交给 `ml`，但 HOG 是手工特征，PCA 是数据驱动降维。
- 原理对应 [principles §13.4 SVM](./principles.md#svm) 与 [§14.2 HOG+SVM](./principles.md#142-hog--svm-行人检测)。

#### 注意事项

- **不要直接用像素值喂 SVM**——不归一化、不平稳时效果差一个数量级。HOG 是数字识别的标准预处理。
- `cartToPolar` 默认输出弧度，`bin = ang * bin_n / (2π)`；改 `bin_n` 时务必同步检查范围。
- 错误率与混淆矩阵成对评估——只看错误率会掩盖"7 经常被分到 1"这种系统性错误。
- 数据增强可加 `--use_flip` 之类的镜像，本例未启用。

#### 应用场景

- 手写数字、字符识别（OCR）。
- 任何"小尺寸 ROI + 形状为主"的分类，套 HOG+SVM 模板。
- 教学示例：演示 `ml` 与 `imgproc` 的协作边界。

---

### 5.2.2 `train_HOG.cpp` —— 自训 HOG + SVM 行人检测器

> **源文件**：`samples/cpp/train_HOG.cpp` ｜ **所属模块**：`ml` + `objdetect` + `imgproc` ｜ **示例类型**：完整流程

#### 功能概述

从正例目录（行人）与负例目录（无行人）加载图像，统一尺寸后用 `HOGDescriptor::compute` 提取 HOG，转成 `CV_32FC1` 喂给线性 `SVM`。训练后把 SVM 的支持向量与 $\rho$ 拼成一个向量，写入 `HOGDescriptor::setSVMDetector`，再调 `detectMultiScale` 做滑动窗口检测。

#### 核心原理

**30 秒心智模型**：线性 SVM 学出的 $w$ 是 3780+1 维向量（64×7×8+1 列），其中 $w$ 与决策函数 $f(x)=w^\top x + b$ 一一对应。HOG 检测器要求 $f(x)\ge 0$ 时报"有人"，而 `SVM::predict` 默认按正负类返回 $\pm 1$，故要把支持向量与偏置 $\rho$ 组装成 `setSVMDetector` 期望的"权向量 + 阈值"格式。

导出公式（线性 SVM 单类决策函数）：

$$
f(x) = \sum_i \alpha_i y_i K(x_i, x) + b = w^\top x + b, \quad w=\sum_i \alpha_i y_i x_i.
$$

`getSupportVectors()` 给出 $w$（线性核下支持向量在多数情况就一个，等于 $w$），`getDecisionFunction(0, alpha, svidx)` 返回 $\rho$ 与对应的 $\alpha$ 索引。OpenCV 的 `setSVMDetector` 期望向量格式为 `[w_0, w_1, ..., w_{n-1}, -ρ]`，即最后一项是 $-\rho$ 而非 $b$。源码 `get_svm_detector` 的写法：

```cpp
vector<float> hog_detector(sv.cols + 1);
memcpy(&hog_detector[0], sv.ptr(), sv.cols * sizeof(float));
hog_detector[sv.cols] = (float)-rho;
```

完整训练流程包含"难例挖掘"的雏形——`sample_neg` 从大尺寸负例图中随机切出与正例同尺寸的子图，缓解"负例只用了少量切片"的问题。

#### 关键 API

- `HOGDescriptor(Size(64,128), Size(16,16), Size(8,8), Size(8,8), 9)`：标准行人检测窗口 64×128，cell 8×8，block 16×16，9 bin。
- `hog.compute(img, descriptors, ...)`: 对一张图算 HOG 描述子，输出列向量。
- `convert_to_ml(train_samples, trainData)`：把若干行/列向量转成行存储 `Mat`，源码注释清晰。
- `SVM::create()` + `setKernel(SVM::LINEAR)` + `setC(0.01)`：线性 SVM 训练。
- `get_svm_detector(svm)`：上面那段，**导出检测器**。
- `hog.setSVMDetector(detector)` + `hog.detectMultiScale(img, found, hitThreshold, winStride, padding, scale0)`：多尺度滑窗检测。

#### 处理流程

1. `load_images(pos_dir, pos_lst)` 与 `load_images(neg_dir, full_neg_lst)`。
2. `sample_neg(full_neg_lst, neg_lst, Size(64,128))`：从大图随机切小图补负例。
3. `computeHOGs(wsize, pos_lst, gradient_lst, use_flip)`：对每张正/负例算 HOG。
4. `convert_to_ml(gradient_lst, trainData)`：拼成 `Mat(N, 3780, CV_32FC1)`。
5. `SVM::train(trainData, ROW_SAMPLE, labels)`，标签 `+1/-1`。
6. `detector = get_svm_detector(svm)` → `HOGDescriptor::setSVMDetector(detector)`。
7. （可选）保存检测器到 `detector.yml`，`test_trained_detector` 在测试目录或视频上验证。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| 窗口 `winSize=(64,128)` | 检测目标尺寸 | 64×128 | 改了要重训，且影响检测速度 |
| `cellSize=(8,8)` | HOG cell | 8 | 小→维数翻倍 |
| `blockStride=(8,8)` | 块滑动步 | 8 | 小→维数翻倍 |
| `nbins=9` | 方向 bin | 9（行人）, 16（数字） | 大→维数翻倍 |
| `C=0.01` | 线性 SVM 惩罚 | 0.01–1 | 大→严，可能过拟合；小→宽容 |
| `hitThreshold=0` | 检测阈值 | 0–2 | 大→少而严；小→多、误检 |
| `winStride=(8,8)` | 滑窗步长 | 4–16 | 大→快、漏检；小→慢、精 |
| `scale0=1.05` | 金字塔缩放比 | 1.01–1.5 | 大→快、漏小目标 |

#### 关联与对比

- 与 [digits_svm.cpp](#521-digit_svmcpp--hog--svm端到端数字识别) 对比：本例 SVM 输出原始距离做检测，digits_svm 输出类别做识别；二者共用 `SVM` 但用法不同。
- 与 [第 6 章 `facedetect.cpp`/`peopledetect.cpp`](./ch06_objdetect_photo.md) 对比：行人检测在 ch06 有内置预训练 `HOGDescriptor::getDefaultPeopleDetector`；本例教你**自己训一个**。
- 原理对应 [principles §14.2 HOG+SVM 行人检测](./principles.md#142-hog--svm-行人检测)。

#### 注意事项

- **负例质量决定检测器上限**：`sample_neg` 只是简单随机切；要做难例挖掘（hard negative mining），用初版检测器扫一遍负例集，把误检子图加回负例重训。
- 线性 SVM 的 `getSupportVectors()` 在 `setKernel(LINEAR)` 下往往只有 1 行；若不满足 `alpha.total()==1` 断言，说明 SVM 把多类合并了，需要单类训练。
- `setSVMDetector` 要求向量最后一项是 $-\rho$（**符号**），源码的 `(float)-rho` 写法是规范用法。
- 训练样本量 < 几千时容易过拟合，行人检测通常需要数万正例 + 数十万负例。

#### 应用场景

- 训练自己的目标检测器（人、车、手势、标志）。
- 教学示例：理解 SVM 决策函数与 HOG 检测器的对接方式。
- 工业检测：HOG+SVM 至今仍是"刚体/类刚体目标"的高性价比方案。

---

## 5.3 监督分类全家桶：KNN、Bayes、LogReg、树模型

`letter_recog.cpp` 是 OpenCV 提供的**多分类器对比实验台**，能在一份数据上跑 RTrees/Boost/MLP/KNN/NBayes/SVM 六种模型并报告训练/测试准确率；`logistic_regression.cpp` 单独演示逻辑回归。

### 5.3.1 `letter_recog.cpp` —— 六分类器对比试验台

> **源文件**：`samples/cpp/letter_recog.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：完整流程

#### 功能概述

读 UCI Letter Recognition 数据集（20000 行 × 16 列特征，标签 A–Z 26 类），按命令行参数切换训练 RTrees / Boost / ANN_MLP / KNearest / NormalBayes / SVM，统一切分 80% 训练 + 20% 测试，输出 `Recognition rate: train=... test=...`。Boost 与 ANN_MLP 因为只支持二分类，源码做了"类别展开"技巧。

#### 核心原理

**30 秒心智模型**：六种模型各有归纳偏置——

- **KNearest**：懒惰学习，预测时找 $K$ 个最近邻投票；不需训练，但预测 $O(N)$。
- **NormalBayes**：假设每类是高斯分布，用均值/协方差做判别；最快但强假设。
- **SVM (C_SVC, LINEAR)**：最大间隔，正则项 + 合页损失。
- **RTrees**：集成多棵决策树投票，每棵用随机子集特征+样本；**抗过拟合**。
- **Boost**：迭代训练弱分类器（深度小决策树），加性组合，每轮重加权关注难样本；二分类原生。
- **ANN_MLP**：多层感知机，反向传播训练，sigmoid 激活；多分类用 one-hot 输出。

RTrees 与 Boost 的核心差异：随机森林**并行**训 $T$ 棵独立树再投票（bagging），Boosting**串行**训 $T$ 棵，每棵纠前一棵错（boosting）。

Boost 的多分类用"one-vs-all"展开：把 $N$ 个 26 类样本展开成 $26N$ 个二分类样本，每行末尾追加"当前类编号"作为额外特征，标签是"是否为该类"。预测时对 26 个二分类器各跑一遍，取 `RAW_OUTPUT` 最大者对应类别（源码 `build_boost_classifier` 的 `tptr[var_count]=(float)j` 与 `predict(..., RAW_OUTPUT)`）。

ANN_MLP 多分类则用 one-hot 输出：训练响应 `train_responses(i, cls_label) = 1.f`，预测取输出向量最大分量的索引加 `'A'`。

#### 关键 API

- `read_num_class_data(filename, var_count, &data, &responses)`：自写 CSV 解析。
- `prepare_train_data(data, responses, ntrain)`：构造 `TrainData` 并标 `var_type`（最后一列 `VAR_CATEGORICAL`）。
- `RTrees::create()` + `setMaxDepth(10)` + `setMinSampleCount(10)` + `setCalculateVarImportance(true)` + `setActiveVarCount(4)`：随机森林。
- `Boost::create()` + `setBoostType(Boost::GENTLE)` + `setWeakCount(100)` + `setMaxDepth(5)`：Boosting。
- `ANN_MLP::create()` + `setLayerSizes(Mat({16,100,100,26}))` + `setActivationFunction(SIGMOID_SYM)` + `setTrainMethod(BACKPROP, 0.001)` + `setTermCriteria(MAX_ITER, 300, 0)`：MLP。
- `KNearest::create()` + `setDefaultK(10)` + `setIsClassifier(true)`：KNN。
- `NormalBayesClassifier::create()`：高斯朴素贝叶斯。
- `StatModel::load<T>(filename)` 与 `model->save(filename)`：通用持久化。

#### 处理流程

1. `read_num_class_data`：从 `letter-recognition.data` 解析 16 列 float 特征 + 1 列字符标签。
2. `prepare_train_data`：构造 `TrainData`，标 `var_type`，80% 训练切分。
3. 按 `method` 调用 `build_rtrees_classifier` / `build_boost_classifier` / `build_mlp_classifier` / `build_knearest_classifier` / `build_nbayes_classifier` / `build_svm_classifier`。
4. `test_and_save_classifier`：在训练集与测试集上分别算准确率，可选保存 XML。
5. RTrees 额外打印 `getVarImportance()`：16 维特征重要性百分比。

#### 参数说明

| 模型 | 关键参数 | 含义 | 调大/调小 |
| --- | --- | --- | --- |
| RTrees | `maxDepth=10` | 单树最大深度 | 大→过拟合；小→欠 |
| RTrees | `activeVarCount=4` | 每节点随机选特征数 | 大→树相似；小→随机性强 |
| RTrees | `nTrees`（隐式 100+） | 树数量 | 大→稳但慢 |
| Boost | `weakCount=100` | 弱学习器数 | 大→更精但过拟合 |
| Boost | `weightTrimRate=0.95` | 权重裁剪 | 大→训练快但可能丢样本 |
| Boost | `maxDepth=5` | 弱树深度 | 大→过拟合 |
| ANN_MLP | `layerSizes={16,100,100,26}` | 各层神经元 | 宽→表达力强但慢 |
| ANN_MLP | `BACKPROP, lr=0.001` | 反向传播学习率 | 大→快但发散；小→慢 |
| ANN_MLP | `MAX_ITER=300` | 迭代次数 | 大→收敛更稳 |
| KNearest | `defaultK=10` | 邻居数 | 大→稳；小→敏感 |
| SVM | `C=1`, `LINEAR` | 软间隔+核 | 同 [5.1.1](#511-introduction_to_svmcpp--线性-svm-入门) |

#### 关联与对比

- 与 [tree_engine.cpp](#541-tree_enginecpp--dtrees--boost--rtrees) 对比：本例在同一份 26 类数据上跑三种树模型，下例在 CSV 上跑 DTrees/Boost/RTrees 并打印变量重要性，更贴近"工业数据"。
- 与 [neural_network.cpp](#561-neural_networkcpp--ann_mlp-简易二分类) 对比：本例 MLP 用 one-hot 多分类输出，下例用单列二分类。
- 原理对应 [principles §13.4 决策树/随机森林/Boosting](./principles.md#决策树--随机森林--boosting)。

#### 注意事项

- **Boost 多分类的展开技巧非常 hack**——把类别编号当特征追加，依赖 `var_type` 标记该列为 `VAR_CATEGORICAL`。生产环境改用 SVM/RTrees/ANN 原生多分类。
- `RTrees::getRoots()` 返回根节点列表，可用于树数计数；变量重要性 `getVarImportance()` 是各特征在所有节点的不纯度下降累计。
- 切分 80/20 用 `setTrainTestSplitRatio` 即可，本例自写 `prepare_train_data` 用 `sample_idx` 控制，是更底层用法。
- 同一数据集上，RTrees/Boost/MLP 通常 90%+，KNN 92%+，SVM 线性 ~80%；这是数据特性决定的。

#### 应用场景

- 模型选型基准：在同一份标定数据上跑六种模型对比准确率。
- 教学示例：理解 RTrees/Boost/MLP 的 API 差异与多分类适配技巧。
- 表格数据分类：16 列数值特征 + 26 类标签，与典型 ML benchmark 一致。

---

### 5.3.2 `logistic_regression.cpp` —— 逻辑回归二分类

> **源文件**：`samples/cpp/logistic_regression.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：完整流程

#### 功能概述

从 `data01.xml` 读 0/1 两类共 40 个 28×28 手写数字样本（每类 20 个），按偶/奇行切分训练/测试。训练 `LogisticRegression`（BATCH + L2 正则），输出准确率，保存 `NewLR_Trained.xml` 后加载复测验证持久化。

#### 核心原理

**30 秒心智模型**：逻辑回归是线性二分类的概率版——$p(y=1|x) = \sigma(w^\top x + b)$，其中 $\sigma(z) = 1/(1+e^{-z})$。损失是负对数似然加 L2：

$$
J(w) = -\frac{1}{N}\sum_i \left[y_i\log \sigma_i + (1-y_i)\log(1-\sigma_i)\right] + \frac{\lambda}{2}\lVert w\rVert^2.
$$

梯度 $\nabla J = \frac{1}{N}X^\top(\sigma - y) + \lambda w$，BATCH 一次用全量样本更新；MINI_BATCH 用 `miniBatchSize` 切片；SGD 一样一次一个。OpenCV 的 `LogisticRegression` 默认 BATCH，对小数据足够。

#### 关键 API

- `LogisticRegression::create()` + `setLearningRate(0.001)` + `setIterations(10)` + `setRegularization(REG_L2)` + `setTrainMethod(BATCH)` + `setMiniBatchSize(1)`。
- `train(data_train, ROW_SAMPLE, labels_train)`。
- `predict(data_test, responses)`：批量预测，输出 `CV_32S` 标签。
- `save(filename)` 与 `StatModel::load<LogisticRegression>(filename)`。

#### 处理流程

1. `FileStorage` 读 `data01.xml` 的 `datamat`/`labelsmat`，转 `CV_32F`。
2. 偶数行训练、奇数行测试；`showImage` 把行向量 `reshape(28,28)` 拼图显示。
3. `lr1->train` 后 `predict`，输出准确率。
4. 保存 `NewLR_Trained.xml`，用 `load` 重新载入，再 `predict` 比对结果。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `learningRate=0.001` | 梯度下降步长 | 1e-4–1e-2 | 大→快但发散；小→慢 |
| `iterations=10` | 迭代次数 | 10–1000 | 大→更收敛 |
| `regularization=REG_L2` | 正则类型 | L1/L2 | L1→稀疏权重 |
| `trainMethod=BATCH` | 训练方法 | BATCH/MINI_BATCH | MINI_BATCH 配合 `miniBatchSize` |
| `miniBatchSize=1` | 小批量大小 | 1–64 | 大→稳但慢 |

#### 关联与对比

- 与 [introduction_to_svm.cpp](#511-introduction_to_svmcpp--线性-svm-入门) 对比：逻辑回归用对数损失，SVM 用合页损失；前者输出概率，后者输出类别。
- 与 [digits_svm.cpp](#521-digit_svmcpp--hog--svm端到端数字识别) 对比：本例直接用 784 维像素喂 LR，错误率会高于 HOG+SVM，是经典的"特征工程对比"教学点。

#### 注意事项

- `data01.xml` 必须用 `samples::findFile` 找到，没数据集则直接报错退出。
- 标签必须 `CV_32F`，源码专门做了 `labels.convertTo(labels, CV_32F)`。
- 迭代 10 次太少，仅作演示；实际 LR 需 100+ 次才稳。
- LR 是线性模型，对 HOG 这类已高度线性可分的特征效果最好。

#### 应用场景

- 二分类点击率/ churn 预测（表格数据）。
- 教学示例：线性模型的概率输出与正则化对比。

---

## 5.4 无监督学习：聚类与混合分布

聚类不依赖标签，发现数据内在结构。`kmeans.cpp` 用硬分配（每点只属一类），`em.cpp` 用软分配（隶属度概率）。

### 5.4.1 `kmeans.cpp` —— K-means 聚类

> **源文件**：`samples/cpp/kmeans.cpp` ｜ **所属模块**：`core` ｜ **示例类型**：完整流程

#### 功能概述

随机生成 2–5 个高斯簇（每簇不同中心、不同点数），用 `kmeans` 聚成 $K$ 类，按 `labels` 染色、画中心，并打印 `compactness` 评估指标。每按一次键重新生成一组数据。

#### 核心原理

**30 秒心智模型**：K-means 假设数据由 $K$ 个等方差高斯球堆叠，迭代地"给点归类→更新中心→再归类"直至中心稳定。每次迭代最小化：

$$
J = \sum_{k=1}^{K}\sum_{x\in C_k}\|x - \mu_k\|^2, \quad \mu_k = \frac{1}{|C_k|}\sum_{x\in C_k} x.
$$

Lloyd 算法两步交替：(1) 分配 $C_k = \{x: k = \arg\min_j \|x-\mu_j\|^2\}$；(2) 更新 $\mu_k$。$J$ 单调下降，但只收敛到**局部最优**，故常用 `attempts` 多次随机初始化取最优。`KMEANS_PP_CENTERS`（k-means++）按距离加权选初始中心，比纯随机稳定得多。

#### 关键 API

- `rng.fill(pointChunk, RNG::NORMAL, center, sigma)`：按高斯分布填充点。
- `randShuffle(points, 1, &rng)`：打乱点顺序，避免数据组织性影响聚类。
- `kmeans(points, K, labels, TermCriteria(EPS+COUNT, 10, 1.0), attempts, KMEANS_PP_CENTERS, centers)`。
- 返回值 `compactness`：最终 $J$ 值，越小越好。

#### 处理流程

1. 随机选 `clusterCount ∈ [2, 5]`，`sampleCount ∈ [1, 1000]`。
2. 按簇数把 `points` 切分，每段用 `rng.fill(NORMAL, center, sigma)` 填高斯点。
3. `randShuffle` 打乱。
4. `kmeans` 输出 `labels` 与 `centers`。
5. 按标签查色表 `colorTab`，画点画中心，输出 `compactness`。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `K=clusterCount` | 簇数 | 2–10 | 大→簇细；小→合并 |
| `attempts=3` | 重试次数 | 1–10 | 大→稳，取最优 |
| `TermCriteria` 10 次, ε=1.0 | 迭代终止 | 10–100 | 大→精但慢 |
| `KMEANS_PP_CENTERS` | 初始化方法 | 默认 | 比 `RANDOM_CENTERS` 稳 |
| `RNG::NORMAL` σ | 簇内方差 | 5%–15% 图宽 | 大→簇重叠；小→分明 |

#### 关联与对比

- 与 [em.cpp](#542-emcpp--高斯混合-em-聚类) 对比：kmeans 是 EM 在协方差相同且硬分配时的特例；EM 软分配且能学协方差。
- 与 [第 2 章分水岭](./ch02_imgproc.md) 对比：分水岭基于梯度拓扑聚类，kmeans 基于距离度量，二者应用层不同。

#### 注意事项

- `clusterCount` 必须 `≤ sampleCount`，源码用 `MIN` 守护。
- `points` 维度由 `Mat(sampleCount, 1, CV_32FC2)` 决定——CV_32FC2 表示 2D 点，要更高维改 `CV_32F`+`cols`。
- `compactness` 跨不同数据集不可比，只用于同数据集调 $K$。
- K-means 假设球形簇，对长条形/环形结构失效——这是它最经典的局限。

#### 应用场景

- 颜色量化、图像分割的初始化。
- 特征空间初步分组，配合监督模型做半监督。
- 教学示例：演示 Lloyd 迭代与 k-means++ 的差异。

---

### 5.4.2 `em.cpp` —— 高斯混合 EM 聚类

> **源文件**：`samples/cpp/em.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：完整流程

#### 功能概述

生成 4 个二维高斯簇共 100 个点，用 `EM` 拟合 4 分量高斯混合模型（GMM），对图像每个像素预测后验最大类别，染色可视化决策区域。

#### 核心原理

**30 秒心智模型**：GMM 假设数据由 $K$ 个高斯混合：$p(x) = \sum_k \pi_k \mathcal{N}(x;\mu_k,\Sigma_k)$。EM 用隐变量 $z_i$ 表示"点 $i$ 来自哪个分量"，迭代 E/M 两步：

- **E 步**：计算后验（"责任"）$\gamma_{ik} = \frac{\pi_k\mathcal{N}(x_i;\mu_k,\Sigma_k)}{\sum_j \pi_j\mathcal{N}(x_i;\mu_j,\Sigma_j)}$。
- **M 步**：用 $\gamma$ 加权更新参数 $\pi_k = \frac{N_k}{N}$、$\mu_k = \frac{1}{N_k}\sum_i \gamma_{ik} x_i$、$\Sigma_k = \frac{1}{N_k}\sum_i \gamma_{ik}(x_i-\mu_k)(x_i-\mu_k)^\top$。

K-means 是 EM 在 $\Sigma_k = \sigma^2 I$（球协方差）且责任退化为 0/1 时的极限。`setCovarianceMatrixType` 控制 $\Sigma_k$ 形式：

- `COV_MAT_SPHERICAL`：$\Sigma_k = \sigma_k^2 I$，参数最少，最不稳。
- `COV_MAT_DIAGONAL`：对角协方差，常用。
- `COV_MAT_GENERIC`：满秩，参数最多，需大数据。

#### 关键 API

- `EM::create()` + `setClustersNumber(N)` + `setCovarianceMatrixType(EM::COV_MAT_SPHERICAL)` + `setTermCriteria(COUNT+EPS, 300, 0.1)`。
- `trainEM(samples, noArray(), labels, noArray())`：训练，输出每点硬标签（取后验最大）。
- `predict2(sample, noArray())`：返回 `[logLikelihood, label]` 两元素向量。

#### 处理流程

1. 构造 100×2 高斯点，按 $N=4$ 等分 4 段，各段用 `randn(samples_part, mean, sigma)` 填充。
2. `EM::trainEM` 输出 `labels`。
3. 对图像每像素 `predict2` 取 label，查 `colors` 染色决策区域。
4. 把训练点叠在决策区域上。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `clustersNumber=N` | 分量数 | 2–10 | 大→更细；小→合并 |
| `COV_MAT_SPHERICAL` | 协方差类型 | SPHERICAL/DIAGONAL/GENERIC | GENERIC 最灵活但需大数据 |
| `MAX_ITER=300, EPS=0.1` | EM 迭代终止 | 100–1000 | 大→更收敛 |
| `randn` σ=30 | 簇内方差 | 数据相关 | 大→簇重叠 |

#### 关联与对比

- 与 [kmeans.cpp](#541-kmeanscpp--k-means-聚类) 对比：EM 软分配 + 学协方差；kmeans 硬分配 + 假设球状。
- 与 [letter_recog.cpp 中的 NormalBayes](#531-letter_recogcpp--六分类器对比试验台) 对比：NormalBayes 是"每类一个 GMM 分量"的简化 EM，参数固定后不再迭代。

#### 注意事项

- `samples` 必须是 `CV_32FC1` 行存储，源码用 `samples.reshape(2,0)`→`reshape(1,0)` 切换通道/行，是 OpenCV 经典套路。
- `predict2` 返回 `Vec2d`，第二元素是 label，第一是 log 似然——可用于异常检测（似然过低判异常）。
- 协方差类型选错会崩溃：球协方差对长条形簇完全无能为力。
- GMM 收敛慢，300 次迭代未必够；建议 `TermCriteria` 加 `EPS` 让算法自然停止。

#### 应用场景

- 背景建模（MOG2 背景减除即"每像素一个 GMM"，见 [ch04](./ch04_video.md)）。
- 软聚类 + 异常检测（用 log 似然判异常点）。
- 语音/图像特征分布建模。

---

## 5.5 决策树与集成：DTrees / Boost / RTrees

`tree_engine.cpp` 在 CSV 数据上同时训练 DTrees、Boost、RTrees 并报告训练/测试误差，是这三个 API 的统一对照示例。

### 5.5.1 `tree_engine.cpp` —— DTrees / Boost / RTrees

> **源文件**：`samples/cpp/tree_engine.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：完整流程

#### 功能概述

从 CSV 文件载入表格数据（指定响应列与变量类型 spec），50% 训练 50% 测试。依次训练 `DTrees`（单决策树）、`Boost`（仅 2 类时启用）、`RTrees`（随机森林），统一用 `train_and_print_errs` 打印训练/测试误差。RTrees 还打印变量重要性 `getVarImportance` 与每棵树根节点。

#### 核心原理

**30 秒心智模型**：

- **决策树 DTrees**：从根节点开始，每个节点选一个变量+阈值，使分桶后**不纯度下降最大**。Gini 不纯度 $I(p) = 1-\sum_c p_c^2$（分类）或方差（回归）。递归分裂到 `maxDepth` 或样本不足停止。
- **Boosting**：迭代训练 $T$ 棵弱树（`weakCount`），每棵拟合上一棵的残差；最终 $F(x) = \sum_t \alpha_t f_t(x)$。`Boost::GENTLE`/`DISCRETE`/`REAL` 是不同变种。
- **Random Forest RTrees**：并行训练 $T$ 棵决策树，每棵用 bootstrap 样本 + 随机选 `activeVarCount` 个特征做节点分裂；预测时投票（分类）或平均（回归）。`getVarImportance` 给出各特征在所有节点的不纯度下降累计。

DTrees 易过拟合（深度大时记忆训练集）；Boost 在弱学习器上逐步纠错；RTrees 用 bagging + 特征随机化抗过拟合。三者构成"基础树→加性集成→并行集成"的演化谱系。

#### 关键 API

- `TrainData::loadFromCSV(filename, 0, response_idx, response_idx+1, typespec)`：从 CSV 载入并指定变量类型 spec（`ord[..]cat[..]`）。
- `data->setTrainTestSplitRatio(0.5)`：50/50 切分，自动洗牌。
- `DTrees::create()` + `setMaxDepth(10)` + `setMinSampleCount(2)` + `setCVFolds(0)`（关交叉验证）。
- `Boost::create()` + `setBoostType(Boost::GENTLE)` + `setWeakCount(100)` + `setWeightTrimRate(0.95)` + `setMaxDepth(2)`。
- `RTrees::create()` + `setMaxDepth(10)` + `setCalculateVarImportance(true)` + `setActiveVarCount(0)`（0 表示全部特征参与）。
- `train_and_print_errs(model, data)`：内部用 `model->train(data)` + `calcError(data, false/true, ...)`。

#### 处理流程

1. `CommandLineParser` 解析 `r`（响应列）、`ts`（类型 spec）、`@input`（CSV 路径）。
2. `TrainData::loadFromCSV` 读入，`setTrainTestSplitRatio(0.5)`。
3. 打印 `getNTestSamples/getNTrainSamples`。
4. 创建 `DTrees` 配置参数→`train_and_print_errs`。
5. 如果 `getClassLabels().total() <= 2`（2 类或回归），创建 `Boost`→`train_and_print_errs`。
6. 创建 `RTrees`→`train_and_print_errs`→打印变量重要性。

#### 参数说明

| 模型 | 关键参数 | 含义 | 调大/调小 |
| --- | --- | --- | --- |
| DTrees | `maxDepth=10` | 树深 | 大→过拟合 |
| DTrees | `minSampleCount=2` | 节点最小样本 | 大→更粗更稳 |
| DTrees | `CVFolds=0` | 交叉验证折数 | 大→自带剪枝但慢 |
| Boost | `weakCount=100` | 弱树数 | 大→更精但慢 |
| Boost | `weightTrimRate=0.95` | 权重裁剪率 | 大→快但丢样本 |
| RTrees | `maxDepth=10` | 单树深 | 大→单树过拟合；集成缓解 |
| RTrees | `activeVarCount=0` | 每节点随机特征数 | 0 表示全特征；常用 $\sqrt{n}$ |
| RTrees | `termCrit MAX_ITER=100` | 树数 | 大→稳但慢 |
| `var_type` spec | `ord[..]cat[..]` | 变量类型 | 错了会破坏分类 |

#### 关联与对比

- 与 [letter_recog.cpp](#531-letter_recogcpp--六分类器对比试验台) 对比：本例对任意 CSV 跑，更工程化；letter_recog 针对单一数据集，但覆盖更多模型。
- 与 [train_HOG.cpp](#522-train_hogcpp--自训-hog--svm-行人检测器) 对比：树模型不依赖核函数，更适合"特征数适中、特征独立含义清晰"的表格数据；HOG+SVM 更适合图像。

#### 注意事项

- Boost 在 OpenCV 中**原生只支持二分类或回归**——这就是 `if getClassLabels().total() <= 2` 的来历。多分类要 [letter_recog.cpp 的展开技巧](#531-letter_recogcpp--六分类器对比试验台)。
- `setCVFolds>0` 启用交叉验证剪枝，对 DTrees 抗过拟合至关重要；本例设 0 仅作演示。
- `var_type` spec 必须把响应列标 `cat`（分类），否则会被当回归处理。
- RTrees 的 `getVarImportance` 可用于特征选择：剔除低重要性特征重训，常能进一步提升泛化。

#### 应用场景

- 表格数据分类/回归（金融、医疗、营销）。
- 特征重要性筛选：RTrees 的 `getVarImportance` 是无监督的"特征排序"工具。
- 教学示例：理解从单树到集成的演化。

---

## 5.6 神经网络与 dnn 桥梁

### 5.6.1 `neural_network.cpp` —— ANN_MLP 简易二分类

> **源文件**：`samples/cpp/neural_network.cpp` ｜ **所属模块**：`ml` ｜ **示例类型**：snippet

#### 功能概述

构造 100×100 随机高斯数据，前 50 行标签 (1,0)、后 50 行 (0,1) 做 one-hot 二分类。创建 3 层 MLP（输入 100 → 隐 20 → 输出 2），用 `BACKPROP` 训练，对全量数据 `predict` 输出概率。

#### 核心原理

**30 秒心智模型**：多层感知机（MLP）是若干全连接层 + 非线性激活的串联。对第 $l$ 层：$a^{(l)} = \sigma(W^{(l)} a^{(l-1)} + b^{(l)})$，输入层 $a^{(0)} = x$，输出层 `softmax`/`sigmoid`。反向传播用链式法则把误差从输出层往回传到每层权重：$\delta^{(l)} = (W^{(l+1)})^\top \delta^{(l+1)} \odot \sigma'(a^{(l)})$。

OpenCV `ANN_MLP` 支持三种激活：`IDENTITY`、`SIGMOID_SYM`（对称 sigmoid，需缩放范围）、`GAUSSIAN`。`BACKPROP` 是经典 SGD，`RPROP` 是无步长的弹性传播。损失默认 MSE，多分类建议配 one-hot 输出 + sigmoid + MSE。

#### 关键 API

- `ANN_MLP::create()` + `setLayerSizes(Mat({100, 20, 2}))`。
- `setActivationFunction(ANN_MLP::SIGMOID_SYM, 0.1, 0.1)`：第二/三参数是 $\alpha$/$\beta$，控制 sigmoid 范围。
- `setTrainMethod(ANN_MLP::BACKPROP, 0.1, 0.1)`：后两参为学习率与动量。
- `train(TrainData)`。
- `isTrained()` 检查是否收敛，`predict(input, output)` 输出 1×输出维度向量。

#### 处理流程

1. `randn` 生成 100×100 数据 + one-hot 标签 (100×2)。
2. 三层 `[100, 20, 2]`，`SIGMOID_SYM` 激活 + `BACKPROP`。
3. `TrainData::create` 后 `train`。
4. `isTrained` 后对全量行 `predict` 打印输出。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `layerSizes=[100,20,2]` | 各层神经元 | 数据驱动 | 宽→表达力强但慢、过拟合 |
| `SIGMOID_SYM α=0.1,β=0.1` | sigmoid 缩放 | [0,1] | 影响 sigmoid 形状与梯度 |
| `BACKPROP lr=0.1, momentum=0.1` | 学习率+动量 | 1e-4–1 | 大→快但发散；小→慢 |
| `MAX_ITER=300` | 迭代次数 | 100–10000 | 大→收敛更稳 |

#### 关联与对比

- 与 [letter_recog.cpp 的 MLP 部分](#531-letter_recogcpp--六分类器对比试验台) 对比：本例是 snippet 级演示，下例是带 one-hot 多分类 + 数据加载的工程版。
- 与 [digits_lenet.cpp](#562-digit_lenetcpp--dnn-模型推理桥梁) 对比：`ANN_MLP` 只能做浅层 MLP；要做 LeNet/CNN 必须走 `dnn` 模块。

#### 注意事项

- `SIGMOID_SYM` 输出范围 $[-1, 1]$，标签需相应缩放（源码用 0/1 是简化）。
- `BACKPROP` 在大数据上极慢，工业上已基本被深度学习取代。
- 隐层维度过小→欠拟合；过大→过拟合且训练慢；常用经验 $\sqrt{n_{in}\cdot n_{out}}$。
- 多分类应 one-hot 输出 + softmax 或 sigmoid + MSE，比直接输出类别号稳。

#### 应用场景

- 教学示例：演示反向传播的最简形式。
- 浅层 MLP 在小数据集上的快速实验。
- 不便引入 `dnn` 的轻量场景。

---

### 5.6.2 `digits_lenet.cpp` —— dnn 模型推理桥梁

> **源文件**：`samples/cpp/digits_lenet.cpp` ｜ **所属模块**：`dnn` + `imgproc` + `highgui` ｜ **示例类型**：完整流程

#### 功能概述

加载 Caffe 格式的 LeNet-5 模型（`.prototxt` + `.caffemodel`），从摄像头或图像读入，做白纸手写数字的实时识别。把识别结果叠加到原图显示。这是 `ml` 章里**唯一一个使用 `dnn` 而非 `ml` 的示例**，作为"传统 ML → 深度学习"的桥梁。

#### 核心原理

**30 秒心智模型**：LeNet-5 是 1998 年 LeCun 提出的 CNN，结构 Conv→Pool→Conv→Pool→FC→FC→Softmax，约 6 万参数。它的核心是用**卷积层**自动学习图像特征，无需手工 HOG/SURF——这把"特征工程"也交给模型学。OpenCV `dnn` 模块只做**推理**（forward），不训练；模型在外部框架（Caffe/TF/PyTorch）训好后导入。

推理流程：图像 → 预处理（resize/归一化/mean subtraction）→ `blobFromImage` → `net.setInput(blob)` → `net.forward()` → 输出 $1\times 10$ 概率向量 → `getMaxClass` 取 argmax。

#### 关键 API

- `dnn::readNet(modelTxt, modelBin)`：从 prototxt + caffemodel 载入。
- `blobFromImage(input, scale, size, mean, swapRB, crop)`：把 `Mat` 转 NCHW `blob`。
- `net.setInput(blob)` + `net.forward()`：前向推理。
- `getMaxClass(probBlob, classId, classProb)`：手写函数取最大概率类。

#### 处理流程

1. `CommandLineParser` 解析模型路径、设备号、阈值。
2. `readNet(modelTxt, modelBin)` 载入 LeNet。
3. `VideoCapture` 或 `imread` 取帧。
4. 预处理：转灰度、二值化或阈值分割、找 ROI。
5. `blobFromImage(roi, 1/255.0, Size(28,28), Scalar(), false, false)`。
6. `setInput` + `forward` → `probBlob`。
7. `getMaxClass` 取最大概率类，叠加显示。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `modelTxt`/`modelBin` | 模型定义/权重 | 必填 | 路径错则退出 |
| `width/height=640/480` | 摄像头尺寸 | 与硬件匹配 | 影响预处理性能 |
| `thr=0.7` | 置信度阈值 | 0.5–0.9 | 大→少而严；小→多误检 |

#### 关联与对比

- 与 [digits_svm.cpp](#521-digit_svmcpp--hog--svm端到端数字识别) 对比：HOG+SVM 90%+，LeNet 99%+，且无需手工 deskew/HOG——这就是深度学习的优势。
- 与 [第 6 章 dbt_face_detection.cpp](./ch06_objdetect_photo.md) 对比：两者都是 `dnn` 推理示例，本章只作桥梁；完整 dnn 教程需 Model Zoo。

#### 注意事项

- 模型文件必须存在；OpenCV 不提供模型，注释里有下载链接。
- `dnn` 只做推理，要训练请用 Caffe/PyTorch/TF。
- 摄像头未指定时用 `0` 默认设备；建议白纸 + 大字占满画面。
- `getMaxClass` 自写函数，对 $1\times N$ blob 简单 argmax。

#### 应用场景

- 从 SVM/KNN 升级到 CNN 的迁移教学。
- 在 OpenCV 中嵌入预训练 CNN 做推理（无需重训）。
- 实时摄像头识别 demo。

---

## 5.7 降维：PCA 主成分分析

PCA 是把高维数据投影到低维子空间、最大化保留方差的经典方法。两个示例分别用 trackbar 调保留方差、用 PCA 提取物体主方向。

### 5.7.1 `pca.cpp` —— PCA 人脸降维与保留方差 trackbar

> **源文件**：`samples/cpp/pca.cpp` ｜ **所属模块**：`core` ｜ **示例类型**：完整流程

#### 功能概述

从图像列表文件读入若干张等大的人脸图像，每张 `reshape(1,1)` 摊平为行向量，做 PCA。用 trackbar 调"保留方差百分比"（0%–100%），实时重算 PCA、投影、反投影重建第一张脸，观察主成分数对人脸重建质量的影响。

#### 核心原理

**30 秒心智模型**：PCA 在数据协方差矩阵上做特征分解，按特征值从大到小取前 $k$ 个特征向量作为低维空间的基。数据点 $x$ 投影为 $y = W_k^\top (x - \mu)$，反投影为 $\hat x = W_k y + \mu$。保留方差比例 $r = \sum_{i=1}^{k}\lambda_i / \sum_i \lambda_i$——这是"信息保留"的标准度量。

数学推导：设数据 $X\in\mathbb{R}^{N\times d}$ 已中心化（减均值），协方差 $C = \frac{1}{N}X^\top X$。$C$ 是半正定对称矩阵，特征分解 $C = V\Lambda V^\top$，$\Lambda$ 对角元素即各方向方差。取前 $k$ 大特征值的特征向量组 $W_k\in\mathbb{R}^{d\times k}$，则投影 $Y = XW_k$，反投影 $\hat X = YW_k^\top + \mu$。

对图像降维的关键技巧：图像 $28\times 28$ 共 784 维，N 张图构成 $N\times 784$ 矩阵，直接 `PCA(data, Mat(), PCA::DATA_AS_ROW, var)` 即可。`DATA_AS_ROW` 表示每行一个样本。

#### 关键 API

- `PCA(data, Mat(), PCA::DATA_AS_ROW, retainedVariance)`：保留方差比例（0–1）或主成分数（≥1）。
- `pca.project(data.row(0))`：投影到低维空间，返回 $1\times k$ 行向量。
- `pca.backProject(point)`：反投影回原空间。
- `pca.eigenvectors` / `pca.eigenvalues`：主成分向量与对应特征值。
- `pca.mean`：数据均值行向量。
- `createTrackbar("Retained Variance (%)", winName, &pos, 100, onTrackbar, &p)`。

#### 处理流程

1. `read_imgList(imgList, images)`：从文本文件逐行读图像路径。
2. `formatImagesForPCA(images)`：把每张 $H\times W$ 灰度图 `reshape(1, 1*W)`，拼成 $N\times (H\cdot W)$ 矩阵。
3. `PCA(data, Mat(), PCA::DATA_AS_ROW, 0.95)` 初始保留 95% 方差。
4. `project` + `backProject` 重建第一张图。
5. trackbar 回调按 pos/100 重算 PCA，实时更新重建结果。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `retainedVariance` (0–1) | 保留方差比例 | 0.95 | 大→更精、维数多；小→模糊、维数少 |
| `PCA::DATA_AS_ROW` | 数据布局 | 默认 | 必须匹配 `Mat` 的行存储方式 |
| `images.size() >= 2` | 样本数 | ≥2 | 少→协方差不满秩 |
| 图像尺寸 | 必须等大 | 数据决定 | 不等大会 reshape 失败 |

#### 关联与对比

- 与 [introduction_to_pca.cpp](#572-introduction_to_pcacpp--pca-求主方向) 对比：本例用 PCA 做**降维与重建**，下例用 PCA 做**主方向提取**（求物体姿态角）。
- 与 [letter_recog.cpp](#531-letter_recogcpp--六分类器对比试验台) 对比：PCA 可作为分类前的预处理降维——把 784 维人脸降到 50 维再喂 SVM，常能提速且防过拟合。

#### 注意事项

- `formatImagesForPCA` 假设所有图像尺寸相同；不等大要先 `resize`。
- `retainedVariance` 是比例（0–1）而非主成分数；源码 trackbar 用 `pos/100.0` 转换。
- PCA 对数据尺度敏感：图像必须先归一化到相同范围（如 `[0,255]` 或 `[0,1]`）。
- 重建图必须 `toGrayscale` 归一化才能稳定显示（PCA 反投影可能越界）。

#### 应用场景

- 人脸识别特征脸（eigenfaces）方法。
- 高维数据可视化前降到 2–3 维。
- 数据压缩与去噪（丢弃低方差分量相当于低通滤波）。
- 教学示例：直观感受"方差=信息"的对应关系。

---

### 5.7.2 `introduction_to_pca.cpp` —— PCA 求主方向

> **源文件**：`samples/cpp/tutorial_code/ml/introduction_to_pca/introduction_to_pca.cpp` ｜ **所属模块**：`core` + `imgproc` ｜ **示例类型**：完整流程

#### 功能概述

读 `pca_test1.jpg`，二值化 + `findContours` 找轮廓；对每个轮廓做 PCA，提取**第一主方向**（最大特征值对应特征向量），用 `drawAxis` 画出主轴与次轴，并把主方向角度 `atan2(eigen_vecs[0].y, eigen_vecs[0].x)` 返回。

#### 核心原理

**30 秒心智模型**：对一组二维点 $(x_i, y_i)$，PCA 给出两个正交主轴——第一主轴指向"点云最延展方向"，即物体的长轴；第二主轴正交于它。这个方向就是物体的**朝向角**。用特征向量乘特征值画轴长，主轴长、次轴短，可视化直观。

数学上：构造 $N\times 2$ 数据矩阵，PCA 求得 `mean`（中心点）、`eigenvectors`（2×2，每行一个主方向）、`eigenvalues`（2×1，对应方差）。第一行 `eigenvectors[0]` 即第一主方向，对应 `eigenvalues[0]` 最大。

#### 关键 API

- `PCA(data_pts, Mat(), PCA::DATA_AS_ROW)`：对二维点做 PCA。
- `pca_analysis.mean`：2D 中心点（`Point`）。
- `pca_analysis.eigenvectors.at<double>(i, 0/1)`：第 $i$ 主方向向量。
- `pca_analysis.eigenvalues.at<double>(i)`：第 $i$ 方向方差。
- `atan2(eigen_vecs[0].y, eigen_vecs[0].x)`：主方向角（弧度）。
- `drawAxis(img, p, q, color, scale)`：画带箭头的轴线。

#### 处理流程

1. `imread` 读图，`cvtColor` 转灰度。
2. `threshold(gray, bw, 50, 255, THRESH_BINARY | THRESH_OTSU)`：Otsu 二值化。
3. `findContours(bw, contours, RETR_LIST, CHAIN_APPROX_NONE)`：找所有轮廓。
4. 遍历轮廓，过滤 `area < 1e2 || area > 1e5` 的小/大轮廓。
5. `getOrientation(contour, img)` 做 PCA + 画主次轴 + 返回角度。

#### 参数说明

| 参数 | 含义 | 典型范围/默认 | 调大/调小会怎样 |
| --- | --- | --- | --- |
| `threshold=50` | 二值化阈值 | 配合 Otsu | Otsu 自动 |
| `THRESH_OTSU` | 自适应阈值 | 默认 | 与 `THRESH_BINARY` 组合 |
| `area ∈ [1e2, 1e5]` | 轮廓面积过滤 | 数据相关 | 过滤噪声或主体 |
| `drawAxis scale=0.02` | 轴线缩放 | 0.01–0.5 | 大→长 |

#### 关联与对比

- 与 [pca.cpp](#571-pcacpp--pca-人脸降维与保留方差-trackbar) 对比：本例 PCA 做**几何分析**（2D 点云主方向），下例做**统计降维**（高维人脸）。PCA 数学同源，应用方向不同。
- 与 [第 3 章 `fitEllipse.cpp`/`minarea.cpp`](./ch03_features.md) 对比：椭圆拟合与最小外接矩形也是求物体主方向的常用方法，PCA 是其统计版本。

#### 注意事项

- `data_pts` 必须是 `CV_64F`（PCA 对 `CV_32F`/`CV_64F` 都支持，但取 `at<double>` 要一致）。
- 第一主方向不区分正负（特征向量乘 -1 仍是特征向量）；如要稳定方向需配合轮廓方向或额外约束。
- 轮廓必须先 `threshold` 得到二值图；PCA 对噪声点敏感，过小轮廓会乱方向。
- `getOrientation` 返回弧度，要度数乘 `180/CV_PI`。

#### 应用场景

- 物体姿态估计（如工件方向、车牌角度）。
- 文本倾斜校正（PCA 求文本行方向）。
- 形状对齐与归一化预处理。
- 教学示例：PCA 几何直觉的最简演示。

---

## 5.8 本章小结

`ml` 模块是 OpenCV 对经典机器学习的工程化封装。本章覆盖的 14 个示例可归纳为五条主线：

| 主线 | 示例 | 核心 API | 关键参数 |
| --- | --- | --- | --- |
| SVM 分类与检测 | `introduction_to_svm`、`non_linear_svms`、`digits_svm`、`train_HOG`、`train_svmsgd` | `SVM`、`SVMSGD` | `C`、`gamma`、核类型 |
| 监督分类全家桶 | `letter_recog`、`logistic_regression` | `RTrees`/`Boost`/`ANN_MLP`/`KNearest`/`NormalBayes`/`LogisticRegression` | `maxDepth`、`weakCount`、`K`、`learningRate` |
| 无监督聚类 | `kmeans`、`em` | `kmeans`、`EM` | `K`、`attempts`、`clustersNumber`、协方差类型 |
| 神经网络与 dnn 桥梁 | `neural_network`、`digits_lenet` | `ANN_MLP`、`dnn::Net` | `layerSizes`、`BACKPROP`、模型路径 |
| 降维与几何分析 | `pca`、`introduction_to_pca` | `PCA` | `retainedVariance`、协方差类型 |
| 树模型 | `tree_engine` | `DTrees`/`Boost`/`RTrees` | `maxDepth`、`activeVarCount`、`weakCount` |

### 5.8.1 贯穿原则与章节衔接

**贯穿性原则**：

1. **`StatModel` 统一接口**：所有模型都从 `train(TrainData)` 入，`predict(Mat)` 出。先把数据拍成 `CV_32F` 行矩阵是第一守则。
2. **特征工程先于模型选择**：`digits_svm` 的 HOG+Hellinger、`train_HOG` 的 HOG、`pca` 的 `reshape(1,1)` 都说明——`ml` 不做特征提取，特征质量决定上限。
3. **线性 vs 非线性**：线性 SVM/LogReg 在高维稀疏（HOG、SURF-BoVW）上往往优于 RBF；非线性核只在低维密集数据上占优。
4. **集成 > 单模型**：RTrees/Boost 通常优于 DTrees 单树，因为集成抗过拟合；`getVarImportance` 还顺带做了特征选择。
5. **浅层 vs 深度**：`ANN_MLP` 浅层网络已被 `dnn` 模块的 CNN 取代；本章只在 `digits_lenet.cpp` 做桥梁，完整 dnn 教程见 Model Zoo。
6. **PCA 的双重身份**：作为统计降维（人脸特征脸、可视化前处理）与几何分析（物体主方向）共享同一数学，工程语境决定用法。

**与其他章节的衔接**：

- 上游：[第 2 章图像处理](./ch02_imgproc.md)（Sobel/直方图/Hough 提供 HOG 的梯度算子）、[第 3 章局部特征](./ch03_features.md)（SURF/ORB 描述子可作为 SVM 的特征向量）。
- 下游：[第 6 章目标检测](./ch06_objdetect_photo.md)（HOG+SVM 行人检测、级联分类器训练）、[第 4 章视频分析](./ch04_video.md)（背景减除 MOG2 即"每像素一个 GMM"）。
- 原理总纲：[principles §13.4 传统机器学习](./principles.md#134-传统机器学习)。

**学习建议**：先吃透 `StatModel`/`TrainData` 抽象与行存储约定，再以 `introduction_to_svm.cpp` → `digits_svm.cpp` → `train_HOG.cpp` 为黄金路径理解"SVM 训练 → 检测器导出 → 滑窗检测"的完整工程链；`letter_recog.cpp` 与 `tree_engine.cpp` 是多模型对比基准；`kmeans.cpp` 与 `em.cpp` 演示无监督谱系；`pca.cpp` 与 `introduction_to_pca.cpp` 演示降维与几何分析的双面性；`digits_lenet.cpp` 作为传统 ML → 深度学习的迁移起点。

### 5.8.2 工程实践建议

#### 功能概述

本节汇总 `ml` 模块在真实工程落地中的选型、调参、部署建议，覆盖从特征工程到模型部署的全链路。

#### 核心原理

工程实践的核心原理：**特征质量 > 模型选择 > 参数调优**。再好的模型也无法弥补糟糕的特征；线性模型在好特征上往往不输非线性模型。

#### 关键 API

- `TrainData::create`：统一数据入口，务必设 `ROW_SAMPLE` + `CV_32F`。
- `StatModel::calcError`：一键训练/测试集误差评估。
- `StatModel::save` / `load<T>`：模型持久化。
- `PCA::project` / `backProject`：降维与重建。

#### 处理流程

1. 特征提取（HOG/SURF/像素直方图）→ `CV_32F` 行矩阵。
2. `TrainData::create` + `setTrainTestSplitRatio(0.7)` 切分。
3. 多模型对比（`letter_recog.cpp` 模式）→ 选最优。
4. 网格搜索关键参数（`C`/`gamma`/`maxDepth`）。
5. `save` 模型文件 → 部署时 `load` + `predict`。

#### 参数说明

| 参数 | 模型 | 建议范围 | 调优方向 |
| --- | --- | --- | --- |
| `C` | SVM | 0.1–100 | 大→硬间隔，小→软间隔 |
| `gamma` | RBF SVM | 0.001–10 | 大→过拟合，小→欠拟合 |
| `maxDepth` | DTrees/RTrees | 5–20 | 深→过拟合 |
| `weakCount` | Boost | 50–500 | 多→强集成 |
| `retainedVariance` | PCA | 0.9–0.99 | 高→保留更多信息 |

#### 关联与对比

- 表格数据 → RTrees/Boost 优先；高维稀疏 → 线性 SVM/LogReg；图像特征 → HOG+SVM 或 dnn。
- 嵌入式部署：线性 SVM/LogReg/PCA 模型文件小（KB 级）、预测快（μs 级），适合 ARM/Cortex。
- 需要概率输出 → LogisticRegression 或 Platt 平滑后的 SVM；需要特征重要性 → RTrees `getVarImportance`。

#### 注意事项

- **务必标准化特征**再喂 SVM/KNN/PCA——量纲不一致是最大坑。
- `TrainData` 的 `varType` 最后一位必须是响应列类型，分类任务设 `VAR_CATEGORICAL`。
- SVM RBF 核的 `gamma` 默认 `1/numFeatures`，往往不合适，需手动 `setGamma()`。
- `kmeans` 的 `K` 选择用肘部法则或轮廓系数，不要拍脑袋。
- `ANN_MLP` 已被 `dnn` 模块取代，新项目建议直接走 `dnn`。

#### 应用场景

- 数字/字符识别：HOG + 线性 SVM（`digits_svm` 模式）。
- 行人/物体检测：HOG + 线性 SVM 导出检测器（`train_HOG` 模式）。
- 图像聚类/主色提取：k-means。
- 人脸识别预处理：PCA 降维 + 最近邻。
- 流式数据在线分类：SVMSGD。
- 表格数据分类/回归：RTrees/Boost（自带特征重要性）。
