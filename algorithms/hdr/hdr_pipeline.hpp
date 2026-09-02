// algorithms/hdr/hdr_pipeline.hpp
// HDR/曝光融合流水线封装: CalibrateDebevec/Robertson -> MergeDebevec/Robertson
//  -> 多种 Tonemap (Drago / Durand / Reinhard / Mantiuk / Linear / CustomGamma),
// 以及 Mertens 曝光融合 (无需曝光时间).
//
// 所有接口均接受 8UC3 输入, 内部自动转 32F.
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/photo.hpp>
#include <map>
#include <string>
#include <vector>

namespace algo {

// 解析文件名中 "et_XXXXXX" 字段为曝光时间 (微秒), 失败返回 0.
double parseExposureTimeFromName(const std::string& filename);

// 解析 ev_-4 / ev_0 / ev_-8 字段, 返回 EV 值; 失败返回 0.
int parseEvValueFromName(const std::string& filename);

// 从文件名中解析 ISO: iso_200 / ISO_800 / _iso1600 等, 失败返回 0.
int parseIsoFromName(const std::string& filename);

// 相机响应函数 (CRF) 校准方法
enum class CrfMethod {
    Debevec = 0,    // Debevec-Malik 1997, OpenCV 原生
    Robertson = 1,  // Robertson-Borman-Stevenson 2003, OpenCV 原生
};

// HDR 融合方法 (如何合并多张 LDR → 一张 HDR radiance map)
enum class HdrMergeMethod {
    Debevec = 0,    // Debevec 加权 (CRF 反演后曝光加权平均)
    Robertson = 1,  // Robertson 迭代估计辐照度
};

// Tonemap 算法
enum class TonemapMethod {
    Drago = 0,          // Adaptive logarithmic mapping, F. Drago 2003
    Durand = 1,         // Bilateral filtering in the tone mapping domain, F. Durand 2002
    Reinhard = 2,       // Photographic tone reproduction for digital images, E. Reinhard 2002
    ReinhardLocal = 3,  // Local version of Reinhard with dodging-and-burning
    Mantiuk = 4,        // A perceptual framework for contrast processing, R. Mantiuk 2006
    Linear = 5,         // 简单线性缩放 (min/max stretch)
    Gamma = 6,          // 幂律 gamma + 线性 stretch
};

// Tonemap 参数集合 (可根据不同算法使用不同字段)
struct TonemapParams {
    float gamma = 2.2f;        // Reinhard/Drago: gamma; 也用于 Gamma tonemap 的幂次
    float intensity = 0.0f;    // Reinhard: 0=auto; 否则 Lmean 设定值 (cd/m^2)
    float lightAdapt = 1.0f;   // Reinhard local: 光适应
    float colorAdapt = 0.0f;   // Reinhard local: 色适应
    float saturation = 1.0f;   // Reinhard: 色彩饱和倍率
    float bias = 0.85f;        // Mantiuk: bias parameter
    float contrast = 3.0f;     // Durand: contrast factor
    float spatialKernel = 8.f; // Durand: spatial sigma of bilateral (像素)
    float rangeKernel = 0.4f;  // Durand: range sigma of bilateral (对数单位)
};

// 多种 tonemap 结果容器 (methodName -> ldrImage).
struct TonemapPack {
    std::map<std::string, cv::Mat> results;  // key = "Drago γ=2.2" 等
};

// HDR 完整结果
struct HdrResult {
    cv::Mat hdrF;                                 // HDR float32 (linear radiance)
    cv::Mat ldr;                                  // 默认 Tonemap (Drago γ=2.2)
    cv::Mat fusion;                               // Mertens 曝光融合 8UC3
    TonemapPack tonemapPack;                      // 多种 tonemap 的对比结果
    std::vector<double> exposureTimes;            // 实际使用的曝光时间
    std::string crfMethodName;                    // "Debevec" / "Robertson"
    std::string mergeMethodName;                  // "Debevec" / "Robertson"
    cv::Mat responseCurve;                        // CRF g(z), 256x1x3 (BGR) CV_32F
    std::vector<float> recoveredLumHistogram;     // HDR log10(Y) 直方图 (20 bins)
};

// 完整 HDR 流水线.
//   images8u:  曝光序列
//   times:     对应的实际曝光时间 (任意单位但需成比例); 全为 0 则仅做 Mertens.
//   crf:       CRF 校准方法
//   merge:     HDR 合并方法
//   tonemaps:  要求生成的 tonemap 种类; 为空则仅生成默认 Drago.
//   tmParams:  全局 tonemap 参数 (所有 tonemap 共用 gamma, 其他字段算法自取)
HdrResult hdrPipeline(const std::vector<cv::Mat>& images8u,
                       const std::vector<double>& times,
                       CrfMethod crf = CrfMethod::Debevec,
                       HdrMergeMethod merge = HdrMergeMethod::Debevec,
                       const std::vector<TonemapMethod>& tonemaps = {},
                       TonemapParams tmParams = TonemapParams{});

// 只跑多种 Tonemap (对已存在的 HDR 结果): 返回 TonemapPack
TonemapPack runMultipleTonemaps(const cv::Mat& hdrF,
                                 const std::vector<TonemapMethod>& tonemaps,
                                 TonemapParams p = TonemapParams{});

// 线性 Tonemap 实现 (min/max stretch + gamma, 不依赖 OpenCV photo 子模块)
cv::Mat tonemapLinear(const cv::Mat& hdrF, float gamma = 2.2f);

// 手动 Gamma 映射: L_out = (L / L_white)^(1/gamma)
cv::Mat tonemapGamma(const cv::Mat& hdrF, float gamma = 2.2f,
                      float whitePointPct = 0.1f);

// 从 HDR 图中计算 log10(Y) 直方图 (binCount 个 bin, 范围 [-6, 2])
std::vector<float> computeHdrLogLuminanceHistogram(const cv::Mat& hdrF,
                                                     int binCount = 20);

// Mertens 曝光融合: 自定义三个权重的权重
cv::Mat mertensFusion(const std::vector<cv::Mat>& images8u,
                       float wContrast = 1.0f,
                       float wSaturation = 1.0f,
                       float wExposedness = 1.0f);

} // namespace algo
