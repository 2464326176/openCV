// algorithms/common/algo_utils.hpp
// 算法公共工具: 评估指标 (PSNR/SSIM/MAE/MSE/LOE/NIQE 框架), 输入归一化/分块,
// 结果可视化 (水平+垂直拼接), 色彩统计, 直方图增强, ECC 配准等.
// 所有算法模块共享本头文件, 保持 API 一致.
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/photo.hpp>
#include <string>
#include <vector>
#include <map>

namespace algo {

// =========================================================================
// ---- 数值范围 / 类型转换 -------------------------------------------------
// =========================================================================

// 把 8UC[1/3] 转 32FC[1/3] 并归一化到 [0,1].
cv::Mat toFloat(const cv::Mat& src);
// 把 [0,1] 浮点图裁剪到 [0,1] 并转回 8UC, 保持通道数.
cv::Mat to8U(const cv::Mat& src, double scale = 255.0);
// 把任意浮点图线性归一化到 [0,1] (用于可视化 float32 HDR/Retinex 等).
cv::Mat normalizeTo01(const cv::Mat& src);

// =========================================================================
// ---- 全参考图像质量评估 (Full-Reference IQA) ------------------------------
// =========================================================================

// 均方误差 MSE = mean( (a-b)^2 ), 越低越好.
double mse(const cv::Mat& a, const cv::Mat& b);
// 平均绝对误差 MAE = mean( |a-b| ), 对异常值比 MSE 更鲁棒.
double mae(const cv::Mat& a, const cv::Mat& b);
// 峰值信噪比, 8UC1/3 通用. 返回 dB. 完全相同返回 1000 dB.
double psnr(const cv::Mat& a, const cv::Mat& b);
// 简化 SSIM: 单通道 8U, 越接近 1 越好. 多通道取均值.
double ssim(const cv::Mat& a, const cv::Mat& b);
// MS-SSIM 框架 (3 层 3× downsample, 用简化 SSIM 加权: luminance+contrast+structure).
// 典型权重: luminance=0.0448, contrast=0.2856, structure=0.1333 三层.
double msssim(const cv::Mat& a, const cv::Mat& b, int levels = 3);
// 亮度顺序误差 LOE (Lightness Order Error, Wang & Bovik):
// 衡量增强前后局部亮度排序被保留的比例, LOE 越小色彩越自然.
// 采样像素点数 samples=32~64 足够获得稳定估计.
int loe(const cv::Mat& before, const cv::Mat& after, int samples = 48);

// =========================================================================
// ---- 无参考图像质量评估 (No-Reference IQA) 框架 --------------------------
// =========================================================================

// 亮度统计 {mean, std, per95, per05}: mean 代表整体亮度,
// std 代表对比, per95/per05 代表最亮 5%/最暗 5% 分位.
struct BrightStats { double mean, std, per95, per05; };
BrightStats brightnessStats(const cv::Mat& bgr);
// 色彩饱和度 (均值) 越大越鲜艳: sat_mean = mean( sqrt(Cr²+Cb²) ) / 128.
double saturationMean(const cv::Mat& bgr);
// 信息熵 H = -Σ p·log2(p), 越高代表信息越丰富, 单位 bit/像素.
double imageEntropy(const cv::Mat& gray);
// EME (Measure of Enhancement): EME = (1/K²)·Σ 20·log(max_d/min_d+ε),
// 对 8×8 分块, 越大代表增强越强 (过高表示过度增强).
double eme(const cv::Mat& gray, int blk = 8);
// NIQE 自然图像质量评估框架的简化近似 (仅基于局部统计特征的方差),
// 返回无参考质量得分 (越小越好, 0 = 理想自然图像).
double niqeScoreApprox(const cv::Mat& gray);

// 把多个 NR-IQA 指标组合成 {entropy, sat, eme, niqe}, 方便一行打印.
struct NRQuality { double entropy; double saturation; double eme; double niqeApprox; };
NRQuality nrQuality(const cv::Mat& bgr);

// =========================================================================
// ---- 图像增强工具 (供 HDR / night_scene / beauty 复用) --------------------
// =========================================================================

// LUT 驱动的 Gamma 校正: out = LUT[I], LUT[i] = pow(i/255, gamma) * 255.
cv::Mat gammaLUT(const cv::Mat& src, double gamma);
// 全局自动亮度: 基于 1% 和 99% 分位做线性拉伸, 避免极端 outliers.
cv::Mat autoContrast1pct(const cv::Mat& bgr);
// 直方图均衡化 (BGR: 只对 Y 通道均衡, 防止色偏).
cv::Mat equalizeHistogramY(const cv::Mat& bgr);
// Simple Color Balance: per-channel 1% low/high percentile stretch.
// 校正偏色的同时提亮, 对 RAW->sRGB 后色偏很有效.
cv::Mat simpleColorBalance(const cv::Mat& bgr, double pct = 0.01);
// 白点检测 (Gray-world + MaxRGB 简化): 估计白点 RGB, 返回 (R,G,B) ∈ [0,1].
cv::Vec3f estimateWhitePointGrayWorld(const cv::Mat& bgr);
// 基于白点估计的简单白平衡: out_c = in_c * (1/white_c) * scale.
cv::Mat whiteBalanceFromPoint(const cv::Mat& bgr, const cv::Vec3f& white,
                               double clip = 2.0);

// =========================================================================
// ---- 可视化 ---------------------------------------------------------------
// =========================================================================

// 把多张同尺寸 BGR/灰度图水平拼接成一张, 各自带标题.
cv::Mat hstackWithLabels(const std::vector<cv::Mat>& imgs,
                         const std::vector<std::string>& labels,
                         int labelHeight = 30);
// 把多张同尺寸 BGR/灰度图按列数 grid 拼接: cols=3 即 3 列多行.
cv::Mat gridWithLabels(const std::vector<cv::Mat>& imgs,
                        const std::vector<std::string>& labels,
                        int cols = 3, int labelHeight = 30);
// 在图下方绘制一张伪彩热力图 (输入单通道 8U), 带 60px 标题栏.
cv::Mat attachHeatmapBelow(const cv::Mat& top, const cv::Mat& grayMap,
                            const std::string& label);
// 把图像缩放到固定最大边再 imshow, 适合调试大图.
void imshowFit(const std::string& win, const cv::Mat& img,
               int maxEdge = 1024, int delay = 0);
// 简易进度/状态打印 (带时间戳).
void log(const std::string& tag, const std::string& msg);
// 把一张 FR-IQA 对比表 (多行 "method" → (p, s)) 格式化为多行字符串.
std::string formatFRIQATable(
    const std::vector<std::pair<std::string, std::pair<double, double>>>& rows);
// 把一张 NR-IQA 对比表格式化为多行字符串.
std::string formatNRIQATable(
    const std::vector<std::pair<std::string, NRQuality>>& rows);

// =========================================================================
// ---- 配准 (单应 / 仿射 / 特征点) ------------------------------------------
// =========================================================================

// 用 ECC 高精度迭代配准把 src 对齐到 ref, 返回单应/仿射矩阵 (默认 affine).
// 失败返回空 Mat. 支持 MOTION_TRANSLATION / MOTION_EUCLIDEAN / MOTION_AFFINE /
// MOTION_HOMOGRAPHY. 对于曝光差异大的序列, 会自动对灰度做 CLAHE 预处理.
cv::Mat alignECC(const cv::Mat& ref, const cv::Mat& src,
                 int motionType = 1 /*cv::MOTION_AFFINE*/, int iters = 50,
                 double eps = 1e-6, bool preCLAHE = true);
// 配准并直接返回对齐后的 warp 图.
cv::Mat alignToRef(const cv::Mat& ref, const cv::Mat& src,
                   int motionType = 1, int iters = 50, double eps = 1e-6,
                   bool preCLAHE = true);
// 粗粒度亮度匹配: 把 src 的 mean/std 对齐到 ref 的 mean/std (Y 通道 only,
// 防止色偏). 用于多帧 / HDR 输入的预处理.
cv::Mat matchLuminanceStats(const cv::Mat& ref, const cv::Mat& src);

// =========================================================================
// ---- 文件工具 & 目录迭代 --------------------------------------------------
// =========================================================================

// 拼接路径 (跨平台.
std::string join(const std::string& a, const std::string& b);
// 确保目录存在, 若不存在则创建 (递归). 创建失败返回 false 但不抛异常.
bool ensureDir(const std::string& dir);
// 列出目录下的所有文件 (不递归), 按扩展名过滤 (大小写不敏感, 不填则全部).
std::vector<std::string> listFiles(const std::string& dir,
                                    const std::string& extFilter = "");
// 获得文件的不含路径扩展名, 如 "foo/bar.png" → "bar".
std::string baseNameNoExt(const std::string& path);

// =========================================================================
// ---- 分块迭代 (用于逐块处理/评估, 便于支持超大图) -------------------------
// =========================================================================

// 把图像切成 blk 大小的有重叠 tile, 对每个 tile 调用 fn(tile, bx, by, isLastX, isLastY).
// overlap 设为配准半径/降噪半径 可消除 tile 边界伪影.
void processTiled(const cv::Mat& src, cv::Mat& dst, int blk, int overlap,
                  std::function<cv::Mat(const cv::Mat&)> fn);

} // namespace algo
