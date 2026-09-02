// algorithms/denoise_single/main.cpp
// 单帧降噪算法综合对比演示.
//
// 扩展能力:
//   1. 噪声类型: 高斯 / 椒盐 / 乘性斑点 / 泊松 四种, 支持命令行参数.
//   2. 算法: Gaussian/Median/Bilateral/NLM/Guided + AdaptiveBilateral +
//           Wiener + AnisotropicDiffusion + LaplacianSoftThreshold + denoiseAuto.
//   3. 参数扫描: 例如 sigma=10/20/30 的高斯噪声, 扫描 h={5,10,15} 的 NLM.
//   4. 表格输出 PSNR/SSIM/MAE/MS-SSIM/LOE 五项指标, 并输出算法推荐排名.
//   5. 同时画"原图→噪声→各算法降噪图"的马赛克大图.
//
// 用法: denoise_single.exe [input_img] [noise_type:gauss|sp|speckle|poisson]
//                        [noise_p1] [noise_p2]
#include "../common/algo_utils.hpp"
#include "../common/single_denoise.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace algo;

struct NoiseConfig {
    std::string name;
    double p1, p2;
};

static cv::Mat applyNoise(const cv::Mat& clean, const NoiseConfig& cfg, unsigned seed) {
    if (cfg.name == "gauss")   return addGaussianNoise(clean, cfg.p1, seed);
    if (cfg.name == "sp")      return addSaltPepperNoise(clean, cfg.p1, seed);
    if (cfg.name == "speckle") return addSpeckleNoise(clean, cfg.p1, seed);
    if (cfg.name == "poisson") return addPoissonNoise(clean, cfg.p1, seed);
    return addGaussianNoise(clean, 15, seed);
}

struct Scores {
    double psnr, ssim, msssim, mae;
    int loe;
};

static Scores scoreOf(const cv::Mat& clean, const cv::Mat& test) {
    Scores s{};
    cv::Mat c, t;
    cv::Size sz(std::min(clean.cols, test.cols), std::min(clean.rows, test.rows));
    if (clean.size() != sz) cv::resize(clean, c, sz); else c = clean;
    if (test.size() != sz) cv::resize(test, t, sz); else t = test;
    s.psnr = psnr(c, t);
    s.ssim = ssim(c, t);
    s.msssim = msssim(c, t);
    s.mae = mae(c, t);
    s.loe = loe(c, t, 48);
    return s;
}

struct ResultRow {
    std::string tag;
    cv::Mat img;
    Scores s;
};

static void compareMany(const std::vector<NoiseConfig>& noises, const cv::Mat& clean) {
    ensureDir("../out/algorithms");
    for (const auto& nc : noises) {
        std::cout << "\n=============================\n"
                  << " Noise: " << nc.name
                  << " (p1=" << std::fixed << std::setprecision(2) << nc.p1
                  << " p2=" << nc.p2 << ")\n=============================\n";
        cv::Mat noisy = applyNoise(clean, nc, 42);
        double estimatedSigma = 0;
        {
            cv::Mat g;
            if (noisy.channels() == 3) cv::cvtColor(noisy, g, cv::COLOR_BGR2GRAY);
            else g = noisy;
            estimatedSigma = estimateNoiseSigma(g);
        }
        std::string autoMethod;
        cv::Mat autoOut = denoiseAuto(noisy, &autoMethod);

        std::vector<ResultRow> rows = {
            {"noisy", noisy, scoreOf(clean, noisy)},
            {"Gauss5x5", denoiseGaussian(noisy, 5), {}},
            {"Median5x5", denoiseMedian(noisy, 5), {}},
            {"Bilateral(d9,75,75)", denoiseBilateral(noisy, 9, 75, 75), {}},
            {"AdaptiveBilateral", denoiseAdaptiveBilateral(noisy, 9, 80, 70, 7), {}},
            {"NLM(h=10)", denoiseNLM(noisy, 10, 7, 21), {}},
            {"NLM(h=15)", denoiseNLM(noisy, 15, 7, 21), {}},
            {"Guided(r8,eps=0.01)", denoiseGuided(noisy, 8, 0.01), {}},
            {"Wiener(k=5)", denoiseWiener(noisy, 5), {}},
            {"PM-Diffusion(it20,K20)", denoiseAnisotropicDiffusion(noisy, 20, 20.0, 0.15), {}},
            {"LaplaceSoft(l4)", denoiseLaplacianSoftThreshold(noisy, 4), {}},
            {"Auto[" + autoMethod + "]", autoOut, {}},
        };

        // 打分 (skip noisy)
        for (size_t i = 1; i < rows.size(); ++i) rows[i].s = scoreOf(clean, rows[i].img);

        // 排序 (按 PSNR desc)
        std::vector<size_t> idx(rows.size());
        for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
        std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b){
            return rows[a].s.psnr > rows[b].s.psnr;
        });

        // 打印表格
        std::printf("estimated sigma via MAD: %.2f\n", estimatedSigma);
        std::printf("%-28s %8s %8s %8s %8s %8s\n",
                    "method", "PSNR", "SSIM", "MS-SSIM", "MAE", "LOE");
        for (auto i : idx) {
            const auto& r = rows[i];
            std::printf("%-28s %8.2f %8.4f %8.4f %8.3f %8d\n",
                        r.tag.c_str(), r.s.psnr, r.s.ssim, r.s.msssim,
                        r.s.mae, r.s.loe);
        }

        // 构造大图 4 列
        std::vector<cv::Mat> panels;
        std::vector<std::string> labels;
        panels.push_back(clean); labels.push_back("clean");
        for (auto& r : rows) { panels.push_back(r.img); labels.push_back(r.tag); }
        cv::Mat canvas = gridWithLabels(panels, labels, 4, 30);
        std::string name = "../out/algorithms/denoise_single_" + nc.name + ".png";
        cv::imwrite(name, canvas);
        std::cout << "[denoise_single] wrote " << name << "\n";
    }
}

int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    std::string noiseType = "gauss";
    double p1 = 15, p2 = 0;
    if (argc > 1) inPath = argv[1];
    if (argc > 2) noiseType = argv[2];
    if (argc > 3) p1 = std::stod(argv[3]);
    if (argc > 4) p2 = std::stod(argv[4]);

    cv::Mat clean = cv::imread(inPath, cv::IMREAD_COLOR);
    if (clean.empty()) {
        log("denoise_single", "input empty: " + inPath + ", fallback synthetic");
        clean = cv::Mat(512, 512, CV_8UC3);
        cv::randu(clean, 0, 256);
    }
    if (std::max(clean.rows, clean.cols) > 720) {
        double s = 720.0 / std::max(clean.rows, clean.cols);
        cv::resize(clean, clean, cv::Size(), s, s, cv::INTER_AREA);
    }

    // 传参模式: 单一噪声
    std::vector<NoiseConfig> cfg;
    if (argc > 2) {
        cfg.push_back({noiseType, p1, p2});
    } else {
        // 默认参数扫描: 4 种噪声 × 2 档强度
        cfg = {
            {"gauss", 10, 0}, {"gauss", 25, 0},
            {"sp", 0.03, 0},  {"sp", 0.08, 0},
            {"speckle", 0.1, 0}, {"speckle", 0.25, 0},
            {"poisson", 5.0, 0}, {"poisson", 1.0, 0},
        };
    }
    compareMany(cfg, clean);
    return 0;
}
