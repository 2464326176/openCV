# learn — OpenCV 分层速学练习树

本目录是与 [`docs/`](../docs/) 对齐的独立练习树，**不复制官方 233 个 sample 源码**，而是为每个主题写 40～80 行可运行最小版，文件头注明官方路径与理论章节锚点。

## 设计原则

1. **不动旧代码**：根目录已有的 `image_process/`、`face_detect/`、`histogram_match/` 等 legacy 实验目录不修改、不扩展。
2. **复用 units**：通过 [`../common/opencv_utils.h`](../common/opencv_utils.h) 的 `getImagePath` / `getModelPath` / `dbgShow` 访问 [`../data`](../data) 与 [`../models`](../models)，无图片时自动合成测试图。
3. **单题独立**：每个 `.cpp` 都有 `int main()`，独立编译为 `learn_<stem>` 可执行文件。
4. **能跑能看能改**：错误路径只报 `empty()`；参数尽量暴露成滑动条或常量。

## 学习层次

| 层 | 目录 | 主题 | 对应 docs | 建议节奏 |
| - | - | - | - | - |
| L0 | [L0_intro](L0_intro/README.md) | 环境与显示 | ch01 §1 + ch08 入门 | 1 天 |
| L1 | [L1_core](L1_core/README.md) | Mat 与像素 | ch01 §2～ | 2～3 天 |
| L2 | [L2_imgproc](L2_imgproc/README.md) | imgproc 主干 | ch02 全章 | 1 周 |
| L3 | [L3_features_video](L3_features_video/README.md) | 结构与运动 | ch03 + ch04 | 3～5 天 |
| L4 | [L4_detect_calib](L4_detect_calib/README.md) | 检测摄影三维 | ch06 + ch07 | 按需 |
| L5 | [L5_ml_gapi](L5_ml_gapi/README.md) | ML/GAPI/GPU 选修 | ch05 + ch08 | 选修 |

## 黄金主线（零基础 1 周计划）

| 天 | 路径 | 标签 |
| - | - | - |
| 1 | L0: 07 → 01 → 04 → 02 → 03 → 06 → 08 → 05 | 主线（5 依赖摄像头） |
| 2～3 | L1: 01 → 02 → 04 → 07 → 03 | 主线 |
| 4～5 | L2: 01 → 08 → 13 → 10 → 14 → 26 → 30 → 32 | 主线（2 相位相关为进阶） |
| 按需 | L3: 02 → 05 → 09 → 10 → 17 | 主线 |
| 按需 | L4: 01 → 05 → 07 → 12 | 主线 |

**标签说明**：`主线` 必做；`进阶` 加深理解；`选修` 可跳过；`依赖设备` 无硬件时有降级路径。

## 编译方法

完整命令见 [根 README §3.3](../README.md#33-等价原生-cmake)。等价最简：

```bash
cmake -B build/learn -G "MinGW Makefiles" -DBUILD_LEARN=ON -DLEARN_LAYER=L0 ..
cmake --build build/learn -j
```

`LEARN_LAYER`：`ALL`（默认）或 `L0`…`L5` 单层。`CMake` 注入 `LEARN_PROJECT_ROOT` / `LEARN_DATA_ROOT` / `LEARN_MODELS_ROOT`，减少 CWD 依赖。

## 与 docs / 官方 sample 的关系

- 阅读链：`principles.md` → `ch0X` → **本目录练习** → `mingw-build/samples/cpp` 官方源码
- 同一主题若官方有多个 demo，**只生成一个练习文件**，README 列出全部官方对照路径
- SURF/SIFT/LATCH、RealSense、CUDA 等**可编译降级**，README 标明跳过原因

## 验收标准

- `BUILD_LEARN=ON` 全层编译通过
- 每个习题产生可视化或控制台输出，参数可调
- 学习者能脱稿写出该层核心管线
