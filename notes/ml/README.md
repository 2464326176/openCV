# 机器学习：SVM 支持向量机（ml）

本节用两个可运行示例讲解 OpenCV `ml` 模块的**支持向量机（SVM）**：线性可分数据的最优分类面，以及用 RBF 核处理线性不可分数据。对应官方示例 [introduction_to_svm.cpp](../../mingw-build/samples/cpp/tutorial_code/ml/introduction_to_svm/introduction_to_svm.cpp)、[non_linear_svms.cpp](../../mingw-build/samples/cpp/tutorial_code/ml/non_linear_svms/non_linear_svms.cpp)。

本目录源码：[svm.cpp](svm.cpp)、[svm_rbf.cpp](svm_rbf.cpp)。

## 1. 章节文件索引

| 文件 | 主题 |
|------|------|
| [svm.cpp](svm.cpp) | 线性 SVM 二分类 + 决策边界可视化 |
| [svm_rbf.cpp](svm_rbf.cpp) | RBF 核处理线性不可分数据 |

## 2. 线性可分 SVM

### 2.1 思想

对两类样本，存在无穷多个分隔直线。SVM 选择**间隔最大**的那条：让离分类面最近的样本（**支持向量**）到分类面的距离最大化。几何间隔越大，泛化能力越强。

$$
\max_{w,b} \frac{2}{\|w\|} \quad \text{s.t.} \quad y_i(w\cdot x_i + b) \ge 1
$$

间隔最大等价于最小化 $\frac{1}{2}\|w\|^2$，是一个凸二次规划问题。

### 2.2 代码解读

来自 [svm.cpp](svm.cpp)：

```cpp
Ptr<SVM> svm = SVM::create();
svm->setType(SVM::C_SVC);        // C-SVC 分类器
svm->setKernel(SVM::LINEAR);     // 线性核
svm->setC(1.0);                  // 正则化参数 C
svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

svm->train(trainData, ROW_SAMPLE, labelsMat);   // ROW_SAMPLE: 每行一个样本
```

| 参数 | 含义 | 调大/调小 |
|------|------|-----------|
| `setType` | 分类器类型 | `C_SVC` 最常用；`NU_SVC` 用 ν 控制支持向量比例 |
| `setKernel` | 核函数 | LINEAR / RBF / POLY / SIGMOID |
| `setC` | 正则化惩罚 | 越大越硬（少误分但可能过拟合），越小越软 |
| `ROW_SAMPLE` | 样本排布 | 每行一个样本；`COL_SAMPLE` 每列一个 |

**预测并可视化决策面**：

```cpp
Mat sample = (Mat_<float>(1, 2) << j, i);
float response = svm->predict(sample);   // 返回预测类别
```

**支持向量**是真正决定分类面的少数样本（其余样本即使删掉也不影响结果）：

```cpp
Mat sv = svm->getUncompressedSupportVectors();
```

## 3. 非线性 SVM：核技巧

当数据线性不可分（如两类交错分布）时，用**核函数**把样本隐式映射到高维空间，使其在高维线性可分，而计算仍留在低维（核技巧）。

RBF 高斯核：

$$
K(x, y) = \exp(-\gamma \|x - y\|^2)
$$

来自 [svm_rbf.cpp](svm_rbf.cpp)：

```cpp
svm->setKernel(SVM::RBF);
svm->setGamma(0.5);   // γ：核宽度，越大边界越"弯曲"（越容易过拟合）
```

| 核 | 表达式 | 适用 |
|----|--------|------|
| LINEAR | $x \cdot y$ | 线性可分 |
| RBF | $\exp(-\gamma\|x-y\|^2)$ | 最常用，非线性 |
| POLY | $(\gamma x\cdot y + r)^d$ | 多项式边界 |
| SIGMOID | $\tanh(\gamma x\cdot y + r)$ | 类神经网络 |

## 4. OpenCV ML 模块其他算法

`cv::ml` 还包含：

- **KNN**（`KNearest`）：最近邻分类，适合小样本。
- **决策树/随机森林**（`DTrees` / `RTrees`）：可解释性好。
- **逻辑回归**（`LogisticRegression`）。
- **PCA**（`PCA`，在 `core` 模块）：降维与数据压缩，常用于特征工程。

## 5. 典型应用场景

- **手写数字识别**：HOG/像素特征 + SVM。
- **目标分类**：特征描述子（ORB/SIFT）+ SVM。
- **异常检测**：一类 SVM。
- **回归预测**：SVR（`setType` 选 `EPS_SVR`）。

## 6. 相关官方示例

- [introduction_to_svm.cpp](../../mingw-build/samples/cpp/tutorial_code/ml/introduction_to_svm/introduction_to_svm.cpp)：线性 SVM
- [non_linear_svms.cpp](../../mingw-build/samples/cpp/tutorial_code/ml/non_linear_svms/non_linear_svms.cpp)：非线性 SVM
- [introduction_to_pca.cpp](../../mingw-build/samples/cpp/tutorial_code/ml/introduction_to_pca/introduction_to_pca.cpp)：PCA 降维
