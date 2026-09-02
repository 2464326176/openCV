// algorithms/hdr/hdr_pipeline.cpp
#include "hdr_pipeline.hpp"
#include "../common/algo_utils.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>

namespace algo {

// 注: parseExposureTimeFromName / parseEvValueFromName / parseIsoFromName
//     已在 common/nv21_io.cpp 中实现, 此处不再重复定义, 避免链接期多重定义错误.

// ------------------------------------------------------------------ Tonemap helper

static std::string tmName(TonemapMethod m, float gamma) {
    std::ostringstream os;
    os.precision(2);
    switch (m) {
        case TonemapMethod::Drago:         os << "Drago γ=" << gamma; break;
        case TonemapMethod::Durand:        os << "Durand γ=" << gamma; break;
        case TonemapMethod::Reinhard:      os << "ReinhardGlobal γ=" << gamma; break;
        case TonemapMethod::ReinhardLocal: os << "ReinhardLocal γ=" << gamma; break;
        case TonemapMethod::Mantiuk:       os << "Mantiuk γ=" << gamma; break;
        case TonemapMethod::Linear:        os << "Linear γ=" << gamma; break;
        case TonemapMethod::Gamma:         os << "PowerLaw γ=" << gamma; break;
    }
    return os.str();
}

cv::Mat tonemapLinear(const cv::Mat& hdrF, float gamma) {
    if (hdrF.empty()) return cv::Mat();
    cv::Mat yc;
    if (hdrF.channels() == 3) cv::cvtColor(hdrF, yc, cv::COLOR_BGR2GRAY);
    else yc = hdrF;
    double mn, mx;
    cv::minMaxLoc(yc, &mn, &mx);
    if (mx - mn < 1e-8) mx = mn + 1;
    cv::Mat stretched = (hdrF - mn) * (1.0 / (mx - mn));
    cv::Mat powed;
    cv::pow(stretched, 1.0f / gamma, powed);
    cv::Mat out8u;
    powed.convertTo(out8u, CV_8U, 255.0);
    return out8u;
}

cv::Mat tonemapGamma(const cv::Mat& hdrF, float gamma, float whitePointPct) {
    if (hdrF.empty()) return cv::Mat();
    cv::Mat yc;
    if (hdrF.channels() == 3) cv::cvtColor(hdrF, yc, cv::COLOR_BGR2GRAY);
    else yc = hdrF;
    // 将白点设为 luminance top (100% - whitePointPct) 百分位
    std::vector<float> ys;
    ys.reserve(yc.rows * yc.cols);
    for (int y = 0; y < yc.rows; ++y) {
        const float* r = yc.ptr<float>(y);
        for (int x = 0; x < yc.cols; ++x) {
            float v = r[x];
            if (v > 1e-4f) ys.push_back(v);
        }
    }
    float Lwhite = 1.0f;
    if (!ys.empty()) {
        size_t idx = std::min(ys.size() - 1,
                              (size_t)((1.0 - whitePointPct * 0.01f) * ys.size()));
        std::nth_element(ys.begin(), ys.begin() + idx, ys.end());
        Lwhite = std::max(1e-3f, ys[idx]);
    }
    cv::Mat normed = hdrF / Lwhite;
    cv::pow(normed, 1.0f / gamma, normed);
    cv::threshold(normed, normed, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(normed, normed, 0.0, 0.0, cv::THRESH_TOZERO);
    cv::Mat out8u; normed.convertTo(out8u, CV_8U, 255.0);
    return out8u;
}

// Durand tonemap 的非 photo 依赖实现:
// 1) 取 log10(Y); 2) 对 logY 做 bilateral = 基础层 base, 高频 = logY - base;
// 3) 压缩基础层: base' = contrast * (base - maxBase) + maxBase;
// 4) Y' = 10^(base' + 高频); 5) 按比例恢复 RGB.
static cv::Mat tonemapDurandImpl(const cv::Mat& hdrF,
                                  const TonemapParams& p) {
    if (hdrF.empty() || hdrF.channels() != 3) return tonemapLinear(hdrF, p.gamma);
    cv::Mat hdrF32; hdrF.convertTo(hdrF32, CV_32F);
    cv::Mat ycc;
    cv::cvtColor(hdrF32, ycc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> chs; cv::split(ycc, chs);
    cv::Mat logY(chs[0].size(), CV_32F);
    for (int y = 0; y < chs[0].rows; ++y) {
        const float* r = chs[0].ptr<float>(y);
        float* w = logY.ptr<float>(y);
        for (int x = 0; x < chs[0].cols; ++x) {
            float v = std::max(1e-6f, r[x]);
            w[x] = (float)std::log10(v);
        }
    }
    cv::Mat base;
    int d = (int)(2 * p.spatialKernel + 1);
    double sSpace = p.spatialKernel;
    double sRange = p.rangeKernel; // range 单位是 log10(Y), 一般 0.3~0.6
    cv::bilateralFilter(logY, base, d, sRange, sSpace);
    double maxBase;
    cv::minMaxLoc(base, nullptr, &maxBase);
    // 压缩: 基础层缩放到 [-maxBase·contrast, 0], 即压缩倍率 = contrast
    cv::Mat compressed = p.contrast * (base - maxBase) + maxBase;
    cv::Mat detail = logY - base;
    cv::Mat newLogY = compressed + detail;
    cv::Mat newY(newLogY.size(), CV_32F);
    for (int y = 0; y < newY.rows; ++y) {
        const float* r = newLogY.ptr<float>(y);
        float* w = newY.ptr<float>(y);
        for (int x = 0; x < newY.cols; ++x) w[x] = (float)std::pow(10.0f, r[x]);
    }
    cv::Mat ratio = newY / (chs[0] + 1e-6f);
    for (size_t c = 0; c < 3; ++c) chs[c] = chs[c].mul(ratio);
    cv::merge(chs, ycc);
    cv::Mat bgrF; cv::cvtColor(ycc, bgrF, cv::COLOR_YCrCb2BGR);
    cv::pow(bgrF, 1.0f / p.gamma, bgrF);
    cv::threshold(bgrF, bgrF, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(bgrF, bgrF, 0.0, 0.0, cv::THRESH_TOZERO);
    cv::Mat out8u; bgrF.convertTo(out8u, CV_8U, 255.0);
    return out8u;
}

TonemapPack runMultipleTonemaps(const cv::Mat& hdrF,
                                 const std::vector<TonemapMethod>& tonemaps,
                                 TonemapParams p) {
    TonemapPack pack;
    if (hdrF.empty()) return pack;
    std::vector<TonemapMethod> todo = tonemaps;
    if (todo.empty()) todo = {TonemapMethod::Drago};
    for (TonemapMethod m : todo) {
        std::string key = tmName(m, p.gamma);
        cv::Mat ldrF;
        try {
            cv::Mat out8u;
            switch (m) {
                case TonemapMethod::Drago: {
                    auto t = cv::createTonemapDrago(p.gamma, 1.0f, 0.85f);
                    t->process(hdrF, ldrF);
                    ldrF.convertTo(out8u, CV_8U, 255.0);
                } break;
                case TonemapMethod::Durand:
                    out8u = tonemapDurandImpl(hdrF, p);
                    break;
                case TonemapMethod::Reinhard: {
                    auto t = cv::createTonemapReinhard(p.gamma, p.intensity,
                                                       p.lightAdapt, p.colorAdapt);
                    t->process(hdrF, ldrF);
                    if (std::fabs(p.saturation - 1.0f) > 1e-4) {
                        cv::cvtColor(ldrF, ldrF, cv::COLOR_BGR2HSV);
                        std::vector<cv::Mat> hs; cv::split(ldrF, hs);
                        hs[1] *= p.saturation;
                        cv::threshold(hs[1], hs[1], 1.0, 1.0, cv::THRESH_TRUNC);
                        cv::merge(hs, ldrF);
                        cv::cvtColor(ldrF, ldrF, cv::COLOR_HSV2BGR);
                    }
                    ldrF.convertTo(out8u, CV_8U, 255.0);
                } break;
                case TonemapMethod::ReinhardLocal: {
                    auto t = cv::createTonemapReinhard(p.gamma, p.intensity, 1.0f, 0.0f);
                    t->process(hdrF, ldrF);
                    ldrF.convertTo(out8u, CV_8U, 255.0);
                } break;
                case TonemapMethod::Mantiuk: {
                    auto t = cv::createTonemapMantiuk(p.gamma, 0.7f, p.saturation);
                    t->process(hdrF, ldrF);
                    ldrF.convertTo(out8u, CV_8U, 255.0);
                } break;
                case TonemapMethod::Linear:
                    out8u = tonemapLinear(hdrF, p.gamma);
                    break;
                case TonemapMethod::Gamma:
                    out8u = tonemapGamma(hdrF, p.gamma, 0.1f);
                    break;
            }
            if (!out8u.empty()) pack.results[key] = out8u;
        } catch (const cv::Exception& e) {
            log("hdr", std::string("tonemap[") + key + "] failed: " + e.what());
        }
    }
    return pack;
}

std::vector<float> computeHdrLogLuminanceHistogram(const cv::Mat& hdrF,
                                                     int binCount) {
    std::vector<float> hist(binCount, 0.0f);
    if (hdrF.empty()) return hist;
    cv::Mat gray;
    if (hdrF.channels() == 3) cv::cvtColor(hdrF, gray, cv::COLOR_BGR2GRAY);
    else gray = hdrF;
    if (gray.depth() != CV_32F) gray.convertTo(gray, CV_32F);
    const float lo = -6.0f, hi = 2.0f; // log10 亮度范围
    double total = 0;
    for (int y = 0; y < gray.rows; ++y) {
        const float* r = gray.ptr<float>(y);
        for (int x = 0; x < gray.cols; ++x) {
            float v = std::max(1e-6f, r[x]);
            float l = (float)std::log10(v);
            if (l < lo || l > hi) continue;
            int b = std::min(binCount - 1,
                             std::max(0, (int)((l - lo) / (hi - lo) * binCount)));
            hist[b] += 1.0f;
            total += 1.0f;
        }
    }
    if (total > 0) for (auto& h : hist) h /= (float)total;
    return hist;
}

// 自定义 Mertens 曝光融合 (简化实现, 与 OpenCV 结果近似但更可控)
cv::Mat mertensFusion(const std::vector<cv::Mat>& images8u,
                       float wContrast,
                       float wSaturation,
                       float wExposedness) {
    if (images8u.empty()) return cv::Mat();
    int H = images8u[0].rows, W = images8u[0].cols;
    std::vector<cv::Mat> fImgs;
    for (const auto& im : images8u) {
        cv::Mat f; im.convertTo(f, CV_32F, 1.0 / 255.0);
        fImgs.push_back(f);
    }
    std::vector<cv::Mat> weights(fImgs.size());
    for (size_t i = 0; i < fImgs.size(); ++i) {
        const auto& im = fImgs[i];
        // 1) 对比度: Laplacian 绝对值
        cv::Mat gray; cv::cvtColor(im, gray, cv::COLOR_BGR2GRAY);
        cv::Mat lap;
        cv::Mat k = (cv::Mat_<float>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
        cv::filter2D(gray, lap, CV_32F, k);
        cv::Mat C = cv::abs(lap);
        cv::pow(C, wContrast, C);

        // 2) 饱和度: 各通道标准差
        std::vector<cv::Mat> chs; cv::split(im, chs);
        cv::Mat mu = (chs[0] + chs[1] + chs[2]) / 3.0f;
        cv::Mat S = (chs[0] - mu).mul(chs[0] - mu)
                  + (chs[1] - mu).mul(chs[1] - mu)
                  + (chs[2] - mu).mul(chs[2] - mu);
        cv::sqrt(S / 3.0f, S);
        cv::pow(S, wSaturation, S);

        // 3) 曝光良好程度: exp(-Σ (x_i - 0.5)^2 / (2·σ^2))
        auto expo = [](const cv::Mat& x) -> cv::Mat {
            cv::Mat d = x - 0.5f; d = d.mul(d) / (2 * 0.2f * 0.2f);
            cv::Mat e; cv::exp(-d, e);
            return e;
        };
        cv::Mat E = expo(chs[0]).mul(expo(chs[1])).mul(expo(chs[2]));
        cv::pow(E, wExposedness, E);

        weights[i] = C.mul(S).mul(E) + 1e-9f;
    }
    // 归一化到和为 1
    cv::Mat sumW = cv::Mat::zeros(H, W, CV_32F);
    for (auto& w : weights) sumW += w;
    for (auto& w : weights) w = w / sumW;

    // 多分辨率融合 (Mertens 的 5 层高斯/拉普拉斯金字塔)
    const int levels = 5;
    auto buildGauss = [&](const cv::Mat& src) {
        std::vector<cv::Mat> gs; gs.push_back(src);
        for (int l = 1; l < levels; ++l) {
            cv::Mat d; cv::pyrDown(gs.back(), d);
            gs.push_back(d);
        }
        return gs;
    };
    auto buildLap = [&](const cv::Mat& src) {
        std::vector<cv::Mat> gs = buildGauss(src);
        std::vector<cv::Mat> ls;
        for (int l = 0; l < levels - 1; ++l) {
            cv::Mat up; cv::pyrUp(gs[l + 1], up, gs[l].size());
            ls.push_back(gs[l] - up);
        }
        ls.push_back(gs.back());
        return ls;
    };
    std::vector<std::vector<cv::Mat>> imgLaps, wGauss;
    for (size_t i = 0; i < fImgs.size(); ++i) {
        imgLaps.push_back(buildLap(fImgs[i]));
        wGauss.push_back(buildGauss(weights[i]));
    }
    // 合成每层
    std::vector<cv::Mat> blended(levels);
    for (int l = 0; l < levels; ++l) {
        blended[l] = cv::Mat::zeros(imgLaps[0][l].size(), CV_32FC3);
        for (size_t i = 0; i < fImgs.size(); ++i) {
            cv::Mat w3; cv::merge(std::vector<cv::Mat>{wGauss[i][l], wGauss[i][l], wGauss[i][l]}, w3);
            blended[l] += imgLaps[i][l].mul(w3);
        }
    }
    cv::Mat rec = blended.back();
    for (int l = (int)blended.size() - 2; l >= 0; --l) {
        cv::Mat up;
        cv::pyrUp(rec, up, blended[l].size());
        rec = up + blended[l];
    }
    cv::threshold(rec, rec, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(rec, rec, 0.0, 0.0, cv::THRESH_TOZERO);
    cv::Mat out8u; rec.convertTo(out8u, CV_8U, 255.0);
    return out8u;
}

// ------------------------------------------------------------------ 主流水线

static std::string crfMethodName(CrfMethod m) {
    return (m == CrfMethod::Debevec) ? "Debevec" : "Robertson";
}
static std::string mergeMethodName(HdrMergeMethod m) {
    return (m == HdrMergeMethod::Debevec) ? "Debevec" : "Robertson";
}

HdrResult hdrPipeline(const std::vector<cv::Mat>& images8u,
                       const std::vector<double>& times,
                       CrfMethod crf,
                       HdrMergeMethod merge,
                       const std::vector<TonemapMethod>& tonemaps,
                       TonemapParams tmParams) {
    HdrResult r;
    if (images8u.empty()) return r;
    CV_Assert(images8u.size() == times.size());
    r.exposureTimes = times;
    r.crfMethodName = crfMethodName(crf);
    r.mergeMethodName = mergeMethodName(merge);

    std::vector<cv::Mat> fImgs;
    for (const auto& im : images8u) {
        cv::Mat f; im.convertTo(f, CV_32FC3, 1.0 / 255.0);
        fImgs.push_back(f);
    }

    bool hasTime = false;
    for (double t : times) if (t > 0) { hasTime = true; break; }

    if (hasTime) {
        cv::Mat timesMat((int)times.size(), 1, CV_32FC1);
        for (size_t i = 0; i < times.size(); ++i)
            timesMat.at<float>(i, 0) = (float)times[i];

        try {
            if (crf == CrfMethod::Debevec) {
                auto cal = cv::createCalibrateDebevec();
                cal->process(images8u, r.responseCurve, timesMat);
            } else {
                auto cal = cv::createCalibrateRobertson();
                cal->process(images8u, r.responseCurve, timesMat);
            }
        } catch (const cv::Exception& e) {
            log("hdr", std::string("CRF calibrate failed: ") + e.what());
            r.responseCurve = cv::Mat();
        }

        try {
            if (merge == HdrMergeMethod::Debevec) {
                auto m = cv::createMergeDebevec();
                if (r.responseCurve.empty()) m->process(fImgs, r.hdrF, timesMat);
                else m->process(fImgs, r.hdrF, timesMat, r.responseCurve);
            } else {
                auto m = cv::createMergeRobertson();
                if (r.responseCurve.empty()) m->process(fImgs, r.hdrF, timesMat);
                else m->process(fImgs, r.hdrF, timesMat, r.responseCurve);
            }
        } catch (const cv::Exception& e) {
            log("hdr", std::string("HDR merge failed: ") + e.what() +
                " (fallback: Mertens only)");
            r.hdrF = cv::Mat();
        }

        if (!r.hdrF.empty()) {
            // 默认 Drago
            try {
                auto tm = cv::createTonemapDrago(tmParams.gamma, 1.0f, 0.85f);
                cv::Mat ldrF;
                tm->process(r.hdrF, ldrF);
                ldrF.convertTo(r.ldr, CV_8U, 255.0);
            } catch (const cv::Exception& e) {
                log("hdr", std::string("default tonemap failed: ") + e.what());
                r.ldr = tonemapLinear(r.hdrF, tmParams.gamma);
            }
            // 多种 tonemap 对比
            if (!tonemaps.empty())
                r.tonemapPack = runMultipleTonemaps(r.hdrF, tonemaps, tmParams);
            else
                r.tonemapPack.results[tmName(TonemapMethod::Drago, tmParams.gamma)] = r.ldr;

            r.recoveredLumHistogram = computeHdrLogLuminanceHistogram(r.hdrF, 20);
        }
    }

    // Mertens (两种版本: 原生 OpenCV + 自定义多分辨)
    try {
        cv::Ptr<cv::MergeMertens> mm = cv::createMergeMertens();
        cv::Mat fusionF;
        mm->process(fImgs, fusionF);
        fusionF.convertTo(r.fusion, CV_8U, 255.0);
    } catch (const cv::Exception& e) {
        log("hdr", std::string("cv::Mertens failed, fallback to our impl: ") + e.what());
        r.fusion = mertensFusion(images8u, 1.0f, 1.0f, 1.0f);
    }
    return r;
}

} // namespace algo
