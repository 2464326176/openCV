// algorithms/stereo/main.cpp
// Stereo matching (disparity estimation) demo: StereoBM vs SGBM on real stereo samples.
//
// Samples: data/aloeL.jpg / data/aloeR.jpg are OpenCV classic calibrated stereo pairs;
//          can also use data/left01.jpg + right01.jpg (paired stereo sequence).
//
// Steps:
//   1. grayscale (SGBM/BM inputs must be single channel);
//   2. StereoBM with several parameter sets (blockSize=5/11/21) + SGBM (blockSize=3/11);
//   3. disparity CV_16S -> normalized 8U, with COLORMAP_JET pseudo color;
//   4. metrics: valid disparity ratio / mean disparity / disparity smoothness (mean |gradient|).
//
// Output: out/algorithms/stereo_compare.png
// Usage: stereo.exe [left_img] [right_img]
#include "../common/algo_utils.hpp"

#include <opencv2/calib3d.hpp>

#include <cstdio>
#include <vector>

using namespace algo;

// Normalize CV_16S disparity (with scale factor) to 8U and gray out invalid (<=0) pixels.
static cv::Mat disparityTo8U(const cv::Mat& disp, int scale = 16) {
    cv::Mat f;
    disp.convertTo(f, CV_32F, 1.0 / scale);
    cv::Mat f8;
    cv::normalize(f, f8, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::Mat valid = disp > 0;
    cv::Mat out;
    f8.copyTo(out);
    out.setTo(128, ~valid);  // set invalid region to mid gray
    return out;
}

static cv::Mat pseudoColor(const cv::Mat& disp8) {
    cv::Mat cm;
    cv::applyColorMap(disp8, cm, cv::COLORMAP_JET);
    return cm;
}

// Full-reference metrics may have no GT; use no-reference statistics here:
// valid ratio / mean disparity / smoothness.
static void reportStats(const char* tag, const cv::Mat& disp, double scale) {
    cv::Mat f;
    disp.convertTo(f, CV_32F, 1.0 / scale);
    cv::Mat valid = disp > 0;
    double ratio = cv::countNonZero(valid) / double(valid.total());
    cv::Scalar mean, stddev;
    cv::meanStdDev(f, mean, stddev, valid);
    cv::Mat gx, gy;
    cv::Sobel(f, gx, CV_32F, 1, 0, 3);
    cv::Sobel(f, gy, CV_32F, 0, 1, 3);
    cv::Mat mag;
    cv::magnitude(gx, gy, mag);
    double smooth = cv::mean(mag, valid)[0];  // smaller means smoother
    std::printf("%-24s valid=%.1f%% mean_disp=%6.2f std=%.2f smoothness=%.3f\n",
                tag, ratio * 100.0, mean[0], stddev[0], smooth);
}

int main(int argc, char** argv) {
    std::string base = "../../data";
    std::string l = base + "/aloeL.jpg";
    std::string r = base + "/aloeR.jpg";
    if (argc > 1) l = argv[1];
    if (argc > 2) r = argv[2];

    cv::Mat imgL = cv::imread(l, cv::IMREAD_GRAYSCALE);
    cv::Mat imgR = cv::imread(r, cv::IMREAD_GRAYSCALE);
    if (imgL.empty() || imgR.empty()) {
        log("stereo", "input empty, synth disks");
        imgL = cv::Mat(320, 400, CV_8UC1);
        cv::ellipse(imgL, cv::Point(200, 160), cv::Size(60, 60), 0, 0, 360, 200, -1);
        imgL = imgL.clone();
        imgR = imgL.clone();
        // artificially create horizontal disparity
        cv::ellipse(imgR, cv::Point(190, 160), cv::Size(60, 60), 0, 0, 360, 200, -1);
    }
    if (imgL.size() != imgR.size()) {
        cv::resize(imgR, imgR, imgL.size(), 0, 0, cv::INTER_AREA);
    }

    ensureDir("../out/algorithms");
    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    panels.push_back(imgL); labels.push_back("left");
    panels.push_back(imgR); labels.push_back("right");

    std::printf("%-24s %s\n", "method", "valid/mean_disp/smoothness");

    constexpr int numDisp = 64;          // must be a multiple of 16
    constexpr int minDisp = 0;

    // ---- StereoBM ----
    for (int bs : {5, 11, 21}) {
        auto bm = cv::StereoBM::create(numDisp, bs);
        bm->setDisp12MaxDiff(1);
        cv::Mat d16;
        bm->compute(imgL, imgR, d16);
        cv::Mat d8 = disparityTo8U(d16);
        reportStats(("StereoBM(bs=" + std::to_string(bs) + ")").c_str(), d16, 16.0);
        panels.push_back(pseudoColor(d8)); labels.push_back("BM bs=" + std::to_string(bs));
    }

    // ---- SGBM ----
    for (int bs : {3, 11}) {
        auto sgbm = cv::StereoSGBM::create(minDisp, numDisp, bs,
                                           8 * 3 * bs * bs, 32 * 3 * bs * bs,
                                           1, 63, 10, 100, 32,
                                           cv::StereoSGBM::MODE_SGBM);
        cv::Mat d16;
        sgbm->compute(imgL, imgR, d16);
        reportStats(("SGBM(bs=" + std::to_string(bs) + ")").c_str(), d16, 16.0);
        panels.push_back(pseudoColor(disparityTo8U(d16))); labels.push_back("SGBM bs=" + std::to_string(bs));
    }

    cv::Mat canvas = gridWithLabels(panels, labels, 2, 30);
    std::string out = "../out/algorithms/stereo_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[stereo] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}
