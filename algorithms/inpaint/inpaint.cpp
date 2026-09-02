// algorithms/inpaint/main.cpp
// Image inpainting: restoration of scratch / text-occluded regions.
//
// Algorithms:
//   INPAINT_TELEA   Telea (fast marching method, fills along isophotes)
//   INPAINT_NS      Navier-Stokes (flow-field guided, edge preserving)
//   The radius parameter r_ controls the diffusion range.
// Evaluation: artificially create scratches/text occlusion on a clean image ->
//       after restoration compute MAE/PSNR within the "occluded region" against the clean image,
//       plus whole-image PSNR (how well the content is preserved).
// Output: out/algorithms/inpaint_compare.png + metrics table.
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// Draw several white scratches + a text-like block on img, returns the mask (255=damaged).
static void drawDamage(cv::Mat& img, cv::Mat& mask) {
    mask = cv::Mat::zeros(img.size(), CV_8UC1);
    cv::RNG rng(12345);
    // several random thin-line scratches
    for (int i = 0; i < 5; ++i) {
        cv::Point p1(rng.uniform(5, img.cols - 5), rng.uniform(5, img.rows - 5));
        cv::Point p2(p1.x + rng.uniform(-150, 150), p1.y + rng.uniform(-60, 60));
        cv::line(img, p1, p2, cv::Scalar(255, 255, 255), 2);
        cv::line(mask, p1, p2, cv::Scalar(255), 3);
    }
    // a text-style occlusion block
    cv::Rect textBox(30, 30, 180, 46);
    cv::rectangle(img, textBox, cv::Scalar(255, 255, 255), cv::FILLED);
    cv::putText(img, "HELP?", cv::Point(38, 64), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                cv::Scalar(0, 0, 0), 2);
    cv::rectangle(mask, textBox, cv::Scalar(255), cv::FILLED);
}

// Evaluate MAE only within the mask region (restoration quality).
static double maskedMAE(const cv::Mat& ref, const cv::Mat& out, const cv::Mat& mask) {
    cv::Mat df, ref32, out32, m32;
    ref.convertTo(ref32, CV_32F); out.convertTo(out32, CV_32F);
    mask.convertTo(m32, CV_32F);
    cv::absdiff(ref32, out32, df);
    df = df.mul(m32);  // use mask[0..1] as weight, equivalent to only counting damaged areas
    int n = cv::countNonZero(mask);
    return n ? cv::sum(df)[0] / (double)n : 0;
}

int main(int argc, char** argv) {
    std::string inPath = (argc > 1) ? argv[1] : "../../data/images/lena.jpg";
    cv::Mat clean = cv::imread(inPath, cv::IMREAD_COLOR);
    if (clean.empty()) { clean = cv::Mat(360, 480, CV_8UC3); cv::randu(clean, 0, 256); }
    if (std::max(clean.rows, clean.cols) > 520) {
        double s = 520.0 / std::max(clean.rows, clean.cols);
        cv::resize(clean, clean, cv::Size(), s, s, cv::INTER_AREA);
    }
    ensureDir("../out/algorithms");

    cv::Mat damaged, mask;
    clean.copyTo(damaged);
    drawDamage(damaged, mask);

    struct R { std::string tag; cv::Mat img8; double fullPSNR, maskMAE; };
    std::vector<R> res;
    for (int method : {cv::INPAINT_TELEA, cv::INPAINT_NS}) {
        for (int r : {3, 8}) {
            cv::Mat out;
            cv::inpaint(damaged, mask, out, r, method);
            const char* nm = (method == cv::INPAINT_TELEA) ? "TELEA" : "NS";
            res.push_back({std::string(nm) + " r=" + std::to_string(r), out,
                           psnr(clean, out), maskedMAE(clean, out, mask)});
        }
    }
    std::printf("damaged pixels = %.2f%%\n", 100.0 * cv::countNonZero(mask) / (double)(mask.total()));
    std::printf("%-14s %10s %10s\n", "method", "wholePSNR", "maskMAE");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(clean); labels.push_back("clean(ref)");
    panels.push_back(damaged); labels.push_back("damaged");
    panels.push_back(mask); labels.push_back("mask");
    for (auto& r : res) {
        std::printf("%-14s %10.2f %10.1f\n", r.tag.c_str(), r.fullPSNR, r.maskMAE);
        panels.push_back(r.img8); labels.push_back(r.tag);
    }
    cv::Mat canvas = gridWithLabels(panels, labels, 3, 30);
    std::string out = "../out/algorithms/inpaint_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[inpaint] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}
