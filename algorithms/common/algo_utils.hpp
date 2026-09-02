// algorithms/common/algo_utils.hpp
// Shared algorithm utilities: quality metrics (PSNR/SSIM/MAE/MSE/LOE/NIQE framework),
// input normalization/tiling, result visualization (horizontal/vertical stitching),
// color statistics, histogram enhancement, ECC alignment, etc.
// Shared by all algorithm modules to keep the API consistent.
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
// ---- Value range / type conversion --------------------------------------
// =========================================================================

// Convert 8UC[1/3] to 32FC[1/3] and normalize to [0,1].
cv::Mat toFloat(const cv::Mat& src);
// Clip a [0,1] float image to [0,1] and convert back to 8UC, preserving channel count.
cv::Mat to8U(const cv::Mat& src, double scale = 255.0);
// Linearly normalize any float image to [0,1] (for visualizing float32 HDR/Retinex, etc.).
cv::Mat normalizeTo01(const cv::Mat& src);

// =========================================================================
// ---- Full-Reference IQA ------------------------------------------------
// =========================================================================

// Mean Squared Error MSE = mean( (a-b)^2 ), lower is better.
double mse(const cv::Mat& a, const cv::Mat& b);
// Mean Absolute Error MAE = mean( |a-b| ), more robust to outliers than MSE.
double mae(const cv::Mat& a, const cv::Mat& b);
// Peak Signal-to-Noise Ratio, works for 8UC1/3. Returns dB. Identical images return 1000 dB.
double psnr(const cv::Mat& a, const cv::Mat& b);
// Simplified SSIM: single-channel 8U, closer to 1 is better. Multi-channel uses the mean.
double ssim(const cv::Mat& a, const cv::Mat& b);
// MS-SSIM framework (3 levels x 3x downsample, weighted simplified SSIM: luminance+contrast+structure).
// Typical weights: luminance=0.0448, contrast=0.2856, structure=0.1333 for the three levels.
double msssim(const cv::Mat& a, const cv::Mat& b, int levels = 3);
// Lightness Order Error LOE (Wang & Bovik):
// measures the fraction of local brightness ordering preserved after enhancement;
// smaller LOE means more natural colors.
// Sampling 32~64 pixels is enough for a stable estimate.
int loe(const cv::Mat& before, const cv::Mat& after, int samples = 48);

// =========================================================================
// ---- No-Reference IQA framework ----------------------------------------
// =========================================================================

// Brightness statistics {mean, std, per95, per05}: mean is overall brightness,
// std is contrast, per95/per05 are the brightest 5%/darkest 5% percentiles.
struct BrightStats { double mean, std, per95, per05; };
BrightStats brightnessStats(const cv::Mat& bgr);
// Color saturation (mean), larger means more vivid: sat_mean = mean( sqrt(Cr^2+Cb^2) ) / 128.
double saturationMean(const cv::Mat& bgr);
// Information entropy H = -sum p*log2(p), higher means richer information, in bit/pixel.
double imageEntropy(const cv::Mat& gray);
// EME (Measure of Enhancement): EME = (1/K^2)*sum 20*log(max_d/min_d+eps),
// over 8x8 blocks; larger means stronger enhancement (too large means over-enhancement).
double eme(const cv::Mat& gray, int blk = 8);
// Simplified approximation of the NIQE no-reference quality framework (variance of local statistics only),
// returns a no-reference quality score (smaller is better, 0 = ideal natural image).
double niqeScoreApprox(const cv::Mat& gray);

// Combine several NR-IQA metrics into {entropy, sat, eme, niqe} for one-line printing.
struct NRQuality { double entropy; double saturation; double eme; double niqeApprox; };
NRQuality nrQuality(const cv::Mat& bgr);

// =========================================================================
// ---- Image enhancement tools (reused by HDR / night_scene / beauty) -----
// =========================================================================

// LUT-driven Gamma correction: out = LUT[I], LUT[i] = pow(i/255, gamma) * 255.
cv::Mat gammaLUT(const cv::Mat& src, double gamma);
// Global auto brightness: linear stretch based on 1% and 99% percentiles, avoids extreme outliers.
cv::Mat autoContrast1pct(const cv::Mat& bgr);
// Histogram equalization (BGR: equalize the Y channel only to prevent color shift).
cv::Mat equalizeHistogramY(const cv::Mat& bgr);
// Simple Color Balance: per-channel 1% low/high percentile stretch.
// Fixes color cast while brightening; very effective for RAW->sRGB color casts.
cv::Mat simpleColorBalance(const cv::Mat& bgr, double pct = 0.01);
// White point detection (simplified Gray-world + MaxRGB): estimates white point RGB, returns (R,G,B) in [0,1].
cv::Vec3f estimateWhitePointGrayWorld(const cv::Mat& bgr);
// Simple white balance based on white point estimate: out_c = in_c * (1/white_c) * scale.
cv::Mat whiteBalanceFromPoint(const cv::Mat& bgr, const cv::Vec3f& white,
                               double clip = 2.0);

// =========================================================================
// ---- Visualization ------------------------------------------------------
// =========================================================================

// Horizontally stitch several same-size BGR/gray images with per-image titles.
cv::Mat hstackWithLabels(const std::vector<cv::Mat>& imgs,
                         const std::vector<std::string>& labels,
                         int labelHeight = 30);
// Stitch several same-size BGR/gray images into a grid of `cols` columns (e.g. cols=3 => 3 columns, multiple rows).
cv::Mat gridWithLabels(const std::vector<cv::Mat>& imgs,
                        const std::vector<std::string>& labels,
                        int cols = 3, int labelHeight = 30);
// Draw a pseudo-color heatmap below the image (input single-channel 8U) with a 60px title bar.
cv::Mat attachHeatmapBelow(const cv::Mat& top, const cv::Mat& grayMap,
                            const std::string& label);
// Resize an image to a fixed max edge then imshow, useful for debugging large images.
void imshowFit(const std::string& win, const cv::Mat& img,
               int maxEdge = 1024, int delay = 0);
// Simple progress/status print (with timestamp).
void log(const std::string& tag, const std::string& msg);
// Format a FR-IQA comparison table (multiple "method" -> (p, s) rows) into a multi-line string.
std::string formatFRIQATable(
    const std::vector<std::pair<std::string, std::pair<double, double>>>& rows);
// Format a NR-IQA comparison table into a multi-line string.
std::string formatNRIQATable(
    const std::vector<std::pair<std::string, NRQuality>>& rows);

// =========================================================================
// ---- Alignment (homography / affine / feature points) -------------------
// =========================================================================

// High-precision iterative ECC alignment to register src onto ref, returns homography/affine matrix (affine by default).
// Returns empty Mat on failure. Supports MOTION_TRANSLATION / MOTION_EUCLIDEAN / MOTION_AFFINE /
// MOTION_HOMOGRAPHY. For sequences with large exposure differences, applies CLAHE preprocessing to grayscale.
cv::Mat alignECC(const cv::Mat& ref, const cv::Mat& src,
                 int motionType = 1 /*cv::MOTION_AFFINE*/, int iters = 50,
                 double eps = 1e-6, bool preCLAHE = true);
// Align and directly return the warped image.
cv::Mat alignToRef(const cv::Mat& ref, const cv::Mat& src,
                   int motionType = 1, int iters = 50, double eps = 1e-6,
                   bool preCLAHE = true);
// Coarse brightness matching: align src mean/std to ref mean/std (Y channel only,
// to prevent color shift). Used as preprocessing for multi-frame / HDR inputs.
cv::Mat matchLuminanceStats(const cv::Mat& ref, const cv::Mat& src);

// =========================================================================
// ---- File tools & directory iteration ----------------------------------
// =========================================================================

// Join two paths (cross-platform).
std::string join(const std::string& a, const std::string& b);
// Ensure a directory exists, create it recursively if missing. Returns false on failure but does not throw.
bool ensureDir(const std::string& dir);
// List all files in a directory (non-recursive), filtered by extension (case-insensitive, empty = all).
std::vector<std::string> listFiles(const std::string& dir,
                                    const std::string& extFilter = "");
// Get the file name without path or extension, e.g. "foo/bar.png" -> "bar".
std::string baseNameNoExt(const std::string& path);

// =========================================================================
// ---- Tiled iteration (block-wise processing/evaluation for huge images) -
// =========================================================================

// Split the image into overlapping blk-size tiles and call fn(tile, bx, by, isLastX, isLastY) for each tile.
// Setting overlap to the registration/denoise radius removes tile boundary artifacts.
void processTiled(const cv::Mat& src, cv::Mat& dst, int blk, int overlap,
                  std::function<cv::Mat(const cv::Mat&)> fn);

} // namespace algo
