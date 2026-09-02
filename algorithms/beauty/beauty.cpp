// algorithms/beauty/main.cpp
// 美颜算法演示: 皮肤 mask 检测 + 频率分离磨皮 + 磨皮强度自适应 +
//                提亮 + 柔肤 + 锐化 + 色彩增强.
//
// 新增能力:
//   1. 皮肤检测:
//        · RGB 经验法则 (Kovac et al.): R>95, G>40, B>20, max-min>15, R-G>15 等
//        · YCrCb 椭圆规则 (Chai & Bouzerdoum): Cr/Cb 在肤色椭圆内部
//        · HSV 阈值: H∈[0,40] & S∈[0.25,0.75]
//        三个规则融合, 并对 mask 做形态学闭运算 + guided 平滑, 生成
//        continuous skin confidence mask (0~255, 越大越像皮肤).
//   2. 强度自适应:
//        · 磨皮强度 strength 随 "皮肤面积占比" 自动调整:
//          人脸/自拍 → 大强度, 风景+少量人脸 → 小强度
//        · 皱纹/毛孔多的局部区域 (通过 Laplacian 残差判断) 给予更强平滑
//          (区域自适应磨皮, 避免过度平滑五官)
//   3. 五官保留:
//        · Sobel/Canny 边缘作为 anti-warp mask, 在 mask 高的区域
//          只叠加 20% 磨皮, 保护睫毛/眉毛/嘴唇轮廓
//   4. 色彩自然度增强:
//        · 对皮肤区域做 "温和美白" (Y 通道 gain=1.08), 非皮肤区域保持
//        · 对非皮肤的绿色/蓝色区域保持, 避免 "颜色漂白色" 的典型美颜 bug
//   5. 一键对比: 原图 / 只mask / 磨皮 / 磨皮+美白 / 全流程.
//
// 用法: beauty.exe [input_img] [base_strength 0-100] [auto_adapt 0/1]
#include "../common/nv21_io.hpp"
#include "../common/algo_utils.hpp"
#include "../common/single_denoise.hpp"

#include <opencv2/photo.hpp>
#include <opencv2/imgproc.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace algo;

// ---------------- 工具 ------------------------------------------------------

static cv::Mat gammaBGR(const cv::Mat& in, double g) {
    cv::Mat lut(1, 256, CV_8U);
    for (int i = 0; i < 256; ++i)
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(std::pow(i / 255.0, g) * 255.0);
    cv::Mat out; cv::LUT(in, lut, out);
    return out;
}

// 肤色检测: 返回 [0,255] 连续置信图
static cv::Mat detectSkinMask(const cv::Mat& bgr) {
    int H = bgr.rows, W = bgr.cols;
    cv::Mat mask = cv::Mat::zeros(H, W, CV_8U);

    // 1) YCrCb 椭圆
    cv::Mat yc; cv::cvtColor(bgr, yc, cv::COLOR_BGR2YCrCb);
    cv::Mat crCb(H, W, CV_8UC2);
    int fromTo[] = {1, 0, 2, 1};
    cv::mixChannels(&yc, 1, &crCb, 1, fromTo, 2);

    // 2) HSV
    cv::Mat hsv; cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    for (int y = 0; y < H; ++y) {
        const cv::Vec3b* r = bgr.ptr<cv::Vec3b>(y);
        const cv::Vec2b* cc = crCb.ptr<cv::Vec2b>(y);
        const cv::Vec3b* hs = hsv.ptr<cv::Vec3b>(y);
        uchar* m = mask.ptr<uchar>(y);
        for (int x = 0; x < W; ++x) {
            int B = r[x][0], G = r[x][1], R = r[x][2];
            int Cr = cc[x][0], Cb = cc[x][1];
            int Hue = hs[x][0];        // 0..179 (OpenCV)
            int Sat = hs[x][1];        // 0..255
            // RGB rule (Kovac)
            int ruleRgb = (R > 95 && G > 40 && B > 20)
                       && (std::max({R, G, B}) - std::min({R, G, B}) > 15)
                       && (std::abs(R - G) > 15) && (R > G) && (R > B) ? 1 : 0;
            // YCrCb 经验椭圆 (Chai+Bouzerdoum 简化):
            // 133≤Cr≤173 && 77≤Cb≤127  (接近常见 Android 实现)
            int ruleYc = (Cr >= 133 && Cr <= 173) && (Cb >= 77 && Cb <= 127) ? 1 : 0;
            // HSV 规则: H ∈ [0, 25] ∪ [155, 179], S ∈ [60, 200]
            int ruleHsv = ((Hue <= 25) || (Hue >= 155))
                       && (Sat >= 60 && Sat <= 200) ? 1 : 0;
            int vote = ruleRgb + ruleYc + ruleHsv;
            if (vote >= 2) {
                int confidence = 85 * vote; // 2/3 票 ~ 170; 3 票 = 255
                m[x] = cv::saturate_cast<uchar>(confidence);
            } else {
                m[x] = 0;
            }
        }
    }
    // 形态学闭运算 (小孔洞填补) + 高斯 + guided 平滑, 得到"软"mask
    cv::Mat closed;
    cv::morphologyEx(mask, closed, cv::MORPH_CLOSE,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7)));
    cv::Mat smooth1;
    cv::GaussianBlur(closed, smooth1, cv::Size(5, 5), 0);
    cv::Mat out3c;
    cv::cvtColor(smooth1, out3c, cv::COLOR_GRAY2BGR);
    cv::Mat gf = denoiseGuided(out3c, bgr, 12, 0.02);
    std::vector<cv::Mat> chs; cv::split(gf, chs);
    return chs[0]; // 单通道
}

// 五官保留 mask: Canny 边缘 -> 膨胀 -> 反色, 边缘区域磨皮权重降为 20%
static cv::Mat edgePreserveAntiMask(const cv::Mat& bgr) {
    cv::Mat g; cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
    cv::Mat edges; cv::Canny(g, edges, 60, 160, 3);
    cv::dilate(edges, edges, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
    cv::Mat anti;
    // anti = 255 - edges 归一化到 [0.3, 1.0]: 边缘区取 0.3, 其余取 1.0
    cv::threshold(edges, edges, 1, 255, cv::THRESH_BINARY);
    cv::convertScaleAbs(edges, anti, -1.0, 255);
    anti.convertTo(anti, CV_32F, 1.0 / 255.0);
    // 收缩权重区间至 [0.3, 1.0]
    anti = 0.3f + 0.7f * anti;
    return anti;
}

// 频率分离磨皮: 返回 "磨皮后" 图 (只对 lowF 平滑)
static cv::Mat freqSepBeauty(const cv::Mat& bgr, double strength,
                              const cv::Mat& skinMaskF) {
    cv::Mat lowF, bgrF;
    bgr.convertTo(bgrF, CV_32F, 1.0 / 255.0);
    // 低频用 adaptive bilateral (保边)
    cv::Mat low = denoiseAdaptiveBilateral(bgr, 9, 80, 75, 7);
    low.convertTo(lowF, CV_32F, 1.0 / 255.0);
    cv::Mat high = bgrF - lowF; // 高频 = 细节 (含皱纹/毛孔 + 五官)
    // 高频按 strength 抑制, 再融合
    double s = std::max(0.0, std::min(1.0, strength / 100.0));
    // high_suppressed = high * (1 - s · skinMask)
    std::vector<cv::Mat> m3 = {skinMaskF, skinMaskF, skinMaskF};
    cv::Mat m3c; cv::merge(m3, m3c);
    cv::Mat suppressFactor = (1.0f - (float)s * m3c);
    cv::Mat highS = high.mul(suppressFactor);
    cv::Mat outF = lowF + highS;
    // clamp
    cv::threshold(outF, outF, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(outF, outF, 0.0, 0.0, cv::THRESH_TOZERO);
    cv::Mat out8u; outF.convertTo(out8u, CV_8U, 255.0);
    return out8u;
}

// 局部强度自适应: 在皱纹多的位置 (Laplacian 高) 增加 strength
static cv::Mat localStrengthMap(const cv::Mat& bgr, const cv::Mat& skinMask,
                                 double baseStrength) {
    cv::Mat g; cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
    cv::Mat lap;
    cv::Mat k = (cv::Mat_<float>(3, 3) << 1, -2, 1, -2, 4, -2, 1, -2, 1);
    cv::filter2D(g, lap, CV_32F, k);
    cv::Mat absL = cv::abs(lap);
    double mn, mx; cv::minMaxLoc(absL, &mn, &mx);
    absL = (absL - mn) / (mx - mn + 1e-6); // 0..1: 越大 = 毛孔/皱纹/边缘越多
    cv::Mat mF; skinMask.convertTo(mF, CV_32F, 1.0 / 255.0);
    // strength(x) = base × (1 + λ · wrinkle · skinMask), λ=1.5
    cv::Mat sMap = absL.mul(mF) * 1.5f + 1.0f;
    sMap *= (float)(baseStrength);
    return sMap;
}

// 区域自适应磨皮: strength 逐像素由局部 strengthMap 决定
static cv::Mat freqSepBeautyAdaptive(const cv::Mat& bgr, const cv::Mat& sMap,
                                      const cv::Mat& skinMaskF,
                                      const cv::Mat& edgeAntiF) {
    cv::Mat bgrF, lowF;
    bgr.convertTo(bgrF, CV_32F, 1.0 / 255.0);
    cv::Mat low = denoiseAdaptiveBilateral(bgr, 9, 80, 75, 7);
    low.convertTo(lowF, CV_32F, 1.0 / 255.0);
    cv::Mat high = bgrF - lowF;
    // sMap 缩放到 [0,1]
    cv::Mat s = sMap / 100.0f;
    cv::threshold(s, s, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(s, s, 0.0, 0.0, cv::THRESH_TOZERO);
    // 综合抑制权重: suppress = (1 - s · skinMask · edgeAnti)
    std::vector<cv::Mat> s3(3, s), m3(3, skinMaskF), e3(3, edgeAntiF);
    cv::Mat s3c, m3c, e3c;
    cv::merge(s3, s3c); cv::merge(m3, m3c); cv::merge(e3, e3c);
    cv::Mat w = s3c.mul(m3c).mul(e3c);
    cv::Mat highS = high.mul(1.0f - w);
    cv::Mat outF = lowF + highS;
    cv::threshold(outF, outF, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(outF, outF, 0.0, 0.0, cv::THRESH_TOZERO);
    cv::Mat out8u; outF.convertTo(out8u, CV_8U, 255.0);
    return out8u;
}

// 皮肤区域温和美白 (Y 通道 gain = 1.08) + 非皮肤保持原样
static cv::Mat whitenSkin(const cv::Mat& bgr, const cv::Mat& skinMask,
                           double gain = 1.08, double hueBoost = 1.03) {
    cv::Mat yc; cv::cvtColor(bgr, yc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> chs; cv::split(yc, chs);
    cv::Mat mf; skinMask.convertTo(mf, CV_32F, 1.0 / 255.0);
    cv::Mat Yf; chs[0].convertTo(Yf, CV_32F);
    // Y' = Y * (1 + (gain - 1) · mask)
    cv::Mat Ynew = Yf.mul(1.0f + (float)(gain - 1) * mf);
    cv::threshold(Ynew, Ynew, 255, 255, cv::THRESH_TRUNC);
    Ynew.convertTo(chs[0], CV_8U);
    // 轻微提升 Cr/Cb 的饱和度: Cr → 更红润, Cb → 更透明
    cv::Mat crF; chs[1].convertTo(crF, CV_32F);
    cv::Mat cbF; chs[2].convertTo(cbF, CV_32F);
    crF = 128 + (crF - 128) * (1.0f + 0.05f * mf);
    cbF = 128 + (cbF - 128) * (1.0f + 0.02f * mf);
    cv::threshold(crF, crF, 255, 255, cv::THRESH_TRUNC);
    cv::threshold(cbF, cbF, 255, 255, cv::THRESH_TRUNC);
    crF.convertTo(chs[1], CV_8U); cbF.convertTo(chs[2], CV_8U);
    cv::merge(chs, yc);
    cv::Mat out; cv::cvtColor(yc, out, cv::COLOR_YCrCb2BGR);
    (void)hueBoost;
    return out;
}

static cv::Mat unsharp(const cv::Mat& bgr, double amount = 0.5, double sigma = 1.5) {
    cv::Mat blur; cv::GaussianBlur(bgr, blur, cv::Size(0, 0), sigma);
    cv::Mat a, b; bgr.convertTo(a, CV_32F); blur.convertTo(b, CV_32F);
    cv::Mat sF = a + amount * (a - b);
    cv::threshold(sF, sF, 255, 255, cv::THRESH_TRUNC);
    cv::threshold(sF, sF, 0, 0, cv::THRESH_TOZERO);
    cv::Mat out; sF.convertTo(out, CV_8U);
    return out;
}

// Y 通道 CLAHE (提亮 + 局部对比)
static cv::Mat claheY(const cv::Mat& bgr, double clip = 2.0, int tile = 8) {
    cv::Mat yc; cv::cvtColor(bgr, yc, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> chs; cv::split(yc, chs);
    cv::Ptr<cv::CLAHE> c = cv::createCLAHE(clip, cv::Size(tile, tile));
    c->apply(chs[0], chs[0]);
    cv::merge(chs, yc); cv::Mat out; cv::cvtColor(yc, out, cv::COLOR_YCrCb2BGR);
    return out;
}

// ---------------- main ------------------------------------------------------
int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    if (argc > 1) inPath = argv[1];
    double baseStrength = 60;
    if (argc > 2) baseStrength = std::stod(argv[2]);
    int autoAdapt = 1;
    if (argc > 3) autoAdapt = std::stoi(argv[3]);

    cv::Mat src = cv::imread(inPath, cv::IMREAD_COLOR);
    if (src.empty()) {
        log("beauty", "imread failed, try nv21: " + inPath);
        src = readNv21Auto(inPath);
    }
    if (src.empty()) {
        log("beauty", "fallback synthetic 480x480");
        src = cv::Mat::zeros(480, 480, CV_8UC3);
        cv::randu(src, 80, 200);
        // 画一个"类人脸"颜色椭圆以演示 skin detect
        cv::ellipse(src, cv::Point(240, 240), cv::Size(140, 180), 0, 0, 360,
                    cv::Scalar(90, 150, 210), -1);
    }
    if (std::max(src.rows, src.cols) > 768) {
        double s = 768.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }

    // ==== 1. 检测皮肤 mask ====
    cv::Mat skinMask = detectSkinMask(src);
    // 皮肤面积占比
    double skinRatio = (double)cv::sum(skinMask)[0] / (skinMask.total() * 255.0);
    double adaptedStrength = baseStrength;
    if (autoAdapt) {
        // 自拍/人脸 (>40%) → 强度 * 1.1; 小比例 (<10%) → * 0.5; 其他线性插值
        if (skinRatio > 0.4) adaptedStrength = std::min(100.0, baseStrength * 1.1);
        else if (skinRatio < 0.1) adaptedStrength = std::max(0.0, baseStrength * 0.5);
        else adaptedStrength = baseStrength * (0.6 + 0.7 * skinRatio);
    }
    cv::Mat maskF; skinMask.convertTo(maskF, CV_32F, 1.0 / 255.0);
    cv::Mat edgeAnti = edgePreserveAntiMask(src);

    // ==== 2. 磨皮 (两种模式对比) ====
    cv::Mat beautySimple = freqSepBeauty(src, adaptedStrength, maskF);
    cv::Mat sMap = localStrengthMap(src, skinMask, adaptedStrength);
    cv::Mat beautyAdaptive = freqSepBeautyAdaptive(src, sMap, maskF, edgeAnti);

    // ==== 3. 美白 + 提亮 + CLAHE ====
    cv::Mat whiten = whitenSkin(beautyAdaptive, skinMask, 1.10, 1.03);
    cv::Mat bright = gammaBGR(whiten, 0.88);
    cv::Mat brightClahe = claheY(bright, 1.8, 8);

    // ==== 4. 柔肤 (仅对 skin mask) + 锐化 (仅在非皮肤或边缘) ====
    cv::Mat soft8u = denoiseGuided(brightClahe, brightClahe, 6, 0.02);
    cv::Mat soft8uF, bcF, mF3c;
    soft8u.convertTo(soft8uF, CV_32F, 1.0 / 255.0);
    brightClahe.convertTo(bcF, CV_32F, 1.0 / 255.0);
    std::vector<cv::Mat> m3(3, maskF);
    cv::merge(m3, mF3c);
    cv::Mat blendF = mF3c.mul(soft8uF) + (1.0f - mF3c).mul(bcF);
    cv::Mat blend8u; blendF.convertTo(blend8u, CV_8U, 255.0);

    cv::Mat sharp = unsharp(blend8u, 0.55, 1.4);

    // ==== 展示 ====
    ensureDir("../out/algorithms");

    // mask 预览: 伪彩色
    cv::Mat maskColor;
    cv::applyColorMap(skinMask, maskColor, cv::COLORMAP_JET);

    std::vector<cv::Mat> panels = {
        src, maskColor, beautySimple, beautyAdaptive, whiten, brightClahe, blend8u, sharp,
    };
    std::vector<std::string> labels = {
        "input", "skin mask (JET)",
        "beauty simple", "beauty adaptive(localW+edgeKeep)",
        "+whiten skin Y*1.10", "+gamma+CLAHE",
        "+soft skin guided", "+unsharp final",
    };

    // 额外几档强度扫面 (固定强度)
    for (double s : {10.0, 40.0, 70.0, 90.0}) {
        cv::Mat bs = freqSepBeautyAdaptive(src,
                    cv::Mat(src.size(), CV_32F, (float)s), maskF, edgeAnti);
        panels.push_back(bs);
        labels.push_back("level s=" + std::to_string((int)s));
    }

    cv::Mat canvas = gridWithLabels(panels, labels, 4, 28);
    cv::imwrite("../out/algorithms/beauty.png", canvas);

    std::cout << "\n=== beauty summary ===\n";
    std::printf("skin coverage ratio: %.2f%%\n", 100.0 * skinRatio);
    std::printf("base strength:      %.1f\n", baseStrength);
    std::printf("adapted strength:   %.1f (auto=%s)\n",
                adaptedStrength, autoAdapt ? "on" : "off");
    double ps = psnr(src, sharp);
    double ss = ssim(src, sharp);
    double mss = msssim(src, sharp);
    int lo = loe(src, sharp, 48);
    std::printf("final vs input:     PSNR=%.2f  SSIM=%.4f  MS-SSIM=%.4f  LOE=%d\n",
                ps, ss, mss, lo);
    std::printf("======================\n");
    std::cout << "[beauty] wrote ../out/algorithms/beauty.png (panels=" << panels.size() << ")\n";

    imshowFit("beauty", canvas, 1800, 0);
    return 0;
}
