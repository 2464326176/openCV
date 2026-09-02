// algorithms/common/single_denoise.hpp
// Single-frame denoising collection: Gaussian / Median / Bilateral / NLM / Guided
//                    + Wiener / WaveletSoftThreshold / AnisotropicDiffusion
//                    + AdaptiveSigmaBilateral.
//
// The multi-frame denoise_multi also reuses Gaussian/NLM as baselines.
// beauty reuses Bilateral + Guided for skin smoothing.
// This file does not depend on opencv_contrib (xphoto / cudaimgproc), for easy standalone compilation.
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

namespace algo {

// ------------------------------------------------------------------ basics

// Gaussian blur: when sigma=0 it is estimated from ksize (sigma = 0.3*((ksize-1)*0.5 - 1) + 0.8).
cv::Mat denoiseGaussian(const cv::Mat& src, int ksize = 5, double sigma = 0);

// Median filter: ksize must be odd. Good for salt-and-pepper noise, preserves step edges but loses thin lines.
cv::Mat denoiseMedian(const cv::Mat& src, int ksize = 5);

// Bilateral filter: edge-preserving smoothing (baseline for beauty / low-light denoising).
// Suggested params: d <= 9, sigmaColor=30~100, sigmaSpace=30~100.
cv::Mat denoiseBilateral(const cv::Mat& src, int d = 9,
                         double sigmaColor = 75, double sigmaSpace = 75);

// Guided filter (He 2010): self-guided version (guide = src) and externally-guided version.
// Denoise while preserving edges, complexity O(N).
// Smaller eps => stronger edge preservation; larger radius => stronger smoothing.
cv::Mat denoiseGuided(const cv::Mat& src, const cv::Mat& guide,
                       int radius = 8, double eps = 0.01);
inline cv::Mat denoiseGuided(const cv::Mat& src, int radius = 8,
                              double eps = 0.01) {
    return denoiseGuided(src, src, radius, eps);
}

// Non-local Means NLM: larger h => stronger smoothing; templateSize is the patch, searchSize is the search radius.
cv::Mat denoiseNLM(const cv::Mat& srcBgr, float h = 10, int templateSize = 7,
                    int searchSize = 21);
// Grayscale NLM.
cv::Mat denoiseNLMGray(const cv::Mat& srcGray, float h = 10);

// ------------------------------------------------------------------ advanced

// Adaptive bilateral: sigmaColor adapts to the local standard deviation.
//   - in detail-rich regions (large std) lower sigmaColor to preserve edges
//   - in flat regions (small std) raise sigmaColor to denoise
// winRadius is the window radius for estimating local std (default 7).
cv::Mat denoiseAdaptiveBilateral(const cv::Mat& src, int d = 9,
                                  double baseSigmaColor = 75,
                                  double sigmaSpace = 75,
                                  int winRadius = 7);

// Wiener filter (local MMSE): out = mean + max(0, (localVar - noiseVar)/localVar)
//                                                                          * (I - mean)
// Assumes additive white Gaussian noise; when noiseVar=0 it auto-estimates noiseVar ≈ mean(localVar).
cv::Mat denoiseWiener(const cv::Mat& src, int ksize = 5, double noiseVar = 0);

// Anisotropic diffusion (Perona-Malik 1990):
// dI/dt = div( c(||∇I||) · ∇I ), using exponential conductivity c(x)=exp(-(x/K)^2).
// Effect: smooth Gaussian noise while protecting strong edges.
// Typical params: niters=20, K=15~30, dt=0.1 (unit-calibrated for 8U images).
cv::Mat denoiseAnisotropicDiffusion(const cv::Mat& src, int niters = 20,
                                     double K = 20.0, double dt = 0.15);

// Wavelet-like soft-threshold denoising approximation (OpenCV has no DWT, use Haar-like bilinear up/down sampling).
// Implementation: 1) build a Laplacian pyramid L0...Ln;
//       2) soft-threshold the high-frequency layers Ln (differences): sign(x)*max(0, |x|-th);
//       3) reconstruct.
// th = noiseStd * thresholdScale; when noiseStd=0 estimate via MAD of the finest layer.
cv::Mat denoiseLaplacianSoftThreshold(const cv::Mat& src, int levels = 4,
                                       double noiseStd = 0,
                                       double thresholdScale = 3.0);

// ------------------------------------------------------------------ noise synthesis (for denoise_single / denoise_multi demos)

// Additive Gaussian noise N(0, sigma^2).
cv::Mat addGaussianNoise(const cv::Mat& src, double sigma, unsigned seed = 42);

// Salt-and-pepper noise: prob p to salt (255), prob p to pepper (0).
cv::Mat addSaltPepperNoise(const cv::Mat& src, double p, unsigned seed = 42);

// Multiplicative Rayleigh noise (ultrasound / MRI simulation): out = I * rayleigh(scale), σ_noise ≈ scale*I.
cv::Mat addSpeckleNoise(const cv::Mat& src, double scale, unsigned seed = 42);

// Poisson noise (photon noise): out ~ Poisson(α*I), smaller α => darker and more random.
cv::Mat addPoissonNoise(const cv::Mat& src, double alpha = 1.0, unsigned seed = 42);

// ------------------------------------------------------------------ utility: automatic noise std estimation (MAD method)

// Estimate noise σ from the Median Absolute Deviation of the finest-layer Laplacian residual.
// σ = median(|I - I*G3x3|) / 0.6745 (J. Immerkær 1996).
double estimateNoiseSigma(const cv::Mat& gray);

// Comprehensive "recommended" denoising pipeline:
//   1) estimateNoiseSigma;
//   2) if σ < 5 -> guided filter (fast edge-preserving);
//      if 5 ≤ σ < 20 -> bilateral + light NLM combination;
//      if σ ≥ 20 -> NLM main denoise + guided residual.
// Returns the final denoised result, and optionally outMethodName describing the method actually used.
cv::Mat denoiseAuto(const cv::Mat& src, std::string* outMethodName = nullptr);

} // namespace algo
