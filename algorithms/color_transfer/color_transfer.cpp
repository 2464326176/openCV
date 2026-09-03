// algorithms/color_transfer/color_transfer.cpp
// Color transfer demo: make a source image take on the color statistics /
// style of a reference image (color consistency between cameras / scenes).
//
// Methods (4):
//   1. ReinhardLAB : mean/std matching in CIELAB (Reinhard et al. 2001)
//   2. MeanStdRGB  : per-channel mean/std matching in BGR
//   3. HistMatch   : per-channel CDF histogram matching (exact 1D OT)
//   4. LumaOnlyLAB : LAB L-channel transfer only (keep own chromaticity,
//                    align tone/luminance - handy for multi-camera tuning)
// All results are blended with the source by a strength factor t (arg3).
//
// Metrics: per-channel |mean(out)-mean(ref)| / |std(out)-std(ref)| match
// error, saturation, entropy (NR-IQA) table.
//
// Usage: color_transfer.exe [src_img] [ref_img] [strength=1.0]
#include "../common/algo_utils.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace algo;

// ---------------------------------------------------------------------------
// Reinhard color transfer in CIELAB: out = (in - meanSrc) * (stdRef/stdSrc)
// + meanRef. std ratio is clamped to [0.5, 2.0] to avoid explosion.
// ---------------------------------------------------------------------------
static cv::Mat transferReinhardLAB(const cv::Mat& src, const cv::Mat& refLabStats,
                                   const cv::Scalar& meanS, const cv::Scalar& stdS) {
    cv::Mat lab;
    cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);
    lab.convertTo(lab, CV_32F);
    std::vector<cv::Mat> ch;
    cv::split(lab, ch);
    for (int c = 0; c < 3; ++c) {
        double ratio = refLabStats.at<double>(1, c) / std::max(stdS[c], 1e-6);
        ratio = std::min(std::max(ratio, 0.5), 2.0);
        ch[c] = (ch[c] - meanS[c]) * ratio + refLabStats.at<double>(0, c);
    }
    cv::Mat outLab;
    cv::merge(ch, outLab);
    outLab.convertTo(outLab, CV_8U);
    cv::Mat out;
    cv::cvtColor(outLab, out, cv::COLOR_Lab2BGR);
    return out;
}

// ---------------------------------------------------------------------------
// Per-channel mean/std matching directly in BGR (simpler, can shift hues more).
// ---------------------------------------------------------------------------
static cv::Mat transferMeanStdRGB(const cv::Mat& src,
                                  const cv::Scalar& meanR, const cv::Scalar& stdR,
                                  const cv::Scalar& meanS, const cv::Scalar& stdS) {
    cv::Mat f;
    src.convertTo(f, CV_32F);
    std::vector<cv::Mat> ch;
    cv::split(f, ch);
    for (int c = 0; c < 3; ++c) {
        double ratio = stdR[c] / std::max(stdS[c], 1e-6);
        ratio = std::min(std::max(ratio, 0.5), 2.0);
        ch[c] = (ch[c] - meanS[c]) * ratio + meanR[c];
    }
    cv::Mat out;
    cv::merge(ch, out);
    return to8U(out, 1.0);
}

// ---------------------------------------------------------------------------
// Exact per-channel histogram matching via CDF lookup tables.
// ---------------------------------------------------------------------------
static cv::Mat transferHistMatch(const cv::Mat& src, const cv::Mat& ref) {
    std::vector<cv::Mat> sc, rc, refc;
    cv::split(src, sc);
    cv::split(ref, refc);
    std::vector<cv::Mat> rc0 = sc;  // output starts as a copy of source channels
    for (int c = 0; c < 3; ++c) {
        // CDF of source and reference for this channel
        int histS[256] = {0}, histR[256] = {0};
        const int nS = sc[c].rows * sc[c].cols;
        const int nR = refc[c].rows * refc[c].cols;
        for (int y = 0; y < sc[c].rows; ++y) {
            const uchar* p = sc[c].ptr<uchar>(y);
            for (int x = 0; x < sc[c].cols; ++x) ++histS[p[x]];
        }
        for (int y = 0; y < refc[c].rows; ++y) {
            const uchar* p = refc[c].ptr<uchar>(y);
            for (int x = 0; x < refc[c].cols; ++x) ++histR[p[x]];
        }
        uchar lut[256];
        int rIdx = 0;
        double accS = 0, accR = 0;
        for (int v = 0; v < 256; ++v) {
            accS += (double)histS[v] / nS;
            while (rIdx < 256 && accR + (double)histR[rIdx] / nR < accS) {
                accR += (double)histR[rIdx] / nR;
                ++rIdx;
            }
            lut[v] = (uchar)rIdx;
        }
        cv::Mat lutM(1, 256, CV_8UC1, lut);
        cv::LUT(sc[c], lutM, rc0[c]);
    }
    cv::Mat out;
    cv::merge(rc0, out);
    return out;
}

// ---------------------------------------------------------------------------
// Luminance-only transfer in LAB: L gets Reinhard treatment, a/b untouched
// (own colors kept, tone/luminance aligned to the reference camera).
// ---------------------------------------------------------------------------
static cv::Mat transferLumaOnlyLAB(const cv::Mat& src, const cv::Mat& refLabStats,
                                   const cv::Scalar& meanS, const cv::Scalar& stdS) {
    cv::Mat lab;
    cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);
    lab.convertTo(lab, CV_32F);
    std::vector<cv::Mat> ch;
    cv::split(lab, ch);
    double ratio = refLabStats.at<double>(1, 0) / std::max(stdS[0], 1e-6);
    ratio = std::min(std::max(ratio, 0.5), 2.0);
    ch[0] = (ch[0] - meanS[0]) * ratio + refLabStats.at<double>(0, 0);
    cv::Mat outLab;
    cv::merge(ch, outLab);
    outLab.convertTo(outLab, CV_8U);
    cv::Mat out;
    cv::cvtColor(outLab, out, cv::COLOR_Lab2BGR);
    return out;
}

// ---------------------------------------------------------------------------
// Color-statistics match error vs the reference: |dm| and |ds| averaged
// over BGR channels (lower = closer to the reference style).
// ---------------------------------------------------------------------------
static void statMatchError(const cv::Mat& out, const cv::Mat& ref,
                           double& dm, double& ds) {
    auto stdOfMat = [](const cv::Mat& m, const cv::Scalar& mu) {
        cv::Mat f32, sd;
        m.convertTo(f32, CV_32F);
        cv::Mat d = f32 - mu;
        cv::reduce(d.mul(d), sd, 0, cv::REDUCE_SUM);
        sd *= 1.0 / (f32.rows * f32.cols);
        cv::sqrt(sd, sd);
        return sd;
    };
    cv::Scalar mo = cv::mean(out);
    cv::Scalar mr = cv::mean(ref);
    cv::Mat sdo = stdOfMat(out, mo);
    cv::Mat sdr = stdOfMat(ref, mr);
    dm = 0; ds = 0;
    for (int c = 0; c < 3; ++c) {
        dm = std::max(dm, std::abs(mo[c] - mr[c]));
        ds = std::max(ds, std::abs(sdo.at<double>(0, c) - sdr.at<double>(0, c)));
    }
}

int main(int argc, char** argv) {
    std::string srcPath = "../../data/images/VCG3.jpg";
    std::string refPath = "../../data/images/VCG5.jpg";
    double strength = 1.0;
    if (argc > 1) srcPath = argv[1];
    if (argc > 2) refPath = argv[2];
    if (argc > 3) strength = std::atof(argv[3]);
    strength = std::min(std::max(strength, 0.0), 1.0);

    cv::Mat src = cv::imread(srcPath, cv::IMREAD_COLOR);
    cv::Mat ref = cv::imread(refPath, cv::IMREAD_COLOR);
    if (src.empty()) {  // synthetic fallback
        log("color_transfer", "src missing, synthesizing");
        src = cv::Mat(540, 810, CV_8UC3);
        cv::randu(src, cv::Scalar::all(60), cv::Scalar::all(190));
        cv::circle(src, {400, 260}, 150, {80, 90, 200}, cv::FILLED);
    }
    if (ref.empty()) {  // tone reference fallback: gamma-darkened source
        log("color_transfer", "ref missing, synthesizing from source");
        cv::cvtColor(src, ref, cv::COLOR_BGR2GRAY);
        cv::cvtColor(ref, ref, cv::COLOR_GRAY2BGR);
        ref.convertTo(ref, CV_8U, 1.0, -30);
    }
    // Working size: cap long edge (fast + same size for visualization).
    cv::Size work(800, 600);
    cv::resize(src, src, work, 0, 0, cv::INTER_AREA);
    cv::resize(ref, ref, work, 0, 0, cv::INTER_AREA);
    log("color_transfer", "src=" + srcPath + "  ref=" + refPath +
        "  strength=" + std::to_string(strength));

    // ---- Statistics ----
    cv::Scalar meanS = cv::mean(src);
    cv::Scalar meanR = cv::mean(ref);
    auto stdOf = [](const cv::Mat& m) {
        cv::Scalar mu = cv::mean(m);
        cv::Mat f32, sd;
        m.convertTo(f32, CV_32F);
        cv::Mat d = f32 - mu;
        cv::reduce(d.mul(d), sd, 0, cv::REDUCE_SUM);
        sd *= 1.0 / (f32.rows * f32.cols);
        cv::sqrt(sd, sd);
        cv::Scalar out;
        for (int c = 0; c < 3; ++c) out[c] = sd.at<double>(0, c);
        return out;
    };
    cv::Scalar stdS = stdOf(src), stdR = stdOf(ref);
    // Reference LAB stats (used by the two LAB methods)
    cv::Mat refLab;
    cv::cvtColor(ref, refLab, cv::COLOR_BGR2Lab);
    refLab.convertTo(refLab, CV_32F);
    cv::Scalar meanRLab = cv::mean(refLab);
    cv::Mat refLabStats(2, 3, CV_64F);
    for (int c = 0; c < 3; ++c) {
        refLabStats.at<double>(0, c) = meanRLab[c];
    }
    {
        cv::Mat d = refLab - meanRLab, sd;
        cv::reduce(d.mul(d), sd, 0, cv::REDUCE_SUM);
        sd *= 1.0 / (refLab.rows * refLab.cols);
        cv::sqrt(sd, sd);
        for (int c = 0; c < 3; ++c) refLabStats.at<double>(1, c) = sd.at<double>(0, c);
    }

    // ---- Methods + strength blending ----
    auto blend = [&](const cv::Mat& out) {
        if (strength >= 1.0) return out;
        cv::Mat s32, o32;
        src.convertTo(s32, CV_32F);
        out.convertTo(o32, CV_32F);
        return to8U(strength * o32 + (1.0 - strength) * s32, 1.0);
    };
    cv::Mat mLab   = blend(transferReinhardLAB(src, refLabStats, meanS, stdS));
    cv::Mat mRGB   = blend(transferMeanStdRGB(src, meanR, stdR, meanS, stdS));
    cv::Mat mHist  = blend(transferHistMatch(src, ref));
    cv::Mat mLuma  = blend(transferLumaOnlyLAB(src, refLabStats, meanS, stdS));

    // ---- Match-error + NR-IQA table ----
    struct Row { std::string tag; cv::Mat img; double dm, ds, sat, ent; };
    std::vector<Row> rows;
    auto addRow = [&](const std::string& tag, const cv::Mat& m) {
        Row r{tag, m, 0, 0, 0, 0};
        statMatchError(m, ref, r.dm, r.ds);
        NRQuality q = nrQuality(m);
        r.sat = q.saturation;
        cv::Mat g;
        cv::cvtColor(m, g, cv::COLOR_BGR2GRAY);
        r.ent = imageEntropy(g);
        rows.push_back(r);
    };
    addRow("source", src);
    addRow("ReinhardLAB", mLab);
    addRow("MeanStdRGB", mRGB);
    addRow("HistMatch", mHist);
    addRow("LumaOnlyLAB", mLuma);

    std::printf("\n%-14s %10s %10s %9s %9s   (vs reference style)\n",
                "method", "maxDmean", "maxDstd", "sat", "entropy");
    for (auto& r : rows)
        std::printf("%-14s %10.2f %10.2f %9.3f %9.3f\n",
                    r.tag.c_str(), r.dm, r.ds, r.sat, r.ent);
    {
        Row r{"reference", ref, 0, 0, 0, 0};
        NRQuality q = nrQuality(ref);
        cv::Mat g; cv::cvtColor(ref, g, cv::COLOR_BGR2GRAY);
        std::printf("%-14s %10s %10s %9.3f %9.3f\n",
                    "reference", "-", "-", q.saturation, imageEntropy(g));
    }
    std::printf("\n> maxDmean/maxDstd: per-channel max |mean/std| gap to the reference; "
                "LumaOnlyLAB keeps own chroma so its chroma gap stays large by design.\n");

    // ---- Visualization ----
    ensureDir("../out/algorithms");
    std::vector<cv::Mat> panels = {src, ref, mLab, mRGB, mHist, mLuma};
    std::vector<std::string> labels = {"source", "reference",
        "ReinhardLAB", "MeanStdRGB", "HistMatch", "LumaOnlyLAB"};
    cv::Mat board = gridWithLabels(panels, labels, 3, 30);
    const std::string outPng = "../out/algorithms/color_transfer_compare.png";
    cv::imwrite(outPng, board);
    log("color_transfer", "saved " + outPng);

    imshowFit("color_transfer", board);
    return 0;
}
