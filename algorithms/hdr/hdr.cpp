// algorithms/hdr/main.cpp
// HDR & 曝光融合演示, 数据来源: data/nv21/ev/ 或 data/nv21/hdr_*/
//
// 相比基础版本, 本 demo 扩展以下能力:
//   1. 同时运行 Debevec + Robertson 两种 CRF/合并组合并对比.
//   2. 对每组合成 HDR 跑 6 种 tonemap (Drago / Durand / ReinhardGlobal
//      / ReinhardLocal / Mantiuk / Linear / PowerLaw), 并输出对比板.
//   3. 打印 HDR log-luminance 直方图 (字符条形图).
//   4. 自定义 Mertens (可调对比度/饱和度/曝光权重) 做 A/B 对照.
//   5. 若有 GT: 对每幅 tonemap 结果自动算 PSNR/SSIM/LOE/MS-SSIM.
//
// 用法: hdr.exe [ev_dir_or_seq1 [seq2 ...]] [optional: gt_path]
#include "../common/nv21_io.hpp"
#include "../common/algo_utils.hpp"
#include "../common/single_denoise.hpp"
#include "hdr_pipeline.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace algo;

static std::vector<algo::LoadedFrame> loadNv21Seq(const std::string& dir) {
    return algo::loadNv21Dir(dir, true);
}

static cv::Mat scaleToEdge(const cv::Mat& in, int maxEdge) {
    int m = std::max(in.rows, in.cols);
    if (m <= maxEdge) return in.clone();
    double s = (double)maxEdge / m;
    cv::Mat out;
    cv::resize(in, out, cv::Size(), s, s, cv::INTER_AREA);
    return out;
}

// 将 histogram 打出来 (字符条形图)
static void printHistogram(const std::vector<float>& hist,
                           const std::string& title, int barWidth = 40) {
    std::cout << "--- " << title << " (20 bins, log10Y ∈ [-6, 2]) ---\n";
    float mx = 0;
    for (auto v : hist) if (v > mx) mx = v;
    if (mx < 1e-7f) { std::cout << "  [empty]\n"; return; }
    for (size_t i = 0; i < hist.size(); ++i) {
        int len = (int)(barWidth * hist[i] / mx);
        std::cout << std::setw(2) << i << " ";
        for (int k = 0; k < len; ++k) std::cout << '#';
        std::cout << " " << std::fixed << std::setprecision(3)
                  << hist[i] << "\n";
    }
}

static void evalOne(const std::string& tag, const cv::Mat& a, const cv::Mat& b) {
    if (a.empty() || b.empty()) return;
    cv::Mat aa, bb;
    cv::Size sz(std::min(a.cols, b.cols), std::min(a.rows, b.rows));
    cv::resize(a, aa, sz); cv::resize(b, bb, sz);
    double ps = psnr(aa, bb);
    double sm = ssim(aa, bb);
    double ms = msssim(aa, bb);
    int lo = loe(aa, bb, 48);
    std::printf("[eval] %-28s PSNR=%6.2fdB SSIM=%.4f MS-SSIM=%.4f LOE=%d\n",
                tag.c_str(), ps, sm, ms, lo);
}

int main(int argc, char** argv) {
    std::string evDir = "../../data/nv21/ev";
    std::string gtPath = "../../data/nv21/hdr_20260622_183924_835_0_100--max/"
                         "morpho_image_refiner_output_20260622_183924_835_"
                         "4032x3000_w_4000_base_0_merge_3.NV21";
    if (argc > 1) evDir = argv[1];
    if (argc > 2) gtPath = argv[2];

    std::vector<algo::LoadedFrame> frames = loadNv21Seq(evDir);
    std::vector<cv::Mat> imgs;
    std::vector<double> times;

    auto fallbackSynthetic = [&]() {
        cv::Mat base = cv::imread("../../data/images/lena.jpg", cv::IMREAD_COLOR);
        if (base.empty()) base = cv::Mat::zeros(512, 512, CV_8UC3);
        base.convertTo(base, CV_32F, 1.0 / 255.0);
        for (float t : {0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f}) {
            cv::Mat e;
            cv::pow(base, 1.0f / t, e);
            cv::min(e, 1.0, e);
            cv::Mat u; e.convertTo(u, CV_8U, 255.0);
            imgs.push_back(u); times.push_back((double)t);
        }
        std::cout << "[hdr] fall back to synthetic exposures (lenna × 6)\n";
    };

    if (frames.size() < 2) fallbackSynthetic();
    else {
        for (const auto& lf : frames) {
            cv::Mat bgr = lf.bgr;
            if (bgr.empty()) continue;
            bgr = scaleToEdge(bgr, 800);
            imgs.push_back(bgr);
            double t = parseExposureTimeFromName(lf.meta.path);
            if (t <= 0) t = 1.0;
            times.push_back(t);
            int ev = parseEvValueFromName(lf.meta.path);
            int iso = parseIsoFromName(lf.meta.path);
            auto nm = baseNameNoExt(lf.meta.path);
            std::printf("[hdr] %-55s ev=%3d et=%.3fus iso=%d\n",
                        nm.c_str(), ev, t, iso);
        }
        if (imgs.size() < 2) fallbackSynthetic();
    }

    ensureDir("../out/algorithms");

    std::vector<cv::Mat> compareImgs;
    std::vector<std::string> compareLabels;
    // 输入序列前几帧 + EV 标签
    for (size_t i = 0; i < std::min<size_t>(imgs.size(), 3); ++i) {
        compareImgs.push_back(imgs[i]);
        std::ostringstream os;
        os << "in#" << i << " t=" << std::fixed << std::setprecision(2) << times[i];
        compareLabels.push_back(os.str());
    }

    // 两次流水线: Debevec + Debevec, Robertson + Robertson
    std::vector<std::pair<CrfMethod, HdrMergeMethod>> pipelines = {
        {CrfMethod::Debevec, HdrMergeMethod::Debevec},
        {CrfMethod::Robertson, HdrMergeMethod::Robertson},
    };

    TonemapParams tp; tp.gamma = 2.2f; tp.contrast = 3.0f;
    std::vector<TonemapMethod> tonemaps = {
        TonemapMethod::Drago, TonemapMethod::Durand, TonemapMethod::Reinhard,
        TonemapMethod::ReinhardLocal, TonemapMethod::Mantiuk,
        TonemapMethod::Linear, TonemapMethod::Gamma,
    };

    cv::Mat gt;
    if (!gtPath.empty()) { gt = readNv21Auto(gtPath); if (!gt.empty()) gt = scaleToEdge(gt, 800); }

    for (auto& [crf, merge] : pipelines) {
        HdrResult r = hdrPipeline(imgs, times, crf, merge, tonemaps, tp);
        std::ostringstream os; os << r.crfMethodName << "+" << r.mergeMethodName;
        std::string head = os.str();
        std::cout << "\n=== Pipeline: " << head << " ===\n";
        printHistogram(r.recoveredLumHistogram, head + " HDR log-luminance hist");
        // 每种 tonemap 加入对比板
        for (auto& [key, ldr] : r.tonemapPack.results) {
            std::string full = head + ":" + key;
            compareImgs.push_back(ldr);
            compareLabels.push_back(full);
            if (!gt.empty()) evalOne(full, ldr, gt);
        }
    }

    // 自定义 Mertens 权重对比 3 组
    {
        auto mm1 = mertensFusion(imgs, 1, 1, 1);
        auto mm2 = mertensFusion(imgs, 2.0f, 0.5f, 1.0f); // 偏好对比度
        auto mm3 = mertensFusion(imgs, 0.8f, 2.0f, 1.2f); // 偏好饱和度
        compareImgs.push_back(mm1); compareLabels.push_back("Mertens C:S:E=1:1:1");
        compareImgs.push_back(mm2); compareLabels.push_back("Mertens C↑ S↓");
        compareImgs.push_back(mm3); compareLabels.push_back("Mertens C↓ S↑ E↑");
        if (!gt.empty()) {
            evalOne("Mertens C:S:E=1:1:1", mm1, gt);
            evalOne("Mertens C↑ S↓", mm2, gt);
            evalOne("Mertens C↓ S↑ E↑", mm3, gt);
        }
    }

    if (!gt.empty()) {
        compareImgs.push_back(gt); compareLabels.push_back("GT: merge_3");
    }

    // 输出对比图 (按列 wrap 到最多 4 列)
    const int colsPerRow = 4;
    cv::Mat bigCanvas = gridWithLabels(compareImgs, compareLabels, colsPerRow, 32);
    std::string outPath = "../out/algorithms/hdr_tonemap_mosaic.png";
    ensureDir("../out/algorithms");
    cv::imwrite(outPath, bigCanvas);
    std::cout << "[hdr] wrote " << outPath
              << "  total " << compareImgs.size() << " panels\n";
    imshowFit("hdr_tonemap_mosaic", bigCanvas, 1800, 0);
    return 0;
}
