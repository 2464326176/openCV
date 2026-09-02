// algorithms/beauty/main.cpp
// Beauty algorithm demo: skin mask detection + frequency-separation skin smoothing +
// adaptive smoothing strength + brightening + softening + sharpening + color enhancement.
//
// Key capabilities:
//   1. Skin detection:
//        · RGB heuristic rule (Kovac et al.): R>95, G>40, B>20, max-min>15, R-G>15, etc.
//        · YCrCb ellipse rule (Chai & Bouzerdoum): Cr/Cb inside the skin-color ellipse
//        · HSV thresholds: H∈[0,40] & S∈[0.25,0.75]
//        The three rules are fused; the mask is then closed morphologically + smoothed with
//        a guided filter to produce a continuous skin confidence mask
//        (0~255, larger means more skin-like).
//   2. Strength adaptation:
//        · smoothing strength adapts automatically to the "skin area ratio":
//          portrait/selfie -> larger strength, landscape with few faces -> smaller strength
//        · local regions with more wrinkles/pores (detected via Laplacian residual) get
//          stronger smoothing (region-adaptive smoothing, avoiding over-smoothing features)
//   3. Feature preservation:
//        · Sobel/Canny edges act as an anti-warp mask; in high-mask regions only 20% of the
//          smoothing is applied, protecting eyelashes/eyebrows/lip contours
//   4. Color naturalness enhancement:
//        · gentle whitening on skin regions (Y channel gain=1.08), non-skin regions unchanged
//        · green/blue non-skin regions are preserved, avoiding the typical beauty bug of color drift
//   5. One-shot comparison: original / mask only / smoothing / smoothing+whitening / full pipeline.
//
// Usage: beauty.exe [input_img] [base_strength 0-100] [auto_adapt 0/1]
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

// ---------------- utilities -------------------------------------------------

static cv::Mat gammaBGR(const cv::Mat& in, double g) {
    cv::Mat lut(1, 256, CV_8U);
    for (int i = 0; i < 256; ++i)
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(std::pow(i / 255.0, g) * 255.0);
    cv::Mat out; cv::LUT(in, lut, out);
    return out;
}

// Skin detection: returns a [0,255] continuous confidence map
static cv::Mat detectSkinMask(const cv::Mat& bgr) {
    int H = bgr.rows, W = bgr.cols;
    cv::Mat mask = cv::Mat::zeros(H, W, CV_8U);

    // 1) YCrCb ellipse
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
            // YCrCb empirical ellipse (simplified Chai+Bouzerdoum):
            // 133≤Cr≤173 && 77≤Cb≤127  (close to common Android implementations)
            int ruleYc = (Cr >= 133 && Cr <= 173) && (Cb >= 77 && Cb <= 127) ? 1 : 0;
            // HSV rule: H ∈ [0, 25] ∪ [155, 179], S ∈ [60, 200]
            int ruleHsv = ((Hue <= 25) || (Hue >= 155))
                       && (Sat >= 60 && Sat <= 200) ? 1 : 0;
            int vote = ruleRgb + ruleYc + ruleHsv;
            if (vote >= 2) {
                int confidence = 85 * vote; // 2/3 votes ~ 170; 3 votes = 255
                m[x] = cv::saturate_cast<uchar>(confidence);
            } else {
                m[x] = 0;
            }
        }
    }
    // Morphological closing (fill small holes) + Gaussian + guided smoothing to get a "soft" mask
    cv::Mat closed;
    cv::morphologyEx(mask, closed, cv::MORPH_CLOSE,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7)));
    cv::Mat smooth1;
    cv::GaussianBlur(closed, smooth1, cv::Size(5, 5), 0);
    cv::Mat out3c;
    cv::cvtColor(smooth1, out3c, cv::COLOR_GRAY2BGR);
    cv::Mat gf = denoiseGuided(out3c, bgr, 12, 0.02);
    std::vector<cv::Mat> chs; cv::split(gf, chs);
    return chs[0]; // single channel
}

// Feature-preservation mask: Canny edges -> dilate -> invert, edge regions get smoothing weight down to 20%
static cv::Mat edgePreserveAntiMask(const cv::Mat& bgr) {
    cv::Mat g; cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
    cv::Mat edges; cv::Canny(g, edges, 60, 160, 3);
    cv::dilate(edges, edges, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
    cv::Mat anti;
    // anti = 255 - edges normalized to [0.3, 1.0]: edges take 0.3, the rest 1.0
    cv::threshold(edges, edges, 1, 255, cv::THRESH_BINARY);
    cv::convertScaleAbs(edges, anti, -1.0, 255);
    anti.convertTo(anti, CV_32F, 1.0 / 255.0);
    // shrink the weight range to [0.3, 1.0]
    anti = 0.3f + 0.7f * anti;
    return anti;
}

// Frequency-separation smoothing: returns the "smoothed" image (only lowF is smoothed)
static cv::Mat freqSepBeauty(const cv::Mat& bgr, double strength,
                              const cv::Mat& skinMaskF) {
    cv::Mat lowF, bgrF;
    bgr.convertTo(bgrF, CV_32F, 1.0 / 255.0);
    // low frequency uses adaptive bilateral (edge-preserving)
    cv::Mat low = denoiseAdaptiveBilateral(bgr, 9, 80, 75, 7);
    low.convertTo(lowF, CV_32F, 1.0 / 255.0);
    cv::Mat high = bgrF - lowF; // high frequency = details (wrinkles/pores + facial features)
    // suppress the high frequency by strength, then fuse
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

// Local strength adaptation: increase strength where wrinkles are dense (high Laplacian)
static cv::Mat localStrengthMap(const cv::Mat& bgr, const cv::Mat& skinMask,
                                 double baseStrength) {
    cv::Mat g; cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
    cv::Mat lap;
    cv::Mat k = (cv::Mat_<float>(3, 3) << 1, -2, 1, -2, 4, -2, 1, -2, 1);
    cv::filter2D(g, lap, CV_32F, k);
    cv::Mat absL = cv::abs(lap);
    double mn, mx; cv::minMaxLoc(absL, &mn, &mx);
    absL = (absL - mn) / (mx - mn + 1e-6); // 0..1: larger = more pores/wrinkles/edges
    cv::Mat mF; skinMask.convertTo(mF, CV_32F, 1.0 / 255.0);
    // strength(x) = base × (1 + λ · wrinkle · skinMask), λ=1.5
    cv::Mat sMap = absL.mul(mF) * 1.5f + 1.0f;
    sMap *= (float)(baseStrength);
    return sMap;
}

// Region-adaptive smoothing: per-pixel strength determined by the local strengthMap
static cv::Mat freqSepBeautyAdaptive(const cv::Mat& bgr, const cv::Mat& sMap,
                                      const cv::Mat& skinMaskF,
                                      const cv::Mat& edgeAntiF) {
    cv::Mat bgrF, lowF;
    bgr.convertTo(bgrF, CV_32F, 1.0 / 255.0);
    cv::Mat low = denoiseAdaptiveBilateral(bgr, 9, 80, 75, 7);
    low.convertTo(lowF, CV_32F, 1.0 / 255.0);
    cv::Mat high = bgrF - lowF;
    // scale sMap to [0,1]
    cv::Mat s = sMap / 100.0f;
    cv::threshold(s, s, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(s, s, 0.0, 0.0, cv::THRESH_TOZERO);
    // combined suppression weight: suppress = (1 - s · skinMask · edgeAnti)
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

// Gentle whitening of skin regions (Y channel gain = 1.08) + non-skin regions unchanged
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
    // slightly boost Cr/Cb saturation: Cr -> more rosy, Cb -> more transparent
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

// Y-channel CLAHE (brightening + local contrast)
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
        // draw a "face-like" color ellipse to demo skin detection
        cv::ellipse(src, cv::Point(240, 240), cv::Size(140, 180), 0, 0, 360,
                    cv::Scalar(90, 150, 210), -1);
    }
    if (std::max(src.rows, src.cols) > 768) {
        double s = 768.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }

    // ==== 1. detect skin mask ====
    cv::Mat skinMask = detectSkinMask(src);
    // skin area ratio
    double skinRatio = (double)cv::sum(skinMask)[0] / (skinMask.total() * 255.0);
    double adaptedStrength = baseStrength;
    if (autoAdapt) {
        // selfie/face (>40%) -> strength * 1.1; small ratio (<10%) -> * 0.5; others linearly interpolated
        if (skinRatio > 0.4) adaptedStrength = std::min(100.0, baseStrength * 1.1);
        else if (skinRatio < 0.1) adaptedStrength = std::max(0.0, baseStrength * 0.5);
        else adaptedStrength = baseStrength * (0.6 + 0.7 * skinRatio);
    }
    cv::Mat maskF; skinMask.convertTo(maskF, CV_32F, 1.0 / 255.0);
    cv::Mat edgeAnti = edgePreserveAntiMask(src);

    // ==== 2. skin smoothing (two modes compared) ====
    cv::Mat beautySimple = freqSepBeauty(src, adaptedStrength, maskF);
    cv::Mat sMap = localStrengthMap(src, skinMask, adaptedStrength);
    cv::Mat beautyAdaptive = freqSepBeautyAdaptive(src, sMap, maskF, edgeAnti);

    // ==== 3. whitening + brightening + CLAHE ====
    cv::Mat whiten = whitenSkin(beautyAdaptive, skinMask, 1.10, 1.03);
    cv::Mat bright = gammaBGR(whiten, 0.88);
    cv::Mat brightClahe = claheY(bright, 1.8, 8);

    // ==== 4. softening (skin mask only) + sharpening (non-skin or edges) ====
    cv::Mat soft8u = denoiseGuided(brightClahe, brightClahe, 6, 0.02);
    cv::Mat soft8uF, bcF, mF3c;
    soft8u.convertTo(soft8uF, CV_32F, 1.0 / 255.0);
    brightClahe.convertTo(bcF, CV_32F, 1.0 / 255.0);
    std::vector<cv::Mat> m3(3, maskF);
    cv::merge(m3, mF3c);
    cv::Mat blendF = mF3c.mul(soft8uF) + (1.0f - mF3c).mul(bcF);
    cv::Mat blend8u; blendF.convertTo(blend8u, CV_8U, 255.0);

    cv::Mat sharp = unsharp(blend8u, 0.55, 1.4);

    // ==== display ====
    ensureDir("../out/algorithms");

    // mask preview: pseudo color
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

    // extra fixed-strength sweeps
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
