# L0 环境与显示

**目标**：跑通 OpenCV 程序的最小骨架——读图、显示、滑动条、鼠标、绘图、视频、通道拆合。

**黄金主线**：07 → 01 → 04 → 02 → 03 → 06 → 08 → 05

**验收**：能写出「读图 → 灰度 → 滑动条调亮度 → 鼠标框选 ROI → 保存」的小工具。

## 核心概念速览

| 主题 | 关键词 | 心智模型 |
| --- | --- | --- |
| 图像读写 | imread/imshow/waitKey | 解码为 Mat→推入窗口→waitKey 刷新事件 |
| 窗口管理 | namedWindow/WINDOW_NORMAL | 控制窗口是否可缩放、是否固定尺寸 |
| Trackbar | createTrackbar/回调 | 滑条值双向绑定变量，变化触发回调重算 |
| 鼠标交互 | setMouseCallback/事件 | 回调拿到 event+x+y，实现 ROI 框选/标注 |
| 绘图 | line/circle/putText | 原地写入 Mat 像素，BGR 顺序 |
| 通道拆合 | split/merge | BGR 三通道分离/合并，OpenCV 默认 BGR 非 RGB |
| 视频 I/O | VideoCapture/VideoWriter | 帧序列读写，需指定 FourCC 编码器 |
| 环境检查 | CV_VERSION/getBuildInformation | 确认链接的 OpenCV 版本与模块开关 |

## 官方对照表

| 练习文件 | 标签 | 官方 sample | docs | 必会 API |
| - | - | - | - | - |
| [07_opencv_version.cpp](07_opencv_version.cpp) | 主线 | `opencv_version.cpp` | ch01 §1.0 | `CV_VERSION` / `getBuildInformation` |
| [01_hello_imread.cpp](01_hello_imread.cpp) | 主线 | `tutorial_code/introduction/display_image/display_image.cpp` | ch01 §1.1 | `imread` / `imshow` / `waitKey` |
| [04_drawing_primitives.cpp](04_drawing_primitives.cpp) | 主线 | `drawing.cpp` | ch08 + ch01 | `line` / `circle` / `putText` |
| [02_named_window_trackbar.cpp](02_named_window_trackbar.cpp) | 主线 | `AddingImagesTrackbar.cpp` | ch08 | `createTrackbar` / `convertScaleAbs` |
| [03_mouse_roi.cpp](03_mouse_roi.cpp) | 进阶 | `drawing.cpp`（鼠标） | ch08 | `setMouseCallback` / ROI |
| [06_split_merge.cpp](06_split_merge.cpp) | 主线 | `snippets/core_split.cpp` | ch01 | `split` / `merge` |
| [08_videowriter.cpp](08_videowriter.cpp) | 主线 | `videowriter_basic.cpp` | ch08 §videoio | `VideoWriter` / `VideoCapture` |
| [05_videocapture_camera.cpp](05_videocapture_camera.cpp) | 依赖设备 | `videocapture_camera.cpp` | ch08 | `VideoCapture`；无相机回退静态图 |

## 关键易错点

| 练习 | 易错点 / 关键参数 |
| --- | --- |
| 01 读图 | 忘 `img.empty()` 判空；忘 `waitKey` 窗口一闪而过 |
| 02 滑条 | `createTrackbar` 后需手动调一次回调做首帧渲染 |
| 03 鼠标 | 框选 ROI 时注意鼠标按下/抬起两事件配对；坐标是 (x,y) 非 (y,x) |
| 04 绘图 | `Scalar(B,G,R)`；`thickness=FILLED`(-1) 才填充封闭图形 |
| 05 相机 | `VideoCapture::open` 返回 false 要降级；`CAP_PROP_*` 读帧宽高 |
| 06 通道 | `split` 会分配 3 个新 Mat；只取单通道用 `extractChannel` 更省 |
| 07 版本 | `getBuildInformation` 返回字符串含编译时间/模块/依赖，用于排查缺模块 |
| 08 写视频 | `VideoWriter` 的 `fourCC`/`fps`/`Size` 必须与帧一致，否则写出空文件 |

## 说明

- `07` 先确认 OpenCV 版本与 data/models 路径是否可用。
- `05` 摄像头失败时回退静态图；`08` 写出 AVI 并回读验证帧数。
