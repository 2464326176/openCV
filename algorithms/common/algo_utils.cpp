// opencvAlgoDev/common/algo_utils.cpp
#include "algo_utils.hpp"
#include "nv21_io.hpp" // 共享日志风格

#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <dirent.h>
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <sstream>

namespace algo {

// =========================================================================
// ---- 数值范围 ------------------------------------------------------------
// =========================================================================

cv::Mat toFloat(const cv::Mat& src) {
    CV_Assert(!src.empty());
    cv::Mat f;
    src.convertTo(f, CV_32F);
    if (src.depth() == CV_8U) f *= 1.0 / 255.0;
    return f;
}

cv::Mat to8U(const cv::Mat& src, double scale) {
    CV_Assert(!src.empty());
    cv::Mat s;
    src.convertTo(s, CV_32F, scale); // 先乘 scale
    cv::threshold(s, s, 255.0, 255.0, cv::THRESH_TRUNC);
    cv::threshold(s, s, 0.0, 0.0, cv::THRESH_TOZERO);
    cv::Mat out;
    s.convertTo(out, CV_8U);
    return out;
}

cv::Mat normalizeTo01(const cv::Mat& src) {
    CV_Assert(!src.empty());
    cv::Mat f;
    src.convertTo(f, CV_32F);
    double mn, mx;
    cv::minMaxLoc(f.reshape(1, 1), &mn, &mx);
    double rng = mx - mn;
    if (rng < 1e-9) return cv::Mat::zeros(f.size(), f.type());
    return (f - mn) / rng;
}

// =========================================================================
// ---- 全参考 IQA ---------------------------------------------------------
// =========================================================================

double mse(const cv::Mat& a, const cv::Mat& b) {
    CV_Assert(a.size() == b.size() && a.type() == b.type());
    cv::Mat sse;
    cv::absdiff(a, b, sse);
    sse.convertTo(sse, CV_32F);
    sse = sse.mul(sse);
    cv::Scalar s = cv::sum(sse);
    double total = s[0] + s[1] + s[2] + s[3];
    return total / (double)(a.total() * a.channels());
}

double mae(const cv::Mat& a, const cv::Mat& b) {
    CV_Assert(a.size() == b.size() && a.type() == b.type());
    cv::Mat d;
    cv::absdiff(a, b, d);
    d.convertTo(d, CV_32F);
    cv::Scalar s = cv::mean(d);
    return (s[0] + s[1] + s[2] + s[3]) / 4.0;
}

double psnr(const cv::Mat& a, const cv::Mat& b) {
    double m = mse(a, b);
    if (m <= 1e-10) return 1000.0; // 完全相同
    return 10.0 * std::log10((255.0 * 255.0) / m);
}

static double ssimChannelInternal(const cv::Mat& a, const cv::Mat& b) {
    // 简化 SSIM: 用 11x11 高斯窗口 (sigma=1.5) 估算均值/方差/协方差.
    const double C1 = 6.5025, C2 = 58.5225;
    cv::Mat a1, b1;
    a.convertTo(a1, CV_32F);
    b.convertTo(b1, CV_32F);
    cv::Mat mu1, mu2, mu1_sq, mu2_sq, mu1_mu2;
    cv::GaussianBlur(a1, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(b1, mu2, cv::Size(11, 11), 1.5);
    mu1_sq = mu1.mul(mu1);
    mu2_sq = mu2.mul(mu2);
    mu1_mu2 = mu1.mul(mu2);
    cv::Mat a_sq = a1.mul(a1), b_sq = b1.mul(b1), ab = a1.mul(b1);
    cv::Mat sigma1_sq, sigma2_sq, sigma12;
    cv::GaussianBlur(a_sq, sigma1_sq, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(b_sq, sigma2_sq, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(ab, sigma12, cv::Size(11, 11), 1.5);
    sigma1_sq -= mu1_sq;
    sigma2_sq -= mu2_sq;
    sigma12 -= mu1_mu2;
    cv::Mat num = (2 * mu1_mu2 + C1).mul(2 * sigma12 + C2);
    cv::Mat den = (mu1_sq + mu2_sq + C1).mul(sigma1_sq + sigma2_sq + C2);
    cv::Mat smap;
    cv::divide(num, den, smap);
    cv::Scalar m = cv::mean(smap);
    return m[0];
}

double ssim(const cv::Mat& a, const cv::Mat& b) {
    CV_Assert(a.size() == b.size() && a.type() == b.type());
    if (a.channels() == 1) return ssimChannelInternal(a, b);
    std::vector<cv::Mat> ca, cb;
    cv::split(a, ca);
    cv::split(b, cb);
    double sum = 0;
    for (int i = 0; i < a.channels(); ++i) sum += ssimChannelInternal(ca[i], cb[i]);
    return sum / a.channels();
}

// MS-SSIM: 多层 SSIM 加权, 权重近似 Wang 2003 推荐值.
double msssim(const cv::Mat& a, const cv::Mat& b, int levels) {
    CV_Assert(a.size() == b.size() && a.type() == b.type());
    // 每层权重 (level 0=fine). 典型 3 层: {0.0448, 0.2856, 0.3001}
    static const double wTbl[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
    std::vector<double> weights;
    int L = std::max(1, std::min(5, levels));
    for (int i = 0; i < L; ++i) weights.push_back(wTbl[i]);
    // 归一化权重
    double wsum = 0; for (auto x : weights) wsum += x;
    for (auto& x : weights) x /= wsum;

    std::vector<cv::Mat> pa, pb;
    pa.push_back(a); pb.push_back(b);
    for (int l = 1; l < L; ++l) {
        cv::Mat da, db;
        cv::pyrDown(pa.back(), da);
        cv::pyrDown(pb.back(), db);
        pa.push_back(da); pb.push_back(db);
    }
    // 最粗层用完整 SSIM (包含 luminance), 其余层只用 contrast * structure.
    double score = 1.0;
    for (int l = L - 1; l >= 0; --l) {
        double sl = ssim(pa[l], pb[l]);
        sl = std::max(1e-6, std::min(1.0, sl));
        score *= std::pow(sl, weights[l]);
    }
    return score;
}

// LOE: 对 samples 个随机像素, 统计增强前后 "与 N×N 邻域像素亮度顺序被颠倒" 的次数.
int loe(const cv::Mat& before, const cv::Mat& after, int samples) {
    cv::Mat g1, g2;
    if (before.channels() == 3) cv::cvtColor(before, g1, cv::COLOR_BGR2GRAY);
    else g1 = before;
    if (after.channels() == 3) cv::cvtColor(after, g2, cv::COLOR_BGR2GRAY);
    else g2 = after;
    CV_Assert(g1.size() == g2.size());
    int H = g1.rows, W = g1.cols;
    int R = std::max(3, std::min(H, W) / 32); // 局部邻域半径
    cv::RNG rng(0xC0FFEEu);
    int errors = 0;
    for (int k = 0; k < samples; ++k) {
        int x = rng.uniform(0, W);
        int y = rng.uniform(0, H);
        int x0 = std::max(0, x - R), x1 = std::min(W - 1, x + R);
        int y0 = std::max(0, y - R), y1 = std::min(H - 1, y + R);
        uchar v1 = g1.at<uchar>(y, x);
        uchar v2 = g2.at<uchar>(y, x);
        for (int yy = y0; yy <= y1; ++yy) {
            for (int xx = x0; xx <= x1; ++xx) {
                uchar u1 = g1.at<uchar>(yy, xx);
                uchar u2 = g2.at<uchar>(yy, xx);
                int r1 = (v1 > u1) ? 1 : (v1 < u1 ? -1 : 0);
                int r2 = (v2 > u2) ? 1 : (v2 < u2 ? -1 : 0);
                if (r1 != r2) ++errors;
            }
        }
    }
    return errors;
}

// =========================================================================
// ---- 无参考 IQA ---------------------------------------------------------
// =========================================================================

static double percentile(std::vector<double>& sorted, double p) {
    if (sorted.empty()) return 0;
    double idx = p * (double)(sorted.size() - 1);
    int lo = (int)idx, hi = std::min((int)sorted.size() - 1, lo + 1);
    double f = idx - lo;
    return sorted[lo] * (1 - f) + sorted[hi] * f;
}

BrightStats brightnessStats(const cv::Mat& bgr) {
    cv::Mat g;
    if (bgr.channels() == 3) cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
    else g = bgr;
    cv::Scalar mu, sd;
    cv::meanStdDev(g, mu, sd);
    // 为分位采样 (稀疏 10k 点够了)
    int H = g.rows, W = g.cols;
    int N = std::min((int)g.total(), 10000);
    std::vector<double> vals(N);
    cv::RNG rng(0xFACEu);
    for (int i = 0; i < N; ++i) {
        int x = rng.uniform(0, W);
        int y = rng.uniform(0, H);
        vals[i] = (double)g.at<uchar>(y, x);
    }
    std::sort(vals.begin(), vals.end());
    BrightStats bs;
    bs.mean = mu[0]; bs.std = sd[0];
    bs.per05 = percentile(vals, 0.05);
    bs.per95 = percentile(vals, 0.95);
    return bs;
}

double saturationMean(const cv::Mat& bgr) {
    if (bgr.channels() != 3) return 0.0;
    cv::Mat yc;
    cv::cvtColor(bgr, yc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> chs; cv::split(yc, chs);
    cv::Mat cr, cb;
    chs[1].convertTo(cr, CV_32F, 1.0 / 255.0);
    chs[2].convertTo(cb, CV_32F, 1.0 / 255.0);
    // 中心化 (YCbCr 中点 = 128/255)
    cr = cr - 0.50196; cb = cb - 0.50196;
    cv::Mat mag; cv::magnitude(cr, cb, mag);
    // 放大 2×, 使纯白 0 → 纯饱和色 → 1.0 附近
    cv::Scalar s = cv::mean(mag);
    return std::min(1.0, s[0] * 2.0);
}

double imageEntropy(const cv::Mat& gray) {
    CV_Assert(!gray.empty() && gray.depth() == CV_8U);
    cv::Mat hist;
    int hsize = 256; float range[] = {0, 256}; const float* rngs = range;
    cv::calcHist(&gray, 1, nullptr, cv::Mat(), hist, 1, &hsize, &rngs);
    double total = gray.total();
    double H = 0;
    for (int i = 0; i < 256; ++i) {
        double p = hist.at<float>(i) / total;
        if (p > 1e-10) H += -p * std::log2(p);
    }
    return H;
}

double eme(const cv::Mat& gray, int blk) {
    CV_Assert(!gray.empty() && gray.depth() == CV_8U);
    int H = gray.rows, W = gray.cols;
    int KB = std::max(1, W / blk);
    int KR = std::max(1, H / blk);
    double sum = 0.0; int cnt = 0;
    const double eps = 3.0;
    for (int r = 0; r < KR; ++r) {
        for (int c = 0; c < KB; ++c) {
            cv::Rect roi(c * blk, r * blk, blk, blk);
            if (roi.x + roi.width > W || roi.y + roi.height > H) continue;
            double mn, mx;
            cv::minMaxLoc(gray(roi), &mn, &mx);
            double d = (mx + eps) / (mn + eps);
            if (d < 1) d = 1.0 / d;
            sum += 20.0 * std::log10(d);
            ++cnt;
        }
    }
    return cnt ? sum / cnt : 0.0;
}

double niqeScoreApprox(const cv::Mat& gray) {
    // NIQE 的简化近似: 用 5×5 局部均值方差 + 局部偏度作为特征,
    // 与"理想自然图像"的经验特征 (Mu=128,Sigma=30, Skew=0.1) 做 Mahalanobis.
    // 这不是完整 NIQE (后者需要 36×M 空间域特征 + GGD 拟合), 只作为趋势估计.
    CV_Assert(!gray.empty() && gray.depth() == CV_8U);
    cv::Mat f; gray.convertTo(f, CV_32F, 1.0 / 255.0);
    cv::Mat mu, sq = f.mul(f), musq;
    cv::GaussianBlur(f, mu, cv::Size(5, 5), 1.0);
    cv::GaussianBlur(sq, musq, cv::Size(5, 5), 1.0);
    cv::Mat var = musq - mu.mul(mu);
    var = cv::max(var, 0);
    cv::Scalar muVar, sdVar;
    cv::meanStdDev(var, muVar, sdVar);
    // skew (三阶矩)
    cv::Mat d1 = f - mu;
    cv::Mat d3 = d1.mul(d1).mul(d1);
    cv::Scalar sk = cv::mean(d3);
    double mvar = muVar[0], svar = sdVar[0], skew = sk[0];
    // 经验"自然图像"目标
    double tMuVar = 0.015, tSVar = 0.01, tSkew = 0.02;
    double d1v = (mvar - tMuVar) / std::max(1e-6, 0.01);
    double d2v = (svar - tSVar) / std::max(1e-6, 0.01);
    double d3v = (skew - tSkew) / std::max(1e-6, 0.05);
    return std::sqrt(d1v * d1v + d2v * d2v + d3v * d3v);
}

NRQuality nrQuality(const cv::Mat& bgr) {
    NRQuality q;
    cv::Mat g;
    if (bgr.channels() == 3) cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
    else g = bgr;
    q.entropy = imageEntropy(g);
    q.saturation = saturationMean(bgr);
    q.eme = eme(g, 8);
    q.niqeApprox = niqeScoreApprox(g);
    return q;
}

// =========================================================================
// ---- 图像增强工具 --------------------------------------------------------
// =========================================================================

cv::Mat gammaLUT(const cv::Mat& src, double gamma) {
    CV_Assert(!src.empty());
    cv::Mat lut(1, 256, CV_8U);
    for (int i = 0; i < 256; ++i)
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(
            std::pow(i / 255.0, gamma) * 255.0);
    cv::Mat out;
    cv::LUT(src, lut, out);
    return out;
}

static double percentileOfMat8U(const cv::Mat& ch, double p) {
    std::vector<uchar> tmp;
    tmp.reserve(ch.total());
    if (ch.isContinuous()) {
        tmp.assign(ch.datastart, ch.dataend);
    } else {
        for (int y = 0; y < ch.rows; ++y)
            for (int x = 0; x < ch.cols; ++x) tmp.push_back(ch.at<uchar>(y, x));
    }
    size_t idx = (size_t)(p * (double)(tmp.size() - 1));
    std::nth_element(tmp.begin(), tmp.begin() + idx, tmp.end());
    return (double)tmp[idx];
}

cv::Mat autoContrast1pct(const cv::Mat& bgr) {
    if (bgr.channels() != 3 || bgr.depth() != CV_8U) return bgr.clone();
    std::vector<cv::Mat> chs; cv::split(bgr, chs);
    std::vector<cv::Mat> outs;
    for (auto& c : chs) {
        double lo = percentileOfMat8U(c, 0.01);
        double hi = percentileOfMat8U(c, 0.99);
        double rng = hi - lo;
        if (rng < 1) { outs.push_back(c.clone()); continue; }
        cv::Mat lut(1, 256, CV_8U);
        for (int i = 0; i < 256; ++i) {
            double v = (i - lo) * 255.0 / rng;
            v = std::max(0.0, std::min(255.0, v));
            lut.at<uchar>(i) = (uchar)v;
        }
        cv::Mat o; cv::LUT(c, lut, o); outs.push_back(o);
    }
    cv::Mat out; cv::merge(outs, out); return out;
}

cv::Mat equalizeHistogramY(const cv::Mat& bgr) {
    if (bgr.channels() != 3) return bgr.clone();
    cv::Mat yc; cv::cvtColor(bgr, yc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> chs; cv::split(yc, chs);
    cv::equalizeHist(chs[0], chs[0]);
    cv::merge(chs, yc);
    cv::Mat out; cv::cvtColor(yc, out, cv::COLOR_YCrCb2BGR);
    return out;
}

cv::Mat simpleColorBalance(const cv::Mat& bgr, double pct) {
    if (bgr.channels() != 3) return bgr.clone();
    std::vector<cv::Mat> chs; cv::split(bgr, chs);
    std::vector<cv::Mat> outs;
    for (auto& c : chs) {
        double lo = percentileOfMat8U(c, pct);
        double hi = percentileOfMat8U(c, 1.0 - pct);
        double rng = hi - lo;
        if (rng < 1) { outs.push_back(c.clone()); continue; }
        cv::Mat lut(1, 256, CV_8U);
        for (int i = 0; i < 256; ++i) {
            double v = (i - lo) * 255.0 / rng;
            v = std::max(0.0, std::min(255.0, v));
            lut.at<uchar>(i) = (uchar)v;
        }
        cv::Mat o; cv::LUT(c, lut, o); outs.push_back(o);
    }
    cv::Mat out; cv::merge(outs, out); return out;
}

cv::Vec3f estimateWhitePointGrayWorld(const cv::Mat& bgr) {
    CV_Assert(bgr.channels() == 3);
    // Gray-world: 假设 mean(B)=mean(G)=mean(R)=gray; Max-RGB 归一化混合.
    cv::Scalar m = cv::mean(bgr); // B,G,R mean
    float b = (float)m[0], g = (float)m[1], r = (float)m[2];
    // 灰度世界假设下, gray=mean(B,G,R) 对应白点
    float gm = (b + g + r) / 3.0f;
    // 归一化: 通道均值越暗, white 越小, 放大系数越大
    float wb = (b > 1) ? gm / b : 1.0f;
    float wg = (g > 1) ? gm / g : 1.0f;
    float wr = (r > 1) ? gm / r : 1.0f;
    // 抑制极端值 (clip 到 0.5 ~ 2.0)
    wb = std::max(0.5f, std::min(2.0f, wb));
    wg = std::max(0.5f, std::min(2.0f, wg));
    wr = std::max(0.5f, std::min(2.0f, wr));
    // 以 1/maxChannel 归一化到 [0,1]: 输出"白点在图像中的灰度值"
    float mxc = std::max({wb, wg, wr});
    return cv::Vec3f(wr / mxc, wg / mxc, wb / mxc); // Vec3f 语义: R,G,B
}

cv::Mat whiteBalanceFromPoint(const cv::Mat& bgr, const cv::Vec3f& white, double clip) {
    // white 是 (R,G,B) ∈ [0,1], 目标: out_B/G/R = in_* * (1/white_*) * scale,
    // 令 scale = max(white_*) 让最大通道不放大, 避免过曝.
    float wr = white[0], wg = white[1], wb = white[2];
    float mx = std::max({wr, wg, wb, 1e-3f});
    float kb = mx / std::max(1e-3f, wb);
    float kg = mx / std::max(1e-3f, wg);
    float kr = mx / std::max(1e-3f, wr);
    kb = std::max(0.1f, std::min((float)clip, kb));
    kg = std::max(0.1f, std::min((float)clip, kg));
    kr = std::max(0.1f, std::min((float)clip, kr));
    std::vector<cv::Mat> chs; cv::split(bgr, chs);
    chs[0].convertTo(chs[0], CV_32F, kb);
    chs[1].convertTo(chs[1], CV_32F, kg);
    chs[2].convertTo(chs[2], CV_32F, kr);
    cv::Mat merged; cv::merge(chs, merged);
    cv::threshold(merged, merged, 255.0, 255.0, cv::THRESH_TRUNC);
    cv::Mat out; merged.convertTo(out, CV_8U);
    return out;
}

// =========================================================================
// ---- 可视化 --------------------------------------------------------------
// =========================================================================

static cv::Mat ensureBGR(const cv::Mat& m) {
    if (m.empty()) return cv::Mat();
    if (m.channels() == 1) {
        cv::Mat b; cv::cvtColor(m, b, cv::COLOR_GRAY2BGR); return b;
    }
    return m;
}

cv::Mat hstackWithLabels(const std::vector<cv::Mat>& imgs,
                         const std::vector<std::string>& labels,
                         int labelHeight) {
    CV_Assert(imgs.size() == labels.size() && !imgs.empty());
    std::vector<cv::Mat> convs;
    for (auto& m : imgs) convs.push_back(ensureBGR(m));
    int maxRows = 0;
    for (auto& c : convs) maxRows = std::max(maxRows, c.rows);
    int totalW = 0;
    for (auto& c : convs) totalW += c.cols;
    int canvasH = maxRows + labelHeight;
    cv::Mat canvas = cv::Mat::zeros(canvasH, totalW, CV_8UC3);
    int x = 0;
    for (size_t i = 0; i < convs.size(); ++i) {
        const auto& c = convs[i];
        cv::Rect roi(x, labelHeight, c.cols, c.rows);
        if (roi.x + roi.width > canvas.cols) break;
        c.copyTo(canvas(roi));
        cv::rectangle(canvas, cv::Rect(x, 0, c.cols, labelHeight),
                      cv::Scalar(50, 50, 50), -1);
        // 分色条: 每图顶部一条彩色分割线 (增强可读性)
        cv::Scalar accent((i * 97) % 200 + 30, (i * 197) % 200 + 30, (i * 311) % 200 + 30);
        cv::rectangle(canvas, cv::Rect(x, labelHeight - 3, c.cols, 3), accent, -1);
        cv::putText(canvas, labels[i], cv::Point(x + 6, labelHeight - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255),
                    1, cv::LINE_AA);
        x += c.cols;
    }
    return canvas;
}

cv::Mat gridWithLabels(const std::vector<cv::Mat>& imgs,
                        const std::vector<std::string>& labels,
                        int cols, int labelHeight) {
    CV_Assert(imgs.size() == labels.size() && !imgs.empty());
    std::vector<cv::Mat> convs;
    for (auto& m : imgs) convs.push_back(ensureBGR(m));
    int N = (int)convs.size();
    cols = std::max(1, std::min(cols, N));
    int rows = (N + cols - 1) / cols;
    // 每列统一宽度, 每行统一高度
    std::vector<int> colW(cols, 0), rowH(rows, 0);
    for (int i = 0; i < N; ++i) {
        int r = i / cols, c = i % cols;
        colW[c] = std::max(colW[c], convs[i].cols);
        rowH[r] = std::max(rowH[r], convs[i].rows);
    }
    int totalW = 0, totalH = 0;
    for (int c = 0; c < cols; ++c) totalW += colW[c];
    for (int r = 0; r < rows; ++r) totalH += (rowH[r] + labelHeight);
    cv::Mat canvas = cv::Mat::zeros(totalH, totalW, CV_8UC3);
    int y = 0;
    for (int r = 0; r < rows; ++r) {
        int x = 0;
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            if (idx >= N) break;
            cv::Rect header(x, y, colW[c], labelHeight);
            cv::rectangle(canvas, header, cv::Scalar(55, 55, 55), -1);
            cv::Scalar accent((idx * 97) % 200 + 30, (idx * 197) % 200 + 30, (idx * 311) % 200 + 30);
            cv::rectangle(canvas,
                          cv::Rect(x, y + labelHeight - 3, colW[c], 3), accent, -1);
            cv::putText(canvas, labels[idx],
                        cv::Point(x + 6, y + labelHeight - 8),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255),
                        1, cv::LINE_AA);
            // 把 img 放到 tile 的左上角 (不足部分留黑)
            cv::Rect tile(x, y + labelHeight, convs[idx].cols, convs[idx].rows);
            convs[idx].copyTo(canvas(tile));
            x += colW[c];
        }
        y += (rowH[r] + labelHeight);
    }
    return canvas;
}

cv::Mat attachHeatmapBelow(const cv::Mat& top, const cv::Mat& grayMap,
                            const std::string& label) {
    cv::Mat t = ensureBGR(top);
    cv::Mat gm;
    if (grayMap.channels() != 1) cv::cvtColor(grayMap, gm, cv::COLOR_BGR2GRAY);
    else gm = grayMap.clone();
    if (gm.size() != t.size()) cv::resize(gm, gm, t.size());
    cv::Mat colorMap;
    cv::applyColorMap(gm, colorMap, cv::COLORMAP_JET);
    int lh = 30;
    int H = t.rows + lh + colorMap.rows;
    int W = std::max(t.cols, colorMap.cols);
    cv::Mat canvas = cv::Mat::zeros(H, W, CV_8UC3);
    t.copyTo(canvas(cv::Rect(0, 0, t.cols, t.rows)));
    int y = t.rows;
    cv::rectangle(canvas, cv::Rect(0, y, W, lh), cv::Scalar(45, 45, 45), -1);
    cv::putText(canvas, label, cv::Point(8, y + lh - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 200, 255), 1, cv::LINE_AA);
    y += lh;
    colorMap.copyTo(canvas(cv::Rect(0, y, colorMap.cols, colorMap.rows)));
    return canvas;
}

void imshowFit(const std::string& win, const cv::Mat& img, int maxEdge, int delay) {
    if (img.empty()) return;
    int maxDim = std::max(img.rows, img.cols);
    double s = maxDim > maxEdge ? (double)maxEdge / maxDim : 1.0;
    cv::Mat out;
    cv::resize(img, out, cv::Size(), s, s, cv::INTER_AREA);
    cv::namedWindow(win, cv::WINDOW_AUTOSIZE);
    cv::imshow(win, out);
    if (delay >= 0) cv::waitKey(delay);
}

void log(const std::string& tag, const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
#ifdef _WIN32
    localtime_s(&bt, &t);
#else
    localtime_r(&t, &bt);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &bt);
    std::fprintf(stdout, "[%s][algo][%s] %s\n", buf, tag.c_str(), msg.c_str());
    std::fflush(stdout);
}

std::string formatFRIQATable(
    const std::vector<std::pair<std::string, std::pair<double, double>>>& rows) {
    std::ostringstream os;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%-18s %8s %8s\n", "method", "PSNR", "SSIM");
    os << buf;
    for (auto& kv : rows) {
        std::snprintf(buf, sizeof(buf), "%-18s %8.2f %8.4f\n",
                      kv.first.c_str(), kv.second.first, kv.second.second);
        os << buf;
    }
    return os.str();
}

std::string formatNRIQATable(
    const std::vector<std::pair<std::string, NRQuality>>& rows) {
    std::ostringstream os;
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "%-18s %8s %8s %8s %8s\n", "method", "entropy", "sat", "EME", "niqe~");
    os << buf;
    for (auto& kv : rows) {
        const auto& q = kv.second;
        std::snprintf(buf, sizeof(buf),
            "%-18s %8.3f %8.3f %8.2f %8.3f\n",
            kv.first.c_str(), q.entropy, q.saturation, q.eme, q.niqeApprox);
        os << buf;
    }
    return os.str();
}

// =========================================================================
// ---- 配准 ----------------------------------------------------------------
// =========================================================================

static cv::Mat preprocessForECC(const cv::Mat& gray) {
    cv::Mat g;
    gray.convertTo(g, CV_32F, 1.0 / 255.0);
    // 简单 CLAHE (在 CV_32F 上近似, 用 normalize + 32bit equalize 替代)
    cv::Mat g8u; g.convertTo(g8u, CV_8U, 255.0);
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    cv::Mat equ; clahe->apply(g8u, equ);
    equ.convertTo(equ, CV_32F, 1.0 / 255.0);
    return equ;
}

cv::Mat alignECC(const cv::Mat& ref, const cv::Mat& src, int motionType,
                 int iters, double eps, bool preCLAHE) {
    CV_Assert(!ref.empty() && !src.empty());
    cv::Mat gref, gsrc;
    if (ref.channels() == 3) cv::cvtColor(ref, gref, cv::COLOR_BGR2GRAY);
    else gref = ref.clone();
    if (src.channels() == 3) cv::cvtColor(src, gsrc, cv::COLOR_BGR2GRAY);
    else gsrc = src.clone();

    cv::Mat rIn, sIn;
    if (preCLAHE) {
        rIn = preprocessForECC(gref);
        sIn = preprocessForECC(gsrc);
    } else {
        gref.convertTo(rIn, CV_32F, 1.0 / 255.0);
        gsrc.convertTo(sIn, CV_32F, 1.0 / 255.0);
    }
    cv::Mat warp;
    if (motionType == cv::MOTION_HOMOGRAPHY) {
        warp = cv::Mat::eye(3, 3, CV_32F);
    } else if (motionType == cv::MOTION_TRANSLATION) {
        warp = cv::Mat::zeros(2, 1, CV_32F);
    } else {
        // MOTION_EUCLIDEAN / MOTION_AFFINE 都是 2×3
        warp = cv::Mat::eye(2, 3, CV_32F);
    }
    cv::TermCriteria tc(cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
                        iters, eps);
    try {
        cv::findTransformECC(rIn, sIn, warp, motionType, tc);
    } catch (const cv::Exception& e) {
        log("alignECC", std::string("failed: ") + e.what());
        return cv::Mat();
    }
    return warp;
}

cv::Mat alignToRef(const cv::Mat& ref, const cv::Mat& src, int motionType,
                   int iters, double eps, bool preCLAHE) {
    cv::Mat warp = alignECC(ref, src, motionType, iters, eps, preCLAHE);
    if (warp.empty()) return src.clone();
    cv::Mat out;
    if (motionType == cv::MOTION_HOMOGRAPHY) {
        cv::warpPerspective(src, out, warp, ref.size());
    } else if (motionType == cv::MOTION_TRANSLATION) {
        cv::Mat M = cv::Mat::eye(2, 3, CV_32F);
        M.at<float>(0, 2) = warp.at<float>(0, 0);
        M.at<float>(1, 2) = warp.at<float>(1, 0);
        cv::warpAffine(src, out, M, ref.size());
    } else {
        cv::warpAffine(src, out, warp, ref.size());
    }
    return out;
}

cv::Mat matchLuminanceStats(const cv::Mat& ref, const cv::Mat& src) {
    CV_Assert(!ref.empty() && !src.empty());
    cv::Mat ycRef, ycSrc;
    if (ref.channels() == 3) cv::cvtColor(ref, ycRef, cv::COLOR_BGR2YCrCb);
    else ycRef = ref.clone();
    if (src.channels() == 3) cv::cvtColor(src, ycSrc, cv::COLOR_BGR2YCrCb);
    else ycSrc = src.clone();
    std::vector<cv::Mat> chsR, chsS;
    cv::split(ycRef, chsR);
    cv::split(ycSrc, chsS);
    cv::Scalar mr, sr, ms, ss;
    cv::meanStdDev(chsR[0], mr, sr);
    cv::meanStdDev(chsS[0], ms, ss);
    double k = (sr[0] > 1e-3) ? (sr[0] / std::max(1e-3, ss[0])) : 1.0;
    double b = mr[0] - k * ms[0];
    cv::Mat newY;
    chsS[0].convertTo(newY, CV_32F, k, b);
    cv::threshold(newY, newY, 255.0, 255.0, cv::THRESH_TRUNC);
    cv::threshold(newY, newY, 0.0, 0.0, cv::THRESH_TOZERO);
    newY.convertTo(chsS[0], CV_8U);
    cv::merge(chsS, ycSrc);
    if (src.channels() == 3) {
        cv::Mat out; cv::cvtColor(ycSrc, out, cv::COLOR_YCrCb2BGR);
        return out;
    }
    return chsS[0];
}

// =========================================================================
// ---- 文件工具 ------------------------------------------------------------
// =========================================================================

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    char last = a.back();
    char first = b.front();
    if (last == '/' || last == '\\') {
        if (first == '/' || first == '\\') return a + b.substr(1);
        return a + b;
    } else {
        if (first == '/' || first == '\\') return a + b;
        return a + "/" + b;
    }
}

static std::string lowerStr(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

static bool makeDirHelper(const std::string& dir) {
    if (dir.empty()) return false;
#ifdef _WIN32
    return CreateDirectoryA(dir.c_str(), nullptr) ||
           GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(dir.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

bool ensureDir(const std::string& dir) {
    if (dir.empty()) return false;
    // 递归逐级创建
    std::string p = dir;
    for (auto& c : p) if (c == '\\') c = '/';
    std::vector<std::string> parts;
    std::string cur;
    for (size_t i = 0; i < p.size(); ++i) {
        if (p[i] == '/') {
            if (!cur.empty()) parts.push_back(cur);
            cur.clear();
        } else cur.push_back(p[i]);
    }
    if (!cur.empty()) parts.push_back(cur);
    std::string build;
    // Windows 盘符保留
    if (!parts.empty() && parts[0].size() == 2 && parts[0][1] == ':') {
        build = parts[0] + "/";
        for (size_t i = 1; i < parts.size(); ++i) {
            build += parts[i];
            if (!makeDirHelper(build)) return false;
            build += "/";
        }
        return true;
    }
    // 绝对路径 Unix / Windows UNC 前缀都当 / 开头
    bool isAbs = (p[0] == '/' || p[0] == '\\');
    if (isAbs) build = "/";
    for (size_t i = 0; i < parts.size(); ++i) {
        build += parts[i];
        if (!makeDirHelper(build)) return false;
        build += "/";
    }
    return true;
}

std::vector<std::string> listFiles(const std::string& dir,
                                    const std::string& extFilter) {
    std::vector<std::string> out;
#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    std::string pattern = dir + "\\*";
    HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return out;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        std::string name = fd.cFileName;
        if (!extFilter.empty()) {
            std::string lo = lowerStr(name);
            std::string ef = lowerStr(extFilter);
            if (lo.find(ef) == std::string::npos) continue;
        }
        out.push_back(join(dir, name));
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR* d = opendir(dir.c_str());
    if (!d) return out;
    struct dirent* e;
    while ((e = readdir(d))) {
        if (e->d_type == DT_DIR) continue;
        std::string name = e->d_name;
        if (!extFilter.empty()) {
            std::string lo = lowerStr(name);
            std::string ef = lowerStr(extFilter);
            if (lo.find(ef) == std::string::npos) continue;
        }
        out.push_back(join(dir, name));
    }
    closedir(d);
#endif
    std::sort(out.begin(), out.end());
    return out;
}

std::string baseNameNoExt(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = name.find_last_of('.');
    if (dot == std::string::npos) return name;
    return name.substr(0, dot);
}

// =========================================================================
// ---- 分块迭代 ------------------------------------------------------------
// =========================================================================

void processTiled(const cv::Mat& src, cv::Mat& dst, int blk, int overlap,
                  std::function<cv::Mat(const cv::Mat&)> fn) {
    CV_Assert(!src.empty());
    int H = src.rows, W = src.cols;
    int C = src.channels();
    dst = cv::Mat::zeros(H, W, CV_MAKE_TYPE(CV_32F, C));
    cv::Mat wSum = cv::Mat::zeros(H, W, CV_32F);
    int step = std::max(1, blk - 2 * overlap);
    for (int y = 0; y < H; y += step) {
        for (int x = 0; x < W; x += step) {
            int x0 = std::max(0, x - overlap);
            int y0 = std::max(0, y - overlap);
            int x1 = std::min(W, x + blk + overlap);
            int y1 = std::min(H, y + blk + overlap);
            cv::Rect roi(x0, y0, x1 - x0, y1 - y0);
            cv::Mat tile = src(roi).clone();
            cv::Mat processed = fn(tile);
            // 裁剪回有效范围 (去掉 padding 的 overlap 部分以避免边缘伪影)
            int ox = x - x0, oy = y - y0;
            int ww = std::min(blk, W - x);
            int hh = std::min(blk, H - y);
            cv::Rect valid(ox, oy, ww, hh);
            cv::Mat p32f;
            processed.convertTo(p32f, CV_MAKE_TYPE(CV_32F, C));
            // 权重: 线性 feather (中心重, 边缘轻)
            cv::Mat wTile = cv::Mat::zeros(valid.height, valid.width, CV_32F);
            for (int j = 0; j < valid.height; ++j) {
                float fy = std::min((float)(j + 1) / valid.height,
                                    (float)(valid.height - j) / valid.height);
                for (int i = 0; i < valid.width; ++i) {
                    float fx = std::min((float)(i + 1) / valid.width,
                                        (float)(valid.width - i) / valid.width);
                    wTile.at<float>(j, i) = std::sqrt(fx * fy) + 1e-3f;
                }
            }
            cv::Mat outRoi = dst(cv::Rect(x, y, ww, hh));
            cv::Mat wRoi = wSum(cv::Rect(x, y, ww, hh));
            cv::Mat srcTile = p32f(valid);
            // 按通道加权叠加
            std::vector<cv::Mat> dCh, sCh, wBroadcast;
            cv::split(outRoi, dCh);
            cv::split(srcTile, sCh);
            for (int c = 0; c < C; ++c) {
                dCh[c] += sCh[c].mul(wTile);
            }
            wRoi += wTile;
            cv::merge(dCh, outRoi);
        }
    }
    // 除以权重
    std::vector<cv::Mat> dCh;
    cv::split(dst, dCh);
    for (int c = 0; c < C; ++c) cv::divide(dCh[c], wSum, dCh[c]);
    cv::merge(dCh, dst);
    if (src.depth() == CV_8U) dst = to8U(dst, 255.0);
}

} // namespace algo
