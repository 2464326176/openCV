// algorithms/template_matching/main.cpp
// 模板匹配: 遍历 OpenCV 各 matchTemplate 方法 + 多尺度定位.
//
// 覆盖:
//   SQDIFF / SQDIFF_NORMED / CCORR / CCORR_NORMED / CCOEFF / CCOEFF_NORMED
//   多尺度 (scale 金字塔) 自选最优尺度返回匹配框
//   TM_ 方法间相似度分数对比
// 输入: data/lena.jpg (场景) + data/lena_tmpl.jpg (模板, 应为其子区域).
// 输出: out/algorithms/template_matching_compare.png + 分数表.
#include "../common/algo_utils.hpp"

#include <cstdio>
#include <iostream>
#include <vector>

using namespace algo;

// 单尺度匹配, 返回 (score, bestLoc). 对 SQDIFF 系列取最小, 其余取最大.
static std::pair<double, cv::Point> matchAt(const cv::Mat& src, const cv::Mat& tpl,
                                            int method) {
    cv::Mat res;
    cv::matchTemplate(src, tpl, res, method);
    double score; cv::Point loc;
    if (method == cv::TM_SQDIFF || method == cv::TM_SQDIFF_NORMED)
        cv::minMaxLoc(res, &score, nullptr, &loc, nullptr);
    else
        cv::minMaxLoc(res, nullptr, &score, nullptr, &loc);
    return {score, loc};
}

// 归一化分数到 [0,1] 用于可视化 (SQDIFF 越小越好 → 取反).
static double normScore(double s, bool sqdiff) {
    return sqdiff ? 1.0 - s : s;
}

int main(int argc, char** argv) {
    std::string scenePath = "../../data/images/lena.jpg";
    std::string tplPath   = "../../data/lena_tmpl.jpg";
    if (argc > 1) scenePath = argv[1];
    if (argc > 2) tplPath   = argv[2];

    cv::Mat scene = cv::imread(scenePath, cv::IMREAD_COLOR);
    cv::Mat tpl   = cv::imread(tplPath, cv::IMREAD_COLOR);
    if (scene.empty() || tpl.empty()) {
        log("template_matching", "input empty, make synthetic");
        scene = cv::Mat(320, 480, CV_8UC3); cv::randu(scene, 0, 256);
        tpl = scene(cv::Rect(80, 60, 100, 100)).clone();
    }
    if (std::max(scene.rows, scene.cols) > 700) {
        double s = 700.0 / std::max(scene.rows, scene.cols);
        cv::resize(scene, scene, cv::Size(), s, s, cv::INTER_AREA);
        cv::resize(tpl, tpl, cv::Size(), s, s, cv::INTER_AREA);
    }
    ensureDir("../out/algorithms");

    std::vector<cv::Mat> panels; std::vector<std::string> labels;
    cv::Mat sceneMarked; scene.copyTo(sceneMarked);

    // ---- 1. 单尺度各 TM_ 方法与分数 ----
    struct Tm { const char* name; int method; double score; cv::Point loc; };
    std::vector<Tm> tms;
    std::printf("%-16s %8s\n", "method", "score");
    for (int m : {cv::TM_SQDIFF, cv::TM_SQDIFF_NORMED, cv::TM_CCORR,
                  cv::TM_CCORR_NORMED, cv::TM_CCOEFF, cv::TM_CCOEFF_NORMED}) {
        auto r = matchAt(scene, tpl, m);
        bool sq = (m == cv::TM_SQDIFF || m == cv::TM_SQDIFF_NORMED);
        tms.push_back({"", m, r.first, r.second});
        const char* nm = (m == cv::TM_SQDIFF) ? "SQDIFF" :
                         (m == cv::TM_SQDIFF_NORMED) ? "SQDIFF_NORMED" :
                         (m == cv::TM_CCORR) ? "CCORR" :
                         (m == cv::TM_CCORR_NORMED) ? "CCORR_NORMED" :
                         (m == cv::TM_CCOEFF) ? "CCOEFF" : "CCOEFF_NORMED";
        tms.back().name = nm;
        std::printf("%-16s %8.4f @ (%d,%d)\n", nm, normScore(r.first, sq),
                    r.second.x, r.second.y);
        // 在独立画布上画匹配框
        cv::Mat c; scene.copyTo(c);
        cv::rectangle(c, cv::Rect(r.second.x, r.second.y, tpl.cols, tpl.rows),
                      cv::Scalar(0, 0, 255), 2);
        panels.push_back(c); labels.push_back(nm);
    }

    // ---- 2. 多尺度: 在尺度和方向上都扫描, 选最优 ----
    struct Best { double score=-1; cv::Rect rect; std::string view= "N/A"; };
    Best best;
    for (double s = 0.5; s <= 2.01; s *= 1.3) {
        int tw = (int)std::lround(tpl.cols * s), th = (int)std::lround(tpl.rows * s);
        if (tw < 8 || th < 8 || tw > scene.cols || th > scene.rows) continue;
        cv::Mat tScaled;
        cv::resize(tpl, tScaled, cv::Size(tw, th), 0, 0, cv::INTER_LINEAR);
        for (int m : {cv::TM_CCOEFF_NORMED, cv::TM_CCORR_NORMED}) {
            auto r = matchAt(scene, tScaled, m);
            if (r.first > best.score) {
                best.score = r.first;
                best.rect = cv::Rect(r.second.x, r.second.y, tw, th);
                char buf[64]; std::snprintf(buf, sizeof(buf), "s=%.2f %s",
                                            s, m == cv::TM_CCOEFF_NORMED ? "NCC" : "NCCor");
                best.view = buf;
            }
        }
    }
    cv::rectangle(sceneMarked, best.rect, cv::Scalar(0, 255, 0), 3);
    cv::putText(sceneMarked, "best multi-scale " + best.view + " score=" +
                    std::to_string(best.score),
                cv::Point(6, 24), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(0, 255, 0), 2);
    std::printf("best multi-scale: %s score=%.4f rect=(%d,%d,%d,%d)\n",
                best.view.c_str(), best.score, best.rect.x, best.rect.y,
                best.rect.width, best.rect.height);
    panels.push_back(sceneMarked); labels.push_back("multi-scale best");

    // 顶部放"模板"面板
    std::vector<cv::Mat> all; std::vector<std::string> allLab;
    all.push_back(tpl); allLab.push_back("template");
    for (size_t i = 0; i < panels.size(); ++i) { all.push_back(panels[i]); allLab.push_back(labels[i]); }
    cv::Mat canvas = gridWithLabels(all, allLab, 3, 28);
    std::string out = "../out/algorithms/template_matching_compare.png";
    cv::imwrite(out, canvas);
    std::cout << "[template_matching] wrote " << out << " (cols=" << canvas.cols
              << " rows=" << canvas.rows << ")\n";
    return 0;
}