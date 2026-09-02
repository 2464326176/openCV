// algorithms/hdr/main.cpp
// HDR & exposure fusion demo, data source: data/nv21/ev/ or data/nv21/hdr_*/
//
// Compared with the basic version, this demo adds:
//   1. Runs both Debevec + Robertson CRF/merge combinations and compares them.
//   2. Runs 7 tonemaps on each merged HDR (Drago / Durand / ReinhardGlobal
//      / ReinhardLocal / Mantiuk / Linear / PowerLaw) and outputs a comparison board.
//   3. Prints the HDR log-luminance histogram (ASCII bar chart).
//   4. Custom Mertens (tunable contrast/saturation/exposure weights) for A/B comparison.
//   5. If GT exists: automatically computes PSNR/SSIM/LOE/MS-SSIM for each tonemap result.
//
// Usage: hdr.exe [ev_dir_or_seq1 [seq2 ...]] [optional: gt_path]
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

// Print the histogram (ASCII bar chart)
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
    // first few frames of the input sequence + EV labels
    for (size_t i = 0; i < std::min<size_t>(imgs.size(), 3); ++i) {
        compareImgs.push_back(imgs[i]);
        std::ostringstream os;
        os << "in#" << i << " t=" << std::fixed << std::setprecision(2) << times[i];
        compareLabels.push_back(os.str());
    }

    // two pipelines: Debevec + Debevec, Robertson + Robertson
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
        // add each tonemap to the comparison board
        for (auto& [key, ldr] : r.tonemapPack.results) {
            std::string full = head + ":" + key;
            compareImgs.push_back(ldr);
            compareLabels.push_back(full);
            if (!gt.empty()) evalOne(full, ldr, gt);
        }
    }

    // custom Mertens weight comparison, 3 groups
    {
        auto mm1 = mertensFusion(imgs, 1, 1, 1);
        auto mm2 = mertensFusion(imgs, 2.0f, 0.5f, 1.0f); // prefers contrast
        auto mm3 = mertensFusion(imgs, 0.8f, 2.0f, 1.2f); // prefers saturation
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

    // output comparison image (wrap to at most 4 columns)
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
