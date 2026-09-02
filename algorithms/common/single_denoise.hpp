// algorithms/common/single_denoise.hpp
// 单帧降噪算法集合: Gaussian / Median / Bilateral / NLM / Guided
//                    + Wiener / WaveletSoftThreshold / AnisotropicDiffusion
//                    + AdaptiveSigmaBilateral.
//
// 多帧降噪 denoise_multi 也会复用 Gaussian/NLM 做对照.
// beauty 会复用 Bilateral + Guided 做磨皮。
// 本文件不依赖 opencv_contrib (xphoto / cudaimgproc), 便于裸编译。
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

namespace algo {

// ------------------------------------------------------------------ 基础

// 高斯滤波: sigma=0 时由 ksize 自动估算 (sigma = 0.3*((ksize-1)*0.5 - 1) + 0.8).
cv::Mat denoiseGaussian(const cv::Mat& src, int ksize = 5, double sigma = 0);

// 中值滤波: ksize 必须为奇数. 适合椒盐噪声, 保护阶跃边缘, 但损失细线。
cv::Mat denoiseMedian(const cv::Mat& src, int ksize = 5);

// 双边滤波: 边缘保持型平滑 (美颜 / 低照度降噪基线).
// 建议参数: d <= 9, sigmaColor=30~100, sigmaSpace=30~100.
cv::Mat denoiseBilateral(const cv::Mat& src, int d = 9,
                         double sigmaColor = 75, double sigmaSpace = 75);

// 引导滤波 (He 2010): 自引导滤波 (guide = src) 与 guided 外引导两版.
// 在保留边缘的同时去噪, 复杂度 O(N).
// eps 越小越保边; radius 越大平滑越强.
cv::Mat denoiseGuided(const cv::Mat& src, const cv::Mat& guide,
                       int radius = 8, double eps = 0.01);
inline cv::Mat denoiseGuided(const cv::Mat& src, int radius = 8,
                              double eps = 0.01) {
    return denoiseGuided(src, src, radius, eps);
}

// 非局部均值 NLM: h 越大平滑越强, templateSize 模板, searchSize 搜索半径.
cv::Mat denoiseNLM(const cv::Mat& srcBgr, float h = 10, int templateSize = 7,
                    int searchSize = 21);
// 灰度 NLM.
cv::Mat denoiseNLMGray(const cv::Mat& srcGray, float h = 10);

// ------------------------------------------------------------------ 进阶

// 自适应双边: sigmaColor 随局部标准差自适应调整.
//   - 在细节丰富的区域 (std 大) 降低 sigmaColor 保边
//   - 在平坦区域 (std 小) 提高 sigmaColor 去噪
// winRadius 为统计局部 std 的窗口半径 (默认 7).
cv::Mat denoiseAdaptiveBilateral(const cv::Mat& src, int d = 9,
                                  double baseSigmaColor = 75,
                                  double sigmaSpace = 75,
                                  int winRadius = 7);

// 维纳滤波 (Wiener, 局部 MMSE): out = mean + max(0, (localVar - noiseVar)/localVar)
//                                                                            · (I - mean)
// 假设加性高斯白噪声; noiseVar=0 时自动用 noiseVar ≈ mean(localVar).
cv::Mat denoiseWiener(const cv::Mat& src, int ksize = 5, double noiseVar = 0);

// 各向异性扩散 (Perona-Malik 1990):
// ∂I/∂t = div( c(||∇I||) · ∇I ), 用指数导电率 c(x)=exp(-(x/K)^2).
// 作用: 在平滑高斯噪声的同时保护强边缘.
// 典型参数: niters=20, K=15~30, dt=0.1 (对 8U 图像已做单位校准).
cv::Mat denoiseAnisotropicDiffusion(const cv::Mat& src, int niters = 20,
                                     double K = 20.0, double dt = 0.15);

// 软阈值小波型降噪的近似 (OpenCV 不带 DWT, 用 Haar-like 双线性上/下采样).
// 实现: 1) 构造 Laplacian 金字塔 L0...Ln;
//       2) 对高频层 Ln (差分) 做 soft threshold: sign(x)·max(0, |x|-th);
//       3) 重建.
// th = noiseStd · thresholdScale; noiseStd=0 时用最细层 MAD 估计.
cv::Mat denoiseLaplacianSoftThreshold(const cv::Mat& src, int levels = 4,
                                       double noiseStd = 0,
                                       double thresholdScale = 3.0);

// ------------------------------------------------------------------ 噪声合成 (供 denoise_single / denoise_multi demo 使用)

// 加性高斯噪声 N(0, sigma^2).
cv::Mat addGaussianNoise(const cv::Mat& src, double sigma, unsigned seed = 42);

// 椒盐噪声: p 概率盐化 (255), p 概率椒化 (0).
cv::Mat addSaltPepperNoise(const cv::Mat& src, double p, unsigned seed = 42);

// 乘性瑞利噪声 (超声 / MRI 模拟): out = I * rayleigh(scale), σ_noise ≈ scale·I.
cv::Mat addSpeckleNoise(const cv::Mat& src, double scale, unsigned seed = 42);

// 泊松噪声 (光子噪声): out ~ Poisson(α·I), 越小的 α 越暗、越随机.
cv::Mat addPoissonNoise(const cv::Mat& src, double alpha = 1.0, unsigned seed = 42);

// ------------------------------------------------------------------ 实用: 自动噪声标准差估计 (MAD 法)

// 通过最细层 Laplacian 残差的 Median Absolute Deviation 估计噪声 σ.
// σ = median(|I - I*G3x3|) / 0.6745 (J. Immerkær 1996).
double estimateNoiseSigma(const cv::Mat& gray);

// 综合"推荐"降噪流程:
//   1) estimateNoiseSigma;
//   2) 若 σ < 5 → guided filter (快速保边);
//      若 5 ≤ σ < 20 → bilateral + 轻微 NLM 组合;
//      若 σ ≥ 20 → NLM 主降噪 + guided 残差.
// 返回最终降噪结果, 并可选 outMethodName 说明实际使用的方法.
cv::Mat denoiseAuto(const cv::Mat& src, std::string* outMethodName = nullptr);

} // namespace algo
