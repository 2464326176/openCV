# 常见问题与排障 FAQ（25+ 条）

> 按"编译链接 / 运行时 / 算法调参 / 数值结果异常"四类组织。
> 基础 10 条也见 [README §7](../README.md#7-常见坑--faq10-条踩过的坑)，本文件是其扩展版。

---

## A. 编译 / 链接（Build & Link）

### A1. `cmake` 报错找不到 OpenCVConfig.cmake
**现象**：`CMake Error at CMakeLists.txt: find_package(OpenCV REQUIRED)` 失败。
**根因**：`OpenCV_DIR` 没指向 `mingw-build/x64/mingw/staticlib`（根 `CMakeLists.txt`
已内建该路径，通常是你自己改了）。
**解决**：确认 `mingw-build/x64/mingw/staticlib/OpenCVConfig.cmake` 存在；手动指定
`-DOpenCV_DIR=...mingw-build/x64/mingw/staticlib`。

### A2. 链接一大堆 `undefined reference to cv::xxx`
**现象**：编译过、链接失败，一堆 OpenCV 符号未定义。
**根因**：用的是**静态库**（`mingw-build/.../staticlib`），需要 `-lopencv_world` 之外还要
`libpng / libjpeg / zlib` 等依赖，且链接顺序敏感。
**解决**：项目已用 CMake 的 `target_link_libraries(x PRIVATE ${OpenCV_LIBS})`，正常不会缺。
若手动写 Makefile，把 `${OpenCV_LIBS}` 放最后，依赖库顺序：OpenCV → 第三方 → 系统库。

### A3. 想用 SURF / SIFT / `xfeatures2d` 编译报 "has no member"
**现象**：`cv::xfeatures2d::SIFT::create` 报找不到。
**根因**：这些在 **opencv_contrib** 里，预编译的 `mingw-build/` 没编进来。
**解决**：要么只用主库检测器（ORB/BRISK/AKAZE/FAST，本项目 `feature_detection` 就是这么做的）；
要么自己用 contrib 源码重编 OpenCV 并设 `OPENCV_EXTRA_MODULES_PATH`。

### A4. Debevec HDR 在 OpenCV 4.13 崩断言
**现象**：`cv::CalibrateCRF` 触发断言。
**根因**：BGR 通道 `reshape` 的已知 bug。
**解决**：**绕路用 Mertens 曝光融合**（`MertensMerge`，不需要 CRF，实测稳）；或把 BGR 拆成
单通道逐通道跑再 Merge。

### A5. `build.ps1` 报 `cmake.exe 不在 PATH`
**现象**：脚本直接退出。
**根因**：MinGW / CMake 没加进 PATH。
**解决**：把 CMake 的 `bin/` 和 MinGW 的 `bin/` 加进系统 PATH；或确认 `mingw32-make` 可用
（MinGW 生成器需要它）。

### A6. 编译极慢 / 卡死
**现象**：`cmake --build` 跑很久。
**根因**：`algorithms/common` 是静态库，`algo_utils.cpp` 含 `niqeApprox` 等重实现，单文件重编代价高。
**解决**：只编需要的模块 `.\build.ps1 -Target algorithms -Module "hdr;beauty"`，避免 `ALL` 全量。

### A7. Windows 上大小写重命名失败 / 文件名冲突
**现象**：把 `Foo.cpp` 改成 `foo.cpp` 后内容变成另一个文件。
**根因**：NTFS 大小写不敏感。
**解决**：走**中转名两步** `Foo → _tmp → foo`，不要一步改大小写。

---

## B. 运行时（Runtime）

### B1. 窗口一闪而过
**根因**：HighGUI 没有事件循环。
**解决**：GUI 代码末尾必须 `waitKey(>0)`，且 `cv::Mat` 窗口对象别先析构。

### B2. `*.exe` 运行找不到 data 图片
**现象**：`imread` 返回空，demo 用合成图兜底。
**根因**：CWD 不是 exe 所在目录，相对路径 `../../data` 解析错。
**解决**：`cd` 到 exe 目录再启动；或直接用 `common/opencv_utils.h` 的 `getImagePath()`
（自带回退 CWD→上级→上上级）。

### B3. 视频类 demo（`optical_flow`）没输出
**现象**：`data/vtest.avi` 打不开。
**根因**：OpenCV 静态库没编 FFMPEG 后端，`VideoCapture` 读不了 avi。
**解决**：会走合成运动兜底分支（看 stdout 的 log）。要真读视频需 contrib 的 ffmpeg。

### B4. `out/algorithms/` 写入失败
**现象**：demo 报"无法创建目录/写文件"。
**根因**：`../out/algorithms` 不存在且 `ensureDir` 权限不够。
**解决**：代码里有 `ensureDir("../out/algorithms")`，一般自动建；CI/服务器无写权限时手动 `mkdir`。

### B5. exe 体积巨大（每个 22 MB+）
**根因**：静态链接 + 没开 strip。
**解决**：Debug 才这么大；Release 用 `cv::buildInformation` 检查，发布前 `strip *.exe`。
项目自带的 exe 是调试遗留，建议 `build.ps1 -Action clean` 后重新 Release 构建。

### B6. 中文路径 / 中文文件名读不出
**现象**：`imread("测试图.jpg")` 返回空。
**根因**：MinGW 下 `std::string` 路径的编码（GBK vs UTF-8）与 OpenCV 内部 `fopen` 不一致。
**解决**：用 `cv::imread(cv::String(utf8))`；或把数据放纯 ASCII 路径（当前 `data/` 命名已是 ASCII）。

### B7. `out/` 被 CMake 缓存污染导致清理误删
**现象**：`out/` 里混入 `CMakeCache.txt` / `Makefile`。
**根因**：曾用 `cmake -B out` 把构建目录设成 `out/`。**`out/` 是算法输出目录，绝不该作构建目录。**
**解决**：构建目录用 `build/algorithms` 这类独立名；`build.ps1 -Action clean -Mode out` 会清掉 PNG 以外的缓存。
（本项目已清理过一次，见提交记录。）

---

## C. 算法调参（Algorithm Tuning）

### C1. `Canny` 边缘碎成片
**解决**：先 `GaussianBlur(Size(5,5), 1.4)`；双阈值高低比 2:1~3:1，例 `Canny(g, c, 100, 200)`。

### C2. `GaussianBlur` 不平滑 / 过度模糊
**解决**：核大小奇数；σ 留 0 让函数按核算；要强平滑用 `Size(15,15)` + σ=4。

### C3. 双边滤波太慢
**解决**：`d`（直径）从 9 降到 5；`σ_color`/`σ_space` 别给太大；单帧可接受，视频别用。

### C4. 形态学开运算把主体也吃没了
**解决**：核太大或 mask 前景本就细。换 `MORPH_CROSS` 或减小 `ksize`。

### C5. 闭运算填不上洞
**解决**：洞比核大。核大小必须**大于**待补缝隙宽度。

### C6. 阈值分割一团糟（光照不均）
**解决**：Otsu 只在直方图双峰时有效；光照不均用 `ADAPTIVE_THRESH_GAUSSIAN_C`，`blockSize` 取奇数。

### C7. Watershed 过分割（红线密密麻麻）
**解决**：前景标记太碎。调大距离变换阈值系数（示例 `0.4 * norm(dist)`）；
先做形态学开运算再求标记。

### C8. GrabCut 抠图缺一块 / 吞背景
**解决**：`rect` 要完整包住目标且留一圈背景；迭代 5 次通常够；交互式用鼠标笔刷更准。

### C9. 模板匹配位置乱（CCORR 尤其明显）
**解决**：**别用裸 `TM_CCORR`**（倾向匹配最亮区）。用 `TM_CCOEFF_NORMED` / `TM_SQDIFF_NORMED`。
注意 SQDIFF 系列是"越小越好"。

### C10. 立体匹配 `valid` 占比低（大量灰）
**解决**：图对未校正 → 先立体校正；或 `numDisparities=64` 不够（近景视差超范围）调大。

### C11. 光流 `tracked` 比率低
**解决**：帧间隔太大 → 换相邻帧或加大 `maxLevel`；纹理不足 → 降低 `qualityLevel` 到 0.001。

### C12. 霍夫直线检出几百条 / 一条都检不出
**解决**：`threshold` 太低→调高（120/150）；完全检不出→Canny 阈值太高或 `minLineLength` 太小。

### C13. 霍夫圆检出假圆一大片
**解决**：`param2`（累加器阈值）太低→调高到 40+；先 `GaussianBlur(9,3)`；给 `minRadius/maxRadius`。

---

## D. 数值结果异常（Metrics & Numbers）

### D1. PSNR 异常高（>60 dB）或为负
**根因**：两图尺寸/通道不一致，`psnr` 实现里做了 `min(rows)` 之类，对齐错位。
**解决**：确保比较的是同尺寸同通道；本项目 `psnr` 已做通道对齐，检查输入是否意外被 resize。

### D2. SSIM 算出来是 1.0（可疑）
**根因**：两张图几乎一样（如输入就是参考），或实现里做了整体减均值后尺度消失。
**解决**：确认参考图真的是"干净原图"而非"同一张图"。

### D3. 去模糊 `Inverse(bare)` 全图噪点
**根因**：逆滤波在 H 接近 0 处放大噪声。
**解决**：用 **Wiener**（带 K 正则）或 **RL**；逆滤波只用于无噪理论演示。

### D4. 去模糊 `Unsharp` 的 PSNR 反而低于模糊图
**根因**：锐化是"增强"不是"复原"，离原图更远但看着更清楚。
**解决**：这是预期行为，说明 PSNR 不反映主观质量，看 SSIM 或肉眼。

### D5. 修复 `wholePSNR` 很高但看着没修好
**根因**：遮挡只占 ~3%，整图 PSNR 被未受损区稀释。
**解决**：**看 `maskMAE`**（仅遮挡区误差），不要信 wholePSNR。

### D6. 频域滤波"没生效"或"图像被打散成条纹"
**根因**：掩膜布局与 `dft` 输出未对齐（象限交换漏做）。
**解决**：构造中心化掩膜后必须 `swapQuadrants` 转回未移位布局，再与 `F` 相乘。

### D7. 频域 `notch` 去噪后还有残纹
**根因**：陷波半径太小或位置算错（与真实噪声频率不符）。
**解决**：检查 `fx/fy`；把 `±3` 邻域调大；噪声频率 = `1/周期(px)`。

### D8. 多帧降噪 `varianceWeighted` 比 `mean` 还糊
**根因**：帧间有残余错位，方差加权把错位当噪声压制。
**解决**：先确认配准对齐（看对齐误差热图）；错位大时改用 `median` 更鲁棒。

### D9. 特征匹配 `ratio` 内点率 < 30%
**根因**：两图非平面/剧烈 3D 旋转，单应模型本身不成立。
**解决**：换 `findEssentialMat`/`recoverPose`；或降低 Lowe 比例到 0.6。

### D10. 指标每次跑都不一样
**根因**：部分算法用 RNG（随机初始化 KMeans、合成图、噪声）。
**解决**：固定 `RNG` 种子（本项目 `inpaint` 已用 `RNG(12345)` 保证可复现）；对比时同种子同输入。

> 仍有问题？先看对应模块的 `README.md` "结果怎么读" 小节，90% 的判读问题那里有解。
