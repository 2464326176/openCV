// algorithms/common/single_denoise.cpp
#include "single_denoise.hpp"
#include "algo_utils.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace algo {

// ------------------------------------------------------------------ basics

cv::Mat denoiseGaussian(const cv::Mat& src, int ksize, double sigma) {
    CV_Assert(!src.empty());
    cv::Mat out;
    cv::GaussianBlur(src, out, cv::Size(ksize, ksize), sigma, sigma);
    return out;
}

cv::Mat denoiseMedian(const cv::Mat& src, int ksize) {
    CV_Assert(!src.empty());
    cv::Mat out;
    cv::medianBlur(src, out, ksize);
    return out;
}

cv::Mat denoiseBilateral(const cv::Mat& src, int d, double sc, double ss) {
    CV_Assert(!src.empty());
    cv::Mat out;
    cv::bilateralFilter(src, out, d, sc, ss);
    return out;
}

cv::Mat denoiseGuided(const cv::Mat& src, const cv::Mat& guide, int radius,
                       double eps) {
    CV_Assert(!src.empty() && !guide.empty());
    cv::Mat g;
    if (guide.channels() == 3) cv::cvtColor(guide, g, cv::COLOR_BGR2GRAY);
    else g = guide.clone();
    g.convertTo(g, CV_32F, 1.0 / 255.0);

    cv::Mat meanI, meanII;
    cv::boxFilter(g, meanI, CV_32F, cv::Size(radius, radius));
    cv::Mat gg = g.mul(g);
    cv::boxFilter(gg, meanII, CV_32F, cv::Size(radius, radius));
    cv::Mat denom = meanII - meanI.mul(meanI) + eps;

    std::vector<cv::Mat> srcChannels;
    bool isColor = (src.channels() == 3);
    if (isColor) cv::split(src, srcChannels);
    else srcChannels.push_back(src);

    std::vector<cv::Mat> outChannels;
    for (const auto& sc : srcChannels) {
        cv::Mat s;
        sc.convertTo(s, CV_32F, 1.0 / 255.0);
        cv::Mat meanP, meanIP_;
        cv::boxFilter(s, meanP, CV_32F, cv::Size(radius, radius));
        cv::Mat gp = g.mul(s);
        cv::boxFilter(gp, meanIP_, CV_32F, cv::Size(radius, radius));
        cv::Mat a = (meanIP_ - meanI.mul(meanP)) / denom;
        cv::Mat b = meanP - a.mul(meanI);
        cv::Mat meanA, meanB;
        cv::boxFilter(a, meanA, CV_32F, cv::Size(radius, radius));
        cv::boxFilter(b, meanB, CV_32F, cv::Size(radius, radius));
        cv::Mat q = meanA.mul(g) + meanB;
        cv::Mat out8u;
        q.convertTo(out8u, CV_8U, 255.0);
        outChannels.push_back(out8u);
    }

    cv::Mat out;
    if (isColor) cv::merge(outChannels, out);
    else out = outChannels[0];
    return out;
}

cv::Mat denoiseNLM(const cv::Mat& srcBgr, float h, int tsize, int ssize) {
    CV_Assert(!srcBgr.empty());
    cv::Mat out;
    cv::fastNlMeansDenoisingColored(srcBgr, out, h, h, tsize, ssize);
    return out;
}

cv::Mat denoiseNLMGray(const cv::Mat& srcGray, float h) {
    CV_Assert(!srcGray.empty());
    cv::Mat out;
    cv::fastNlMeansDenoising(srcGray, out, h, 7, 21);
    return out;
}

// ------------------------------------------------------------------ advanced

cv::Mat denoiseAdaptiveBilateral(const cv::Mat& src, int d,
                                  double baseSigmaColor, double sigmaSpace,
                                  int winRadius) {
    CV_Assert(!src.empty() && src.channels() == 3);
    cv::Mat gray; cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::Mat m, v;
    cv::boxFilter(gray, m, CV_32F, cv::Size(winRadius * 2 + 1, winRadius * 2 + 1));
    cv::Mat g2; gray.convertTo(g2, CV_32F); g2 = g2.mul(g2);
    cv::boxFilter(g2, v, CV_32F, cv::Size(winRadius * 2 + 1, winRadius * 2 + 1));
    cv::Mat localVar = cv::max(0.0, v - m.mul(m));
    cv::Mat stdMat; cv::sqrt(localVar, stdMat);

    // σ_color(p) = baseSigma * exp( -std(p)^2 / 40^2 )
    // In flat regions (std≈0) σ_color≈base (strong smoothing); in detail regions (large std) σ↓ to preserve edges.
    double denom = 40.0 * 40.0;
    std::vector<cv::Mat> chs; cv::split(src, chs);
    std::vector<cv::Mat> outChs;
    for (auto& c : chs) {
        cv::Mat smooth_base;
        cv::bilateralFilter(c, smooth_base, d, baseSigmaColor, sigmaSpace);
        cv::Mat smooth_weak;
        cv::bilateralFilter(c, smooth_weak, d, std::max(5.0, baseSigmaColor * 0.3), sigmaSpace);
        cv::Mat w;
        cv::exp(-localVar / denom, w);
        // w ∈ [0,1]: w=1 => strong smoothing (flat), w=0 => weak smoothing (edge-preserving)
        smooth_base.convertTo(smooth_base, CV_32F);
        smooth_weak.convertTo(smooth_weak, CV_32F);
        cv::Mat outF = w.mul(smooth_base) + (1.0f - w).mul(smooth_weak);
        cv::Mat out8u; outF.convertTo(out8u, CV_8U);
        outChs.push_back(out8u);
    }
    cv::Mat out; cv::merge(outChs, out);
    return out;
}

// Wiener per-channel
static cv::Mat wienerSingleChannel(const cv::Mat& src8u, int ksize, double noiseVar) {
    cv::Mat f; src8u.convertTo(f, CV_32F);
    cv::Mat mean, sqmean;
    cv::blur(f, mean, cv::Size(ksize, ksize));
    cv::Mat sq = f.mul(sq);
    cv::blur(sq, sqmean, cv::Size(ksize, ksize));
    cv::Mat var = sqmean - mean.mul(mean);
    if (noiseVar <= 0) noiseVar = std::max(1.0, cv::mean(var)[0] * 0.5);
    cv::Mat factor = var / (var + noiseVar);
    cv::Mat outF = mean + factor.mul(f - mean);
    cv::Mat out8u; outF.convertTo(out8u, CV_8U);
    return out8u;
}

cv::Mat denoiseWiener(const cv::Mat& src, int ksize, double noiseVar) {
    CV_Assert(!src.empty());
    if (src.channels() == 1) return wienerSingleChannel(src, ksize, noiseVar);
    std::vector<cv::Mat> chs; cv::split(src, chs);
    for (auto& c : chs) c = wienerSingleChannel(c, ksize, noiseVar);
    cv::Mat out; cv::merge(chs, out);
    return out;
}

// Perona-Malik anisotropic diffusion (explicit Euler), single channel
static cv::Mat pmStep(const cv::Mat& src, double K, double dt) {
    CV_Assert(src.depth() == CV_32F);
    int H = src.rows, W = src.cols;
    cv::Mat out = src.clone();
    for (int y = 1; y < H - 1; ++y) {
        const float* rN = src.ptr<float>(y - 1);
        const float* rM = src.ptr<float>(y);
        const float* rS = src.ptr<float>(y + 1);
        float* rO = out.ptr<float>(y);
        for (int x = 1; x < W - 1; ++x) {
            float gN = rM[x] - rN[x];
            float gS = rS[x] - rM[x];
            float gW = rM[x] - rM[x - 1];
            float gE = rM[x + 1] - rM[x];
            auto conduct = [&](float g) -> float {
                float r = g / K;
                return (float)std::exp(-r * r);
            };
            float cN = conduct(gN), cS = conduct(gS), cW = conduct(gW), cE = conduct(gE);
            rO[x] = rM[x] + (float)dt * (cN * gN + cS * gS + cW * gW + cE * gE);
        }
    }
    return out;
}

cv::Mat denoiseAnisotropicDiffusion(const cv::Mat& src, int niters,
                                     double K, double dt) {
    CV_Assert(!src.empty());
    if (src.channels() == 3) {
        // Diffuse on the Y channel of YCrCb to preserve color
        cv::Mat yc; cv::cvtColor(src, yc, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> chs; cv::split(yc, chs);
        cv::Mat f; chs[0].convertTo(f, CV_32F);
        for (int i = 0; i < niters; ++i) f = pmStep(f, K, dt);
        f.convertTo(chs[0], CV_8U);
        cv::merge(chs, yc);
        cv::Mat out; cv::cvtColor(yc, out, cv::COLOR_YCrCb2BGR);
        return out;
    } else {
        cv::Mat f; src.convertTo(f, CV_32F);
        for (int i = 0; i < niters; ++i) f = pmStep(f, K, dt);
        cv::Mat out; f.convertTo(out, CV_8U);
        return out;
    }
}

cv::Mat denoiseLaplacianSoftThreshold(const cv::Mat& src, int levels,
                                       double noiseStd, double thScale) {
    CV_Assert(!src.empty() && levels >= 1);
    // Only process the Y channel, keep Cr/Cb for color
    cv::Mat yc;
    if (src.channels() == 3) cv::cvtColor(src, yc, cv::COLOR_BGR2YCrCb);
    else yc = src.clone();
    std::vector<cv::Mat> chs; cv::split(yc, chs);
    cv::Mat Y; chs[0].convertTo(Y, CV_32F, 1.0 / 255.0);

    // Build Gaussian + Laplacian pyramid
    std::vector<cv::Mat> gp; gp.push_back(Y);
    for (int l = 1; l < levels; ++l) {
        cv::Mat d; cv::pyrDown(gp.back(), d);
        gp.push_back(d);
    }
    std::vector<cv::Mat> laplacians;
    for (int l = 0; l < levels - 1; ++l) {
        cv::Mat up; cv::pyrUp(gp[l + 1], up, gp[l].size());
        laplacians.push_back(gp[l] - up);
    }
    laplacians.push_back(gp.back()); // keep the coarsest layer

    // Estimate noise σ (only the finest layer)
    if (noiseStd <= 0) {
        cv::Mat L0 = laplacians[0];
        int H = L0.rows, W = L0.cols;
        std::vector<float> vals; vals.reserve(H * W);
        for (int y = 0; y < H; ++y) {
            const float* r = L0.ptr<float>(y);
            for (int x = 0; x < W; ++x) vals.push_back(std::fabs(r[x]));
        }
        std::nth_element(vals.begin(), vals.begin() + vals.size() / 2, vals.end());
        double mad = vals[vals.size() / 2];
        noiseStd = mad / 0.6745;
    }
    double th = noiseStd * thScale;

    // Soft threshold on Laplacian layers (coarsest layer not thresholded, to preserve brightness DC)
    for (int l = 0; l < (int)laplacians.size() - 1; ++l) {
        auto& L = laplacians[l];
        // sign(x)*max(0,|x|-th)
        cv::Mat a = cv::abs(L) - th;
        a = cv::max(a, 0.0);
        cv::Mat sign;
        cv::divide(L, cv::abs(L) + 1e-12, sign);  // sign(x) ∈ {-1,0,1}, x=0 -> 0
        L = sign.mul(a);
    }

    // Reconstruct
    cv::Mat rec = laplacians.back();
    for (int l = (int)laplacians.size() - 2; l >= 0; --l) {
        cv::Mat up;
        cv::pyrUp(rec, up, gp[l].size());
        rec = up + laplacians[l];
    }
    cv::threshold(rec, rec, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(rec, rec, 0.0, 0.0, cv::THRESH_TOZERO);
    rec.convertTo(chs[0], CV_8U, 255.0);
    cv::merge(chs, yc);
    if (src.channels() == 3) {
        cv::Mat out; cv::cvtColor(yc, out, cv::COLOR_YCrCb2BGR);
        return out;
    }
    return chs[0];
}

// ------------------------------------------------------------------ noise synthesis

cv::Mat addGaussianNoise(const cv::Mat& src, double sigma, unsigned seed) {
    cv::Mat n(src.size(), src.type());
    cv::RNG rng(seed);
    rng.fill(n, cv::RNG::NORMAL, 0, sigma);
    cv::Mat noisy = src + n;
    return noisy;
}

cv::Mat addSaltPepperNoise(const cv::Mat& src, double p, unsigned seed) {
    cv::RNG rng(seed);
    cv::Mat mask(src.size(), CV_8UC1);
    rng.fill(mask, cv::RNG::UNIFORM, 0, 10000);
    cv::Mat out = src.clone();
    int H = src.rows, W = src.cols;
    int C = src.channels();
    int thresholdSalt = (int)(p * 10000 * 0.5);
    int thresholdPep = (int)(p * 10000);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int r = mask.at<uchar>(y, x);
            if (r < thresholdSalt) {
                if (C == 1) out.at<uchar>(y, x) = 255;
                else out.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
            } else if (r < thresholdPep) {
                if (C == 1) out.at<uchar>(y, x) = 0;
                else out.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
            }
        }
    }
    return out;
}

cv::Mat addSpeckleNoise(const cv::Mat& src, double scale, unsigned seed) {
    // Rayleigh CDF sampling: s = scale·sqrt(-2·ln(U))
    std::mt19937_64 gen(seed);
    std::uniform_real_distribution<double> uni(1e-5, 1.0 - 1e-5);
    cv::Mat out; src.convertTo(out, CV_64F);
    int H = src.rows, W = src.cols, C = src.channels();
    std::vector<double> ray(H * W);
    for (size_t i = 0; i < ray.size(); ++i) {
        double u = uni(gen);
        ray[i] = scale * std::sqrt(-2.0 * std::log(u));
    }
    double* p = (double*)out.data;
    size_t i = 0;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W * C; ++x) {
            size_t k = (y * W + x / C);
            p[i] = p[i] * ray[k];
            if (p[i] < 0) p[i] = 0;
            if (p[i] > 255) p[i] = 255;
            ++i;
        }
    }
    cv::Mat u8; out.convertTo(u8, CV_8U);
    return u8;
}

cv::Mat addPoissonNoise(const cv::Mat& src, double alpha, unsigned seed) {
    // For each pixel I, draw a Poisson(α·I) sample then divide by α
    std::mt19937_64 gen(seed);
    cv::Mat out = src.clone();
    int H = src.rows, W = src.cols, C = src.channels();
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (C == 1) {
                double lam = alpha * std::max(1e-6, (double)src.at<uchar>(y, x));
                std::poisson_distribution<int> pd(lam);
                double v = pd(gen) / alpha;
                out.at<uchar>(y, x) = cv::saturate_cast<uchar>(v);
            } else {
                auto v = src.at<cv::Vec3b>(y, x);
                cv::Vec3b r;
                for (int c = 0; c < 3; ++c) {
                    double lam = alpha * std::max(1e-6, (double)v[c]);
                    std::poisson_distribution<int> pd(lam);
                    double val = pd(gen) / alpha;
                    r[c] = cv::saturate_cast<uchar>(val);
                }
                out.at<cv::Vec3b>(y, x) = r;
            }
        }
    }
    return out;
}

// ------------------------------------------------------------------ noise estimation

double estimateNoiseSigma(const cv::Mat& gray) {
    CV_Assert(!gray.empty() && gray.depth() == CV_8U);
    // Laplacian kernel = [[1 -2 1], [-2 4 -2], [1 -2 1]]
    cv::Mat lap;
    cv::Mat ker = (cv::Mat_<float>(3, 3) << 1, -2, 1, -2, 4, -2, 1, -2, 1);
    cv::filter2D(gray, lap, CV_32F, ker);
    lap = cv::abs(lap) / 6.0; // scale to match the high-frequency residual magnitude
    std::vector<float> vals;
    vals.reserve(lap.total());
    for (int y = 0; y < lap.rows; ++y) {
        const float* r = lap.ptr<float>(y);
        for (int x = 0; x < lap.cols; ++x) vals.push_back(r[x]);
    }
    if (vals.empty()) return 0;
    size_t mid = vals.size() / 2;
    std::nth_element(vals.begin(), vals.begin() + mid, vals.end());
    double mad = vals[mid];
    return mad / 0.6745;
}

cv::Mat denoiseAuto(const cv::Mat& src, std::string* outMethodName) {
    CV_Assert(!src.empty());
    cv::Mat gray;
    if (src.channels() == 3) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else gray = src;
    double sigma = estimateNoiseSigma(gray);
    std::string method;
    cv::Mat out;
    if (sigma < 5.0) {
        method = "guided(r=4,eps=1e-3)";
        out = denoiseGuided(src, 4, 1e-3);
    } else if (sigma < 20.0) {
        method = "bilateral(9,75,75) + guided(r=6,eps=0.02)";
        cv::Mat b = denoiseBilateral(src, 9, 75, 75);
        out = denoiseGuided(b, src, 6, 0.02);
    } else {
        float h = (float)(sigma * 0.6);
        method = "NLM(h=" + std::to_string(h).substr(0, 4) + ") + guided";
        cv::Mat n = (src.channels() == 3) ? denoiseNLM(src, h, 7, 21) : denoiseNLMGray(gray, h);
        out = denoiseGuided(n, src, 5, 0.02);
    }
    if (outMethodName) *outMethodName = method;
    (void)algo::log; // keep unused symbol referenced; no logging side effect
    return out;
}

} // namespace algo
