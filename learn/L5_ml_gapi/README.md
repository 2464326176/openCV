# L5 选修：ML / G-API / GPU

**目标**：理解 OpenCV 内置机器学习算法（聚类/分类/降维/概率/神经网络/集成/回归），掌握 G-API 声明式管线思想与 GPU 加速入口。本章为选修，重在「算法选型与 API 语义」而非训练大模型。

**黄金主线**：01 → 02 → 04 → 13 → 14 → 11 → 15

**建议顺序**：01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 → 10 → 11 → 12 → 13 → 14 → 15

**验收**：能用 k-means 做颜色量化；能训一个线性 SVM 并预测；能用 PCA 降维可视化；能说清「SVM 核函数怎么选」；能写一个 G-API 声明式管线并理解它为何比命令式更快/更省内存。

## 核心概念速览

| 主题 | 关键词 | 心智模型 |
| --- | --- | --- |
| 聚类 | kmeans/K/距离/初始化 | 无监督分 K 组，迭代更新质心 |
| SVM | 间隔/核/支持向量/软间隔 | 找最大间隔超平面，核函数升维 |
| 降维 | PCA/协方差/特征值/投影 | 保留方差最大方向，去相关 |
| 概率模型 | EM/GMM/隐变量 | 似然最大化，E 步估后验 M 步更新参数 |
| 神经网络 | ANN-MLP/前向/激活/反向 | 加权求和 + 非线性激活，梯度下降 |
| 树模型 | 决策树/信息增益/剪枝 | 递归选最优分裂特征 |
| 集成 | Boosting/AdaBoost/弱学习器 | 串行训练，错分样本加权 |
| 回归 | Logistic/SGD/对率 | 线性+sigmoid，梯度下降在线学习 |
| DNN 桥梁 | dnn/blob/forward | 从 ml 传统模型过渡到深度前向 |
| G-API | 图/内核/编译/流水线 | 声明式构图，编译期融合优化 |
| GPU | UMat/cuda/Stream | 异构加速，UMat 隐式 OpenCL |

## 官方对照表

| 练习文件 | 标签 | 官方 sample | tutorialDoc | 必会 API |
| - | - | - | - | - |
| [01_kmeans.cpp](01_kmeans.cpp) | 主线 | `kmeans.cpp` | ch05 §聚类 | `kmeans`（`KMEANS_PP_CENTERS`） |
| [02_svm_intro.cpp](02_svm_intro.cpp) | 主线 | `introduction_to_svm.cpp` | ch05 §SVM | `SVM::create` / `train` / `predict` |
| [03_svm_nonlinear.cpp](03_svm_nonlinear.cpp) | 进阶 | `non_linear_svms.cpp` | ch05 §核 | `SVM::RBF` / `SVM::POLY` |
| [04_pca.cpp](04_pca.cpp) | 主线 | `introduction_to_pca.cpp` | ch05 §PCA | `PCA` / `project` / `backProject` |
| [05_em.cpp](05_em.cpp) | 进阶 | `em.cpp` | ch05 §EM | `EM::create` / `trainEM` |
| [06_ann_mlp.cpp](06_ann_mlp.cpp) | 进阶 | `neural_network.cpp` | ch05 §ANN | `ANN_MLP::create` / 层结构 |
| [07_decision_tree.cpp](07_decision_tree.cpp) | 选修 | `tree_engine.cpp` | ch05 §树 | `DTrees::create` |
| [08_boosting.cpp](08_boosting.cpp) | 选修 | `letter_recog.cpp`（Boost） | ch05 §集成 | `Boost::create` |
| [09_logistic_reg.cpp](09_logistic_reg.cpp) | 选修 | `logistic_regression.cpp` | ch05 §回归 | `LogisticRegression::create` |
| [10_svmsgd.cpp](10_svmsgd.cpp) | 选修 | `train_svmsgd.cpp` | ch05 §SVMSGD | `SVMSGD::create`（在线） |
| [11_gapi_blur_canny.cpp](11_gapi_blur_canny.cpp) | 主线 | `api_ref_snippets.cpp` | ch08 §G-API | `GMat` / `GaussianBlur` / `Canny` |
| [12_gpu_basic.cpp](12_gpu_basic.cpp) | 选修 | `gpu-basics-similarity.cpp` | ch08 §CUDA | `GpuMat` / `cuda::` 内核 |
| [13_digits_dnn.cpp](13_digits_dnn.cpp) | 进阶 | `digits_lenet.cpp` | ch05 §DNN 桥梁 | `dnn::blobFromImage` / `forward` |
| [14_hog_svm_train.cpp](14_hog_svm_train.cpp) | 进阶 | `train_HOG.cpp` | ch05 §HOG 训练 | `HOGDescriptor` / `svm->train` |
| [15_gapi_pipeline.cpp](15_gapi_pipeline.cpp) | 进阶 | `face_beautification.cpp`（简化） | ch08 §G-API 管线 | `GComputation` / 流水线构图 |

## 主题分组与先修关系

```
无监督（01,05）          ← 只需数据，不需标签
   │
监督分类（02,03,06-10）   ← 需特征+标签；02 是 SVM 入口
   │
降维（04）               ← 可作分类预处理（特征压缩）
   │
工程桥梁（13,14）        ← ml→dnn；HOG+SVM 完整训练链
   │
异构加速（11,12,15）     ← G-API/GPU，独立于算法本身
```

- **无监督**：01 k-means 做颜色/点聚类；05 EM 是概率版聚类（GMM），可与 01 对比。
- **监督分类**：02 SVM 线性是入口；03 扩展到非线性核；06-10 是其他分类器家族，理解选型差异即可。
- **降维**：04 PCA 既是预处理工具也是可视化手段，可与 13 的 MNIST 降维显示结合。
- **工程桥梁**：13 把 ml 的 SVM/数据与 dnn 的前向打通；14 是 HOG+SVM 的经典行人训练链。
- **异构加速**：11/15 是 G-API 声明式（编译期融合）；12 是 CUDA 入口（无 GPU 时打印说明）。

## 关键参数与易错点

| 练习 | 易错点 / 关键参数 |
| --- | --- |
| 01 kmeans | `K` 太大过拟合、太小欠分；`TermCriteria` 控制迭代停止；`KMEANS_PP_CENTERS` 比随机初始化稳定 |
| 02 SVM | `C`（惩罚参数）：大易过拟合、大欠拟合；`SVM::LINEAR` 适合线性可分 |
| 03 SVM 核 | `gamma` 越大局部长程越短（易过拟合）；RBF 通用，POLY 需调 degree |
| 04 PCA | `maxComponents` 决定保留维度；保留方差比 ≥95% 通常足够 |
| 05 EM | `nclusters` 需先验；协方差类型影响拟合能力 |
| 06 ANN | 层结构 `Mat(layerSizes)`；隐藏层太多易过拟合需早停 |
| 14 HOG+SVM | `winSize`/`blockSize`/`cellSize` 必须与检测时一致；正负样本需均衡 |
| 11 G-API | 内核参数在构图时固定；数据在 `GComputation::apply` 时才流入 |
| 12 GPU | `GpuMat` 不能直接 `imshow`，需 `download` 回 `Mat`；无 CUDA 时应优雅降级 |

## 资源与降级

- **数据**：ML 题用合成数据（随机点/合成数字），不依赖 `letter_recog` 大数据集。
- **GPU**：`12_gpu_basic` 无 CUDA 环境时打印说明并退出，不崩溃。
- **G-API**：`11`/`15` 纯 CPU 即可跑，G-API 不强制 OpenCL 后端。
- **DNN**：`13` 无 ONNX 模型时演示 `blobFromImage` 与张量形状 API 语义。

## 与官方/文档的关系

- 阅读链：`principles.md` §13.4 → `ch05` → **本目录练习** → `mingw-build/samples/cpp` 官方源码
- ML 主题见 [ch05_ml.md](../../docs/ch05_ml.md)；G-API/GPU 见 [ch08_gui_gapi_gpu.md](../../docs/ch08_gui_gapi_gpu.md)
- 同一主题若官方有多份 demo，**只生成一个练习文件**
- DNN 完整深度学习样例需 Model Zoo，本章仅以 `digits_lenet` 作桥梁

## 说明

- `02` 与 `03` 对比：线性 SVM vs 非线性核 SVM，体会「升维为何能线性可分」。
- `01` 与 `05` 对比：硬划分 k-means vs 软概率 GMM，理解概率聚类的后验含义。
- `11` 与 `15` 对比：单内核 G-API vs 多内核流水线，体会声明式构图如何让编译器融合内存传递。
- `13` 是 ml→dnn 的桥梁：用 ml 的 SVM 数据格式喂给 dnn 的 `blobFromImage`，理解传统 ML 与深度学习的接口差异。
- `14` 是 L4 `02_hog_pedestrian` 的「训练版」：那里用预训练检测器，这里从零训 SVM。
