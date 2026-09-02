// algorithms/morphology/main.cpp
// Morphological processing comparison demo. Applies the full set of morphological
// operations to a binary mask, plus two practical scenarios:
// removing spurs with opening / filling holes with closing.
//
// Operators covered (cv::morphologyEx and manual erode/dilate):
//   basic:    erode / dilate
//   combined: open = erode→dilate / close = dilate→erode
//             morphological gradient = dilate − erode
//             tophat = src − open / blackhat = close − src
//   advanced: hit-or-miss (corner detection)
//   SE:       structuring element rect/ellipse/cross comparison; kernel size 3/5/7 comparison
//
// Output: out/algorithms/morphology_compare.png panorama + pixel change statistics.
// Usage: morphology.exe [input_img] [bin_threshold(default Otsu)]
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// Apply a given morphological operation to an 8UC1 binary image.
static cv::Mat morph(const cv::Mat& bin, int op, int ksize = 3,
                     int shape = cv::MORPH_RECT) {
    cv::Mat kernel = cv::getStructuringElement(shape, cv::Size(ksize, ksize));
    cv::Mat out;
    cv::morphologyEx(bin, out, op, kernel);
    return out;
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    if (argc > 1) inPath = argv[1];

    cv::Mat src = cv::imread(inPath, cv::IMREAD_GRAYSCALE);
    if (src.empty()) { log("morphology", "input empty, synth"); src = cv::Mat(512, 512, CV_8UC1); cv::randu(src, 0, 256); }
    if (std::max(src.rows, src.cols) > 700) {
        double s = 700.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }

    // Binarize to get the foreground mask (white=foreground). Otsu picks the threshold automatically.
    cv::Mat bin;
    double thr = (argc > 2) ? std::stod(argv[2]) :
                 cv::threshold(src, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    if (argc > 2) cv::threshold(src, bin, thr, 255, cv::THRESH_BINARY);
    std::printf("[morphology] threshold = %.1f\n", thr);

    ensureDir("../out/algorithms");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(src); labels.push_back("gray input");
    panels.push_back(bin); labels.push_back("binary(mask)");

    // ---- 1. basic: erode / dilate (3x3 rect SE) ----
    cv::Mat k3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    cv::Mat er, di;
    cv::erode(bin, er, k3);
    cv::dilate(bin, di, k3);
    panels.push_back(er); labels.push_back("erode(3x3)");
    panels.push_back(di); labels.push_back("dilate(3x3)");

    // ---- 2. combined: open / close / gradient / tophat / blackhat ----
    panels.push_back(morph(bin, cv::MORPH_OPEN));   labels.push_back("open(3x3)");
    panels.push_back(morph(bin, cv::MORPH_CLOSE));  labels.push_back("close(3x3)");
    panels.push_back(morph(bin, cv::MORPH_GRADIENT)); labels.push_back("gradient(3x3)");
    panels.push_back(morph(bin, cv::MORPH_TOPHAT)); labels.push_back("tophat(3x3)");
    panels.push_back(morph(bin, cv::MORPH_BLACKHAT)); labels.push_back("blackhat(3x3)");

    // ---- 3. structuring element shape comparison (opening) ----
    {
        cv::Mat ker1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
        cv::Mat ker2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5));
        cv::Mat ker3 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(5,5));
        cv::Mat o1, o2, o3;
        cv::morphologyEx(bin, o1, cv::MORPH_OPEN, ker1);
        cv::morphologyEx(bin, o2, cv::MORPH_OPEN, ker2);
        cv::morphologyEx(bin, o3, cv::MORPH_OPEN, ker3);
        panels.push_back(o1); labels.push_back("open-ellipse(5)");
        panels.push_back(o2); labels.push_back("open-cross(5)");
        panels.push_back(o3); labels.push_back("open-rect(5)");
    }

    // ---- 4. kernel size comparison (rect closing, fill small holes) ----
    for (int k : {5, 9}) {
        panels.push_back(morph(bin, cv::MORPH_CLOSE, k, cv::MORPH_ELLIPSE));
        labels.push_back("close-Ell(" + std::to_string(k) + ")");
    }

    // ---- 5. hit-or-miss: use a complementary kernel pair to detect 2x2 all-foreground corner blocks ----
    {
        // True hit-or-miss requires the fg kernel to hit foreground and the bg kernel to hit background (bg kernel = !kernel)
        // Here we use OpenCV's hit-or-miss (fg normalized), outputting white markers at "corner" positions.
        cv::Mat kHit = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2,2));
        cv::Mat hit;
        cv::morphologyEx(bin, hit, cv::MORPH_HITMISS, kHit);
        // With 3x3 only a foreground center hits -> used for "fully isolated pixel detection", demo purpose
        cv::Mat thin;
        cv::erode(bin, thin, kHit);
        panels.push_back(hit);   labels.push_back("hit-miss(2x2)");
        panels.push_back(thin);  labels.push_back("erode(2x2)");
    }

    // ---- metrics: foreground area ratio / connected-component count / change amount ----
    std::printf("%-28s %14s %10s\n", "panel", "fg_pixels", "components");
    for (size_t i = 0; i < panels.size(); ++i) {
        if (i == 0) continue;  // skip gray input
        cv::Mat g;
        if (panels[i].channels() == 3) cv::cvtColor(panels[i], g, cv::COLOR_BGR2GRAY);
        else g = panels[i];
        cv::Mat b = g >= 8;
        double fg = double(cv::countNonZero(b)) / double(b.total()) * 100.0;
        cv::Mat labelsMat, stats, cent;
        int n = cv::connectedComponents(b, labelsMat, 8);
        std::printf("%-28s %12.2f%% %10d\n",
                    labels[i].c_str(), fg, n - 1);
    }

    cv::Mat canvas = gridWithLabels(panels, labels, 4, 30);
    std::string out = "../out/algorithms/morphology_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[morphology] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}
