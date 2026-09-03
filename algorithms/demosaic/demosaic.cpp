// algorithms/demosaic/demosaic.cpp
// Demosaicing (CFA -> RGB) algorithm comparison demo (ISP Demosaic stage).
//
// Pipeline:
//   1. Load a reference RGB image, synthesize an RGGB Bayer CFA raw frame by
//      point-sampling (optionally with shot-noise) - a synthetic "RAW" frame.
//   2. Run 4 demosaic algorithms and compare against the original:
//        - Bilinear  : cvtColor COLOR_BayerBG2BGR (RGGB), fast, zipper artifacts
//        - Malvar    : Malvar-He-Cutler 2004 5x5 linear filters (implemented here)
//        - VNG       : Variable Number of Gradients, COLOR_BayerBG2BGR_VNG
//        - EdgeAware : Edge-Aware, COLOR_BayerBG2BGR_EA
//   3. Metrics: PSNR / SSIM / per-channel MAE vs the original; plus a zoomed
//      detail crop panel to expose zipper/false-color artifacts.
//
// Usage: demosaic.exe [input_img] [noise_sigma=0]
#include "../common/algo_utils.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace algo;

// ---------------------------------------------------------------------------
// Malvar-He-Cutler (2004) high-quality linear interpolation demosaicing (RGGB).
// Native CFA samples are kept as-is; non-native sites are filled with the
// 5x5 Laplacian-correction filters from the paper (all applied with 1/8 gain):
//   kG    : G at R/B site
//   kRGrR : R at G-in-R-row  / B at G-in-B-row
//   kRGbB : R at G-in-B-row  / B at G-in-R-row  (transpose of kRGrR)
//   kRB   : R at B site      / B at R site
// ---------------------------------------------------------------------------
static cv::Mat demosaicMalvar(const cv::Mat& cfa /*CV_8UC1, RGGB*/) {
    CV_Assert(cfa.type() == CV_8UC1 && cfa.rows % 2 == 0 && cfa.cols % 2 == 0);
    cv::Mat srcF;
    cfa.convertTo(srcF, CV_32F);

    static const float kG[25]    = { 0, 0, -1,   0, 0,
                                     0, 0,  2,   0, 0,
                                    -1, 2,  4,   2, -1,
                                     0, 0,  2,   0, 0,
                                     0, 0, -1,   0, 0};
    static const float kRGrR[25] = { 0, 0,  0.5f, 0, 0,
                                     0, -1, 0,  -1, 0,
                                    -1, 4,  5,   4, -1,
                                     0, -1, 0,  -1, 0,
                                     0, 0,  0.5f, 0, 0};
    static const float kRGbB[25] = { 0,   0, -1,  0,   0,
                                     0,  -1,  4, -1,   0,
                                     0.5f, 0, 5,  0, 0.5f,
                                     0,  -1,  4, -1,   0,
                                     0,   0, -1,  0,   0};
    static const float kRB[25]   = { 0, 0, -1.5f, 0, 0,
                                     0, 2,  0,    2, 0,
                                    -1.5f, 0, 6, 0, -1.5f,
                                     0, 2,  0,    2, 0,
                                     0, 0, -1.5f, 0, 0};
    const float gain = 1.0f / 8.0f;
    auto conv = [&](const float k[25]) {
        cv::Mat ker = cv::Mat(5, 5, CV_32F, (void*)k) * gain, f;
        cv::filter2D(srcF, f, CV_32F, ker, cv::Point(-1, -1), 0,
                     cv::BORDER_REFLECT_101);
        return f;
    };
    cv::Mat fG    = conv(kG);
    cv::Mat fRGrR = conv(kRGrR);
    cv::Mat fRGbB = conv(kRGbB);
    cv::Mat fRB   = conv(kRB);

    // Site masks: 0:(0,0)=R  1:(0,1)=G(in R row)  2:(1,0)=G(in B row)  3:(1,1)=B
    cv::Mat mR = cv::Mat::zeros(cfa.size(), CV_32F);
    cv::Mat mGr = mR.clone(), mGb = mR.clone(), mB = mR.clone();
    for (int y = 0; y < cfa.rows; ++y) {
        float* pr = mR.ptr<float>(y);
        float* p1 = mGr.ptr<float>(y);
        float* p2 = mGb.ptr<float>(y);
        float* pb = mB.ptr<float>(y);
        for (int x = 0; x < cfa.cols; ++x) {
            switch (((y & 1) << 1) | (x & 1)) {
                case 0:  pr[x] = 1.0f; break;
                case 1:  p1[x] = 1.0f; break;
                case 2:  p2[x] = 1.0f; break;
                default: pb[x] = 1.0f; break;
            }
        }
    }

    // Native planes + corrections at non-native sites
    cv::Mat R = srcF.mul(mR) + fRGrR.mul(mGr) + fRGbB.mul(mGb) + fRB.mul(mB);
    cv::Mat B = srcF.mul(mB) + fRGrR.mul(mGb) + fRGbB.mul(mGr) + fRB.mul(mR);
    cv::Mat G = srcF.mul(mGr + mGb) + fG.mul(mR + mB);

    std::vector<cv::Mat> ch = {B, G, R};  // BGR order
    cv::Mat out;
    cv::merge(ch, out);
    return to8U(out, 1.0);
}

// ---------------------------------------------------------------------------
// Synthesize an RGGB CFA frame from a BGR image: sample R at even-even,
// G at odd-even / even-odd, B at odd-odd (OpenCV BayerBG2BGR == RGGB).
// ---------------------------------------------------------------------------
static cv::Mat makeRGGB(const cv::Mat& bgr) {
    cv::Mat cfa(bgr.size(), CV_8UC1, cv::Scalar::all(0));
    std::vector<cv::Mat> ch;
    cv::split(bgr, ch);
    for (int y = 0; y < bgr.rows; ++y) {
        for (int x = 0; x < bgr.cols; ++x) {
            int site = ((y & 1) << 1) | (x & 1);
            cfa.at<uchar>(y, x) =
                site == 0 ? ch[2].at<uchar>(y, x) :   // R
                site == 3 ? ch[0].at<uchar>(y, x) :   // B
                          ch[1].at<uchar>(y, x);      // G
        }
    }
    return cfa;
}

struct ResultRow {
    std::string tag;
    cv::Mat img;
    double psnr = 0, ssim = 0, maeB = 0, maeG = 0, maeR = 0;
};

static void scoreRow(ResultRow& r, const cv::Mat& ref) {
    r.psnr = psnr(ref, r.img);
    r.ssim = ssim(ref, r.img);
    std::vector<cv::Mat> a, b;
    cv::split(ref, a);
    cv::split(r.img, b);
    r.maeB = mae(a[0], b[0]);
    r.maeG = mae(a[1], b[1]);
    r.maeR = mae(a[2], b[2]);
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/VCG5.jpg";
    double noiseSigma = 0.0;
    if (argc > 1) inPath = argv[1];
    if (argc > 2) noiseSigma = std::atof(argv[2]);

    cv::Mat orig = cv::imread(inPath, cv::IMREAD_COLOR);
    if (orig.empty()) {
        log("demosaic", "input missing, synthesizing a color test pattern");
        orig = cv::Mat(720, 1080, CV_8UC3);
        cv::randu(orig, cv::Scalar::all(40), cv::Scalar::all(210));
        cv::rectangle(orig, {120, 120, 260, 260}, {40, 40, 220}, cv::FILLED);
        cv::circle(orig, {720, 360}, 170, {210, 200, 40}, cv::FILLED);
        for (int i = 30; i < 1000; i += 10)
            cv::line(orig, {i, 540}, {i, 700}, {255, 255, 255}, 2);
    }
    // Even dims + moderate size (VNG/EA are slow on large frames).
    if (orig.cols % 2 || orig.rows % 2)
        orig = orig(cv::Rect(0, 0, orig.cols & ~1, orig.rows & ~1)).clone();
    if (std::max(orig.rows, orig.cols) > 1200) {
        double s = 1200.0 / std::max(orig.rows, orig.cols);
        cv::resize(orig, orig, cv::Size(), s, s, cv::INTER_AREA);
        orig = orig(cv::Rect(0, 0, orig.cols & ~1, orig.rows & ~1)).clone();
    }
    log("demosaic", "ref " + std::to_string(orig.cols) + "x" + std::to_string(orig.rows) +
        ", noise sigma=" + std::to_string(noiseSigma));

    // ---- Synthetic RAW: RGGB CFA (+ optional shot noise) ----
    cv::Mat cfa = makeRGGB(orig);
    if (noiseSigma > 0) {
        cv::Mat nz(cfa.size(), CV_16SC1);
        cv::randn(nz, cv::Scalar::all(0), cv::Scalar::all(noiseSigma));
        cv::Mat c16;
        cfa.convertTo(c16, CV_16SC1);
        c16 += nz;                      // convertTo saturates on the way back
        c16.convertTo(cfa, CV_8UC1);
    }

    // ---- Demosaic algorithms ----
    cv::Mat bil, mal, vng, ea;
    cv::cvtColor(cfa, bil, cv::COLOR_BayerBG2BGR);      // RGGB bilinear
    mal = demosaicMalvar(cfa);
    cv::cvtColor(cfa, vng, cv::COLOR_BayerBG2BGR_VNG);
    cv::cvtColor(cfa, ea,  cv::COLOR_BayerBG2BGR_EA);

    std::vector<ResultRow> rows = {
        {"Bilinear", bil, 0, 0, 0, 0, 0},
        {"Malvar(2004)", mal, 0, 0, 0, 0, 0},
        {"VNG", vng, 0, 0, 0, 0, 0},
        {"EdgeAware", ea, 0, 0, 0, 0, 0},
    };
    for (auto& r : rows) scoreRow(r, orig);

    // ---- Metric table ----
    std::printf("\n%-16s %8s %8s %8s %8s %8s\n",
                "method", "PSNR", "SSIM", "MAE-B", "MAE-G", "MAE-R");
    for (auto& r : rows)
        std::printf("%-16s %8.2f %8.4f %8.2f %8.2f %8.2f\n",
                    r.tag.c_str(), r.psnr, r.ssim, r.maeB, r.maeG, r.maeR);
    std::printf("\n> Malvar clearly beats Bilinear at near-zero cost; VNG/EA are\n"
                "> stronger on edges but image-dependent. G channel MAE is smallest\n"
                "> (half the CFA samples are G).\n");

    // ---- Zoom detail panels: expose zipper / false color ----
    cv::Rect roi(orig.cols / 2 - 90, orig.rows / 2 - 90, 180, 180);
    roi &= cv::Rect(0, 0, orig.cols, orig.rows);
    auto crop3x = [&](const cv::Mat& m) {
        cv::Mat c = m(roi).clone(), up;
        cv::resize(c, up, cv::Size(), 3, 3, cv::INTER_NEAREST);
        return up;
    };
    std::vector<cv::Mat> panels = {crop3x(orig), crop3x(bil), crop3x(mal),
                                   crop3x(vng), crop3x(ea)};
    auto tag = [](const std::string& name, double v) {
        return name + " [" + std::to_string((int)v) + "dB]";
    };
    std::vector<std::string> labels = {"original", "Bilinear",
                                       tag("Malvar", rows[1].psnr),
                                       tag("VNG", rows[2].psnr),
                                       tag("EdgeAware", rows[3].psnr)};
    cv::Mat detail = hstackWithLabels(panels, labels, 30);

    // ---- Full-frame board ----
    std::vector<cv::Mat> full = {orig, bil, mal, vng, ea};
    std::vector<std::string> flab = labels;
    flab[0] = "original (ref)";
    cv::Mat board = gridWithLabels(full, flab, 3, 30);
    const std::string outPng = "../out/algorithms/demosaic_compare.png";
    cv::imwrite(outPng, board);
    const std::string outDetail = "../out/algorithms/demosaic_detail.png";
    cv::imwrite(outDetail, detail);
    log("demosaic", "saved " + outPng + " + " + outDetail);

    imshowFit("demosaic", board);
    return 0;
}
