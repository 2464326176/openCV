// algorithms/hdr/hdr_pipeline.hpp
// HDR / exposure fusion pipeline wrapper: CalibrateDebevec/Robertson -> MergeDebevec/Robertson
//  -> multiple Tonemaps (Drago / Durand / Reinhard / Mantiuk / Linear / CustomGamma),
// plus Mertens exposure fusion (no exposure times needed).
//
// All interfaces accept 8UC3 input and internally convert to 32F.
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/photo.hpp>
#include <map>
#include <string>
#include <vector>

namespace algo {

// Parse the "et_XXXXXX" field in the filename as exposure time (microseconds); returns 0 on failure.
double parseExposureTimeFromName(const std::string& filename);

// Parse ev_-4 / ev_0 / ev_-8 fields, return the EV value; returns 0 on failure.
int parseEvValueFromName(const std::string& filename);

// Parse ISO from the filename: iso_200 / ISO_800 / _iso1600, etc.; returns 0 on failure.
int parseIsoFromName(const std::string& filename);

// Camera response function (CRF) calibration method
enum class CrfMethod {
    Debevec = 0,    // Debevec-Malik 1997, native OpenCV
    Robertson = 1,  // Robertson-Borman-Stevenson 2003, native OpenCV
};

// HDR merge method (how to merge multiple LDRs into one HDR radiance map)
enum class HdrMergeMethod {
    Debevec = 0,    // Debevec weighting (exposure-weighted average after CRF inversion)
    Robertson = 1,  // Robertson iterative irradiance estimation
};

// Tonemap algorithms
enum class TonemapMethod {
    Drago = 0,          // Adaptive logarithmic mapping, F. Drago 2003
    Durand = 1,         // Bilateral filtering in the tone mapping domain, F. Durand 2002
    Reinhard = 2,       // Photographic tone reproduction for digital images, E. Reinhard 2002
    ReinhardLocal = 3,  // Local version of Reinhard with dodging-and-burning
    Mantiuk = 4,        // A perceptual framework for contrast processing, R. Mantiuk 2006
    Linear = 5,         // simple linear scaling (min/max stretch)
    Gamma = 6,          // power-law gamma + linear stretch
};

// Tonemap parameter set (different fields used by different algorithms)
struct TonemapParams {
    float gamma = 2.2f;        // Reinhard/Drago: gamma; also the exponent of the Gamma tonemap
    float intensity = 0.0f;    // Reinhard: 0=auto; otherwise the Lmean value (cd/m^2)
    float lightAdapt = 1.0f;   // Reinhard local: light adaptation
    float colorAdapt = 0.0f;   // Reinhard local: color adaptation
    float saturation = 1.0f;   // Reinhard: color saturation multiplier
    float bias = 0.85f;        // Mantiuk: bias parameter
    float contrast = 3.0f;     // Durand: contrast factor
    float spatialKernel = 8.f; // Durand: spatial sigma of bilateral (pixels)
    float rangeKernel = 0.4f;  // Durand: range sigma of bilateral (log units)
};

// Container for multiple tonemap results (methodName -> ldrImage).
struct TonemapPack {
    std::map<std::string, cv::Mat> results;  // key = "Drago γ=2.2" etc.
};

// Full HDR result
struct HdrResult {
    cv::Mat hdrF;                                 // HDR float32 (linear radiance)
    cv::Mat ldr;                                  // default tonemap (Drago γ=2.2)
    cv::Mat fusion;                               // Mertens exposure fusion 8UC3
    TonemapPack tonemapPack;                      // comparison results of multiple tonemaps
    std::vector<double> exposureTimes;            // actually used exposure times
    std::string crfMethodName;                    // "Debevec" / "Robertson"
    std::string mergeMethodName;                  // "Debevec" / "Robertson"
    cv::Mat responseCurve;                        // CRF g(z), 256x1x3 (BGR) CV_32F
    std::vector<float> recoveredLumHistogram;     // HDR log10(Y) histogram (20 bins)
};

// Full HDR pipeline.
//   images8u:  exposure bracket
//   times:     corresponding actual exposure times (any unit, but proportional); all 0 => Mertens only.
//   crf:       CRF calibration method
//   merge:     HDR merge method
//   tonemaps:  requested tonemap kinds; empty => only the default Drago.
//   tmParams:  global tonemap params (all tonemaps share gamma; other fields are algorithm-specific)
HdrResult hdrPipeline(const std::vector<cv::Mat>& images8u,
                       const std::vector<double>& times,
                       CrfMethod crf = CrfMethod::Debevec,
                       HdrMergeMethod merge = HdrMergeMethod::Debevec,
                       const std::vector<TonemapMethod>& tonemaps = {},
                       TonemapParams tmParams = TonemapParams{});

// Run multiple tonemaps only (on an existing HDR result): returns a TonemapPack
TonemapPack runMultipleTonemaps(const cv::Mat& hdrF,
                                 const std::vector<TonemapMethod>& tonemaps,
                                 TonemapParams p = TonemapParams{});

// Linear tonemap implementation (min/max stretch + gamma, independent of the OpenCV photo submodule)
cv::Mat tonemapLinear(const cv::Mat& hdrF, float gamma = 2.2f);

// Manual gamma mapping: L_out = (L / L_white)^(1/gamma)
cv::Mat tonemapGamma(const cv::Mat& hdrF, float gamma = 2.2f,
                      float whitePointPct = 0.1f);

// Compute a log10(Y) histogram from an HDR image (binCount bins, range [-6, 2])
std::vector<float> computeHdrLogLuminanceHistogram(const cv::Mat& hdrF,
                                                     int binCount = 20);

// Mertens exposure fusion: custom weights for the three weight terms
cv::Mat mertensFusion(const std::vector<cv::Mat>& images8u,
                       float wContrast = 1.0f,
                       float wSaturation = 1.0f,
                       float wExposedness = 1.0f);

} // namespace algo
