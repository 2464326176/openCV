// algorithms/segmentation/main.cpp
// Comparison demo of multiple image segmentation methods.
//
// Methods covered:
//   threshold:  Otsu global threshold / adaptive threshold (mean & gaussian)
//   color clustering: KMeans color quantization (k=3/5/8)
//   systems:    MeanShift pixel clustering / GrabCut (rect-init interactive foreground segmentation) /
//               Watershed (distance-transform markers) / connectedComponents
//
// Output: out/algorithms/segmentation_compare.png panorama + region count statistics per method.
// Usage: segmentation.exe [input_img]
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// Colorize connected-component / label maps with random pseudo colors for visualization.
static cv::Mat colorizeLabels(const cv::Mat& labels, int n) {
    cv::Mat out(labels.size(), CV_8UC3, cv::Scalar(0, 0, 0));
    std::vector<cv::Vec3b> colors(n);
    for (int i = 0; i < n; ++i)
        colors[i] = cv::Vec3b((i * 57) & 255, (i * 113) & 255, (i * 193) & 255);
    for (int y = 0; y < labels.rows; ++y)
        for (int x = 0; x < labels.cols; ++x) {
            int l = labels.at<int>(y, x);
            if (l >= 0 && l < n) out.at<cv::Vec3b>(y, x) = colors[l];
        }
    return out;
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    if (argc > 1) inPath = argv[1];

    cv::Mat src = cv::imread(inPath, cv::IMREAD_COLOR);
    if (src.empty()) { log("segmentation", "input empty, synth"); src = cv::Mat(400,400,CV_8UC3); cv::randu(src,0,256); }
    if (std::max(src.rows, src.cols) > 720) {
        double s = 720.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

    ensureDir("../out/algorithms");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(src); labels.push_back("input");

    // ---- 1. thresholding ----
    cv::Mat otsu;
    double t = cv::threshold(gray, otsu, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    panels.push_back(otsu); labels.push_back("Otsu(t=" + std::to_string((int)t) + ")");

    cv::Mat adaptM, adaptG;
    cv::adaptiveThreshold(gray, adaptM, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 51, 3);
    cv::adaptiveThreshold(gray, adaptG, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 51, 3);
    panels.push_back(adaptM); labels.push_back("Adaptive-mean(51)");
    panels.push_back(adaptG); labels.push_back("Adaptive-gauss(51)");

    // ---- 2. KMeans color quantization ----
    cv::Mat flat = src.reshape(1, src.rows * src.cols);
    flat.convertTo(flat, CV_32F);
    for (int k : {3, 5, 8}) {
        cv::Mat bestLabels, centers;
        cv::kmeans(flat, k, bestLabels,
                   cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.6),
                   3, cv::KMEANS_PP_CENTERS, centers);
        // recolor each pixel with the cluster center (centers is k x 3 CV_32F)
        cv::Mat out = cv::Mat::zeros(src.size(), CV_8UC3);
        int count = (int)bestLabels.total();
        for (int i = 0; i < count; ++i) {
            int cl = bestLabels.at<int>(i);
            int y = i / src.cols, x = i % src.cols;
            cv::Vec3f cen = centers.at<cv::Vec3f>(cl, 0);
            cv::Vec3b p(cv::saturate_cast<uchar>(cen[0]), cv::saturate_cast<uchar>(cen[1]), cv::saturate_cast<uchar>(cen[2]));
            out.at<cv::Vec3b>(y, x) = p;
        }
        panels.push_back(out); labels.push_back("KMeans k=" + std::to_string(k));
    }

    // ---- 3. MeanShift pixel clustering (smoothing + intermediate for segmentation) ----
    cv::Mat ms;
    cv::pyrMeanShiftFiltering(src, ms, 20, 30);
    panels.push_back(ms); labels.push_back("MeanShift(sp=20,sr=30)");

    // ---- 4. GrabCut (rect init, take fg + probable-fg) ----
    {
        cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
        cv::Rect rect(cvRound(src.cols * 0.12), cvRound(src.rows * 0.12),
                      cvRound(src.cols * 0.76), cvRound(src.rows * 0.76));
        cv::Mat bgd, fgd;
        cv::grabCut(src, mask, rect, bgd, fgd, 5, cv::GC_INIT_WITH_RECT);
        cv::Mat fg = (mask == cv::GC_FGD) + (mask == cv::GC_PR_FGD);
        cv::Mat fg8; fg.convertTo(fg8, CV_8UC1, 255.0);
        panels.push_back(fg8); labels.push_back("GrabCut(rect)");
    }

    // ---- 5. Watershed (distance-transform-based markers) ----
    {
        cv::Mat bin;
        cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
        cv::Mat dist;
        cv::distanceTransform(bin, dist, cv::DIST_L2, 3);
        cv::Mat sureFg;
        cv::threshold(dist, sureFg, 0.4 * cv::norm(dist, cv::NORM_INF), 255, cv::THRESH_BINARY);
        sureFg.convertTo(sureFg, CV_8U);  // threshold preserves the input type (CV_32F), convert to 8U
        cv::Mat markers, m8;
        int n = cv::connectedComponents(sureFg, markers);
        // background label = n, other labels +1 (0 reserved for unknown region)
        markers = markers + 1;
        cv::Mat unknown;
        cv::dilate(bin, unknown, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3)));
        unknown = unknown - sureFg;
        markers.setTo(0, unknown);
        cv::Mat srcWS = src.clone();
        cv::watershed(srcWS, markers);
        cv::Mat color = colorizeLabels(markers, n + 2);
        // draw watershed boundary lines in red
        for (int y = 0; y < markers.rows; ++y)
            for (int x = 0; x < markers.cols; ++x)
                if (markers.at<int>(y, x) == -1) color.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255);
        panels.push_back(color); labels.push_back("Watershed(" + std::to_string(n) + " fg)");
    }

    // ---- 6. connectedComponents region count statistics ----
    {
        cv::Mat otsu2;
        cv::threshold(gray, otsu2, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        cv::Mat labelsMat, stats, cent;
        int n = cv::connectedComponentsWithStats(otsu2, labelsMat, stats, cent);
        panels.push_back(colorizeLabels(labelsMat, n)); labels.push_back("CC4map(n=" + std::to_string(n - 1) + ")");
        std::printf("[segmentation] Otsu foreground CC count = %d\n", n - 1);
    }

    cv::Mat canvas = gridWithLabels(panels, labels, 3, 30);
    std::string out = "../out/algorithms/segmentation_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[segmentation] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}
