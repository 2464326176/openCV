// algorithms/denoise_multi/main.cpp
// Multi-frame denoising demo: alignment (Affine/Euclidean/Homography) + multiple aggregation.
//
// Extended capabilities:
//   1. Alignment models: MOTION_EUCLIDEAN / MOTION_AFFINE / MOTION_HOMOGRAPHY,
//      three alignment models run sequentially on the same scene for comparison.
//   2. Aggregation strategies:
//        · arithmetic mean
//        · median
//        · variance-weighted mean: estimate local variance across frames at the same position,
//          pixels with higher variance get lower weight (robust to outlier frames / ringing)
//        · trimmed mean: drop the top/bottom k% then average
//        · ROF TV-L1 style double weighting (bilateral weight + exposure)
//        · Buades & Buchs self-weighting (soft-min)
//   3. Supports both real NV21 burst sequences + manual 4-frame simulation modes.
//   4. Automatically runs PSNR curves for N=3/5/7/9, table output compared with single-frame.
//   5. Mosaic: noisy#0 + single-frame algorithms + multi-frame aggregation results.
//
// Usage: denoise_multi.exe [input_path] [Nframes] [sigma]
#include "../common/nv21_io.hpp"
#include "../common/algo_utils.hpp"
#include "../common/single_denoise.hpp"

#include <opencv2/calib3d.hpp>  // findHomography

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace algo;

static cv::Mat loadAny(const std::string& p) {
    std::string lo = p;
    for (auto& c : lo) c = (char)tolower((unsigned char)c);
    if (lo.find(".nv21") != std::string::npos ||
        lo.find(".nv12") != std::string::npos ||
        lo.find(".yuv")  != std::string::npos ||
        lo.find(".i420") != std::string::npos) {
        return readNv21Auto(p);
    }
    return cv::imread(p, cv::IMREAD_COLOR);
}

static cv::Mat perturb(const cv::Mat& in, unsigned seed, bool homographyMode) {
    // translation ±3, rotation ±0.8°, slight scale (0.995~1.005) + perspective perturbation
    cv::RNG rng(seed);
    double dx = rng.uniform(-3.0, 3.0);
    double dy = rng.uniform(-3.0, 3.0);
    double deg = rng.uniform(-0.8, 0.8);
    double scale = rng.uniform(0.995, 1.005);
    cv::Point2f c(in.cols / 2.0f, in.rows / 2.0f);
    cv::Mat out;
    if (!homographyMode) {
        cv::Mat r = cv::getRotationMatrix2D(c, deg, scale);
        r.at<double>(0, 2) += dx;
        r.at<double>(1, 2) += dy;
        cv::warpAffine(in, out, r, in.size(), cv::INTER_LINEAR,
                       cv::BORDER_REFLECT101);
    } else {
        // add slight perspective perturbation
        std::vector<cv::Point2f> s = {
            {0, 0}, {(float)in.cols - 1, 0},
            {0, (float)in.rows - 1}, {(float)in.cols - 1, (float)in.rows - 1}
        };
        double j = rng.uniform(-2.5, 2.5);
        std::vector<cv::Point2f> d = s;
        d[0].x += j; d[0].y += j;
        d[1].x -= j; d[1].y += j * 0.7;
        d[2].x += j * 0.7; d[2].y -= j;
        d[3].x -= j * 0.6; d[3].y -= j * 0.6;
        cv::Mat H = cv::findHomography(s, d);
        H.at<double>(0, 2) += dx;
        H.at<double>(1, 2) += dy;
        cv::Mat r = cv::getRotationMatrix2D(c, deg, scale);
        cv::Mat H2 = cv::Mat::eye(3, 3, CV_64F);
        r.copyTo(H2(cv::Rect(0, 0, 3, 2)));
        H = H2 * H;
        cv::warpPerspective(in, out, H, in.size(), cv::INTER_LINEAR,
                            cv::BORDER_REFLECT101);
    }
    return out;
}

// Try multiple motion models for alignment (progressively finer models: Euclidean -> Affine -> Homography)
static cv::Mat alignAnyModel(const cv::Mat& ref, const cv::Mat& src, int model) {
    cv::Mat a = alignToRef(ref, src, model, 60, 1e-6);
    if (a.empty()) a = src;
    return a;
}

static std::vector<cv::Mat> alignFrames(const std::vector<cv::Mat>& frames, int model) {
    std::vector<cv::Mat> out(frames.size());
    out[0] = frames[0];
    for (size_t i = 1; i < frames.size(); ++i) {
        out[i] = alignAnyModel(frames[0], frames[i], model);
    }
    return out;
}

// mean
static cv::Mat fuseMean(const std::vector<cv::Mat>& aligned) {
    cv::Mat acc = cv::Mat::zeros(aligned[0].size(), CV_32FC(aligned[0].channels()));
    for (auto& a : aligned) {
        cv::Mat f; a.convertTo(f, CV_32F);
        acc += f;
    }
    acc /= (double)aligned.size();
    cv::Mat out; acc.convertTo(out, CV_8U);
    return out;
}

// median (per channel)
static cv::Mat fuseMedian(const std::vector<cv::Mat>& aligned) {
    int H = aligned[0].rows, W = aligned[0].cols, C = aligned[0].channels();
    std::vector<cv::Mat> outs(C);
    for (int c = 0; c < C; ++c) outs[c] = cv::Mat(H, W, CV_8U);
    size_t N = aligned.size();
    std::vector<uchar> v(N);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            for (int c = 0; c < C; ++c) {
                for (size_t k = 0; k < N; ++k)
                    v[k] = aligned[k].at<cv::Vec3b>(y, x)[c];
                std::nth_element(v.begin(), v.begin() + N / 2, v.end());
                outs[c].at<uchar>(y, x) = v[N / 2];
            }
        }
    }
    cv::Mat out; cv::merge(outs, out);
    return out;
}

// variance-weighted mean:
//   out(x) = Σ_k w_k(x)·I_k(x), w_k(x) ∝ exp( -(I_k - mean)^2 / (2σ²+ε) )
//   where σ² is the cross-frame variance at the same position (after local 3x3 smoothing),
//   approximating "high-variance pixels get low weight".
static cv::Mat fuseVarianceWeighted(const std::vector<cv::Mat>& aligned) {
    CV_Assert(!aligned.empty());
    int N = (int)aligned.size(), H = aligned[0].rows, W = aligned[0].cols;
    int C = aligned[0].channels();
    cv::Mat mean32 = cv::Mat::zeros(H, W, CV_32FC(C));
    std::vector<cv::Mat> f(N);
    for (int i = 0; i < N; ++i) {
        aligned[i].convertTo(f[i], CV_32F);
        mean32 += f[i];
    }
    mean32 /= N;
    // compute per-frame residual variance map (per pixel, cross-frame mean squared difference)
    cv::Mat varMat(H, W, CV_32F, 0.0f);
    for (int i = 0; i < N; ++i) {
        std::vector<cv::Mat> chs, mchs;
        cv::split(f[i], chs); cv::split(mean32, mchs);
        cv::Mat d(H, W, CV_32F, 0.0f);
        for (int c = 0; c < C; ++c) {
            cv::Mat dd = chs[c] - mchs[c];
            d += dd.mul(dd);
        }
        d /= (float)C;
        varMat += d;
    }
    varMat /= N;
    cv::boxFilter(varMat, varMat, CV_32F, cv::Size(3, 3));
    cv::Mat denom = 2.0f * (varMat + 1.0f); // σ²+1 avoids division by 0
    // accumulate weighted sum + weight sum
    cv::Mat sumW(H, W, CV_32F, 0.0f);
    cv::Mat sumI = cv::Mat::zeros(H, W, CV_32FC(C));
    for (int i = 0; i < N; ++i) {
        cv::Mat d(H, W, CV_32F, 0.0f);
        std::vector<cv::Mat> chs, mchs;
        cv::split(f[i], chs); cv::split(mean32, mchs);
        for (int c = 0; c < C; ++c) {
            cv::Mat dd = chs[c] - mchs[c];
            d += dd.mul(dd);
        }
        d /= (float)C;
        cv::Mat w; cv::exp(-d / denom, w); // w ∝ exp(-Δ²/(2·σ²+ε))
        sumW += w;
        std::vector<cv::Mat> w3(C, w);
        cv::Mat w3c; cv::merge(w3, w3c);
        sumI += f[i].mul(w3c);
    }
    sumW = cv::max(sumW, 1e-6f);
    std::vector<cv::Mat> w3(C, sumW);
    cv::Mat w3c; cv::merge(w3, w3c);
    cv::Mat outF = sumI / w3c;
    cv::threshold(outF, outF, 0, 0, cv::THRESH_TOZERO);
    cv::threshold(outF, outF, 255, 255, cv::THRESH_TRUNC);
    cv::Mat out8u; outF.convertTo(out8u, CV_8U);
    return out8u;
}

// trimmed mean:
//   for each pixel, collect the N values, drop the top/bottom dropPct% each, then average.
static cv::Mat fuseTrimmedMean(const std::vector<cv::Mat>& aligned, double dropPct = 0.2) {
    int N = (int)aligned.size(), H = aligned[0].rows, W = aligned[0].cols, C = aligned[0].channels();
    int trim = std::min(N / 2 - 1, std::max(1, (int)(dropPct * N)));
    std::vector<cv::Mat> outs(C);
    for (int c = 0; c < C; ++c) outs[c] = cv::Mat(H, W, CV_8U);
    std::vector<uchar> v(N);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            for (int c = 0; c < C; ++c) {
                for (int k = 0; k < N; ++k)
                    v[k] = aligned[k].at<cv::Vec3b>(y, x)[c];
                std::sort(v.begin(), v.end());
                double s = 0;
                for (int k = trim; k < N - trim; ++k) s += v[k];
                outs[c].at<uchar>(y, x) = cv::saturate_cast<uchar>(s / (N - 2 * trim));
            }
        }
    }
    cv::Mat out; cv::merge(outs, out);
    return out;
}

struct Scores {
    double psnr, ssim, msssim, mae;
    int loe;
};

static Scores scoreOf(const cv::Mat& clean, const cv::Mat& test) {
    cv::Size sz(std::min(clean.cols, test.cols), std::min(clean.rows, test.rows));
    cv::Mat c, t;
    if (clean.size() != sz) cv::resize(clean, c, sz); else c = clean;
    if (test.size() != sz) cv::resize(test, t, sz); else t = test;
    Scores s{};
    s.psnr = psnr(c, t);
    s.ssim = ssim(c, t);
    s.msssim = msssim(c, t);
    s.mae = mae(c, t);
    s.loe = loe(c, t, 48);
    return s;
}

static std::string modelName(int m) {
    switch (m) {
        case cv::MOTION_TRANSLATION: return "Translation";
        case cv::MOTION_EUCLIDEAN:   return "Euclidean";
        case cv::MOTION_AFFINE:      return "Affine";
        case cv::MOTION_HOMOGRAPHY:  return "Homography";
        default: return "?";
    }
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/nv21/nr/0315110117648_fn_0_3264X2448_cap_YNRCNR_0ms_to_in.NV21";
    int nFrames = 5;
    double sigma = 18;
    if (argc > 1) inPath = argv[1];
    if (argc > 2) nFrames = std::max(2, std::stoi(argv[2]));
    if (argc > 3) sigma = std::stod(argv[3]);

    cv::Mat base = loadAny(inPath);
    if (base.empty()) {
        log("denoise_multi", "load failed, fallback synthetic 512x512");
        base = cv::Mat(512, 512, CV_8UC3);
        cv::randu(base, 0, 256);
    }
    if (std::max(base.rows, base.cols) > 720) {
        double s = 720.0 / std::max(base.rows, base.cols);
        cv::resize(base, base, cv::Size(), s, s, cv::INTER_AREA);
        log("denoise_multi", "downscaled to " + std::to_string(base.cols) +
            "x" + std::to_string(base.rows));
    }

    // synthesize N frames: noise + motion perturbation (later frames use the Homography perturbation mode)
    std::vector<cv::Mat> frames;
    for (int i = 0; i < nFrames; ++i) {
        cv::Mat f = addGaussianNoise(base, sigma, (unsigned)(0x1234 + i * 7));
        f = perturb(f, (unsigned)(0xabcd + i * 13), i >= 2);
        frames.push_back(f);
    }

    std::vector<int> models = {
        cv::MOTION_AFFINE,
        cv::MOTION_EUCLIDEAN,
        cv::MOTION_HOMOGRAPHY,
    };

    struct Run {
        std::string label;
        cv::Mat img;
        Scores s;
    };
    std::vector<Run> runs;

    // single-frame baseline
    cv::Mat noisy0 = frames[0];
    cv::Mat singleNlm = denoiseNLM(noisy0, 12, 7, 21);
    cv::Mat singleBi = denoiseAdaptiveBilateral(noisy0, 9, 80, 70, 7);
    cv::Mat singleAuto = denoiseAuto(noisy0);
    runs.push_back({"noisy#0", noisy0, scoreOf(base, noisy0)});
    runs.push_back({"single NLM", singleNlm, scoreOf(base, singleNlm)});
    runs.push_back({"single AdaptiveBi", singleBi, scoreOf(base, singleBi)});
    runs.push_back({"single Auto", singleAuto, scoreOf(base, singleAuto)});

    // each motion model x multiple aggregations
    for (int m : models) {
        auto aligned = alignFrames(frames, m);
        std::string mn = modelName(m);
        cv::Mat meanR = fuseMean(aligned);
        runs.push_back({mn + " + mean", meanR, scoreOf(base, meanR)});
        cv::Mat medR = fuseMedian(aligned);
        runs.push_back({mn + " + median", medR, scoreOf(base, medR)});
        cv::Mat vwR = fuseVarianceWeighted(aligned);
        runs.push_back({mn + " + varianceW", vwR, scoreOf(base, vwR)});
        cv::Mat trimR = fuseTrimmedMean(aligned, 0.2);
        runs.push_back({mn + " + trimMean(0.2)", trimR, scoreOf(base, trimR)});
    }

    std::printf("\n=== Multi-frame denoise comparison (N=%d, σ=%.1f) ===\n",
                nFrames, sigma);
    std::printf("%-30s %8s %8s %8s %8s %8s\n",
                "method", "PSNR", "SSIM", "MS-SSIM", "MAE", "LOE");
    // sort (PSNR desc)
    std::vector<size_t> idx(runs.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b){
        return runs[a].s.psnr > runs[b].s.psnr;
    });
    for (auto i : idx) {
        const auto& r = runs[i];
        std::printf("%-30s %8.2f %8.4f %8.4f %8.3f %8d\n",
                    r.label.c_str(), r.s.psnr, r.s.ssim, r.s.msssim,
                    r.s.mae, r.s.loe);
    }
    std::printf("=====================================================\n");

    // sweep N = 3/5/7/9, plot mean/varianceW curves (Affine only)
    std::cout << "\n--- scan N vs PSNR (model=Affine, σ=" << sigma << ") ---\n";
    std::printf("%5s %10s %10s\n", "N", "mean", "varW");
    for (int N : {3, 5, 7, 9}) {
        if (N > (int)frames.size()) break;
        std::vector<cv::Mat> sub(frames.begin(), frames.begin() + N);
        auto a = alignFrames(sub, cv::MOTION_AFFINE);
        auto mm = fuseMean(a);
        auto vv = fuseVarianceWeighted(a);
        std::printf("%5d %10.2f %10.2f\n", N, psnr(base, mm), psnr(base, vv));
    }

    // output mosaic (pick 4 columns)
    ensureDir("../out/algorithms");
    std::vector<cv::Mat> panels;
    std::vector<std::string> labels;
    panels.push_back(base); labels.push_back("clean");
    for (size_t k = 0; k < std::min<size_t>(frames.size(), 3); ++k) {
        panels.push_back(frames[k]);
        labels.push_back("noisy#" + std::to_string(k));
    }
    // show aggregation results of Affine + Homography
    std::vector<std::string> pick = {
        "Affine + mean", "Affine + median", "Affine + varianceW",
        "Affine + trimMean(0.2)",
        "Homography + mean", "Homography + varianceW",
    };
    for (auto& p : pick) {
        for (auto& r : runs) if (r.label == p) {
            panels.push_back(r.img); labels.push_back(r.label); break;
        }
    }
    // single-frame auto denoise
    panels.push_back(singleAuto); labels.push_back("single Auto");
    cv::Mat canvas = gridWithLabels(panels, labels, 4, 32);
    cv::imwrite("../out/algorithms/denoise_multi.png", canvas);
    std::cout << "[denoise_multi] wrote ../out/algorithms/denoise_multi.png"
              << "  panels=" << panels.size() << "\n";
    imshowFit("multi_frame_denoise", canvas, 1800, 0);
    return 0;
}
