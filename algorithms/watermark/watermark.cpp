// algorithms/watermark/main.cpp
// Watermark algorithm demo: visible watermark overlay + DFT-domain invisible watermark
// + DCT-domain invisible watermark (8x8 blocks) + robustness attack tests.
//
// 1. Visible watermark:
//    - text watermark overlaid at bottom-right, alpha blend + outline for readability
//    - logo watermark (if data/images/opencv-logo.png exists) tiled semi-transparent
//    - texture watermark: diagonal/grid semi-transparent overlay
// 2. DFT-domain watermark (Cox 1997 style):
//    - generate 32x32 binary watermark bits (procedural pattern)
//    - embed watermark bits into the mid-frequency region of the source DFT magnitude spectrum
//      (modify log|F| at a conjugate pair)
//    - IDFT yields a visually indistinguishable watermarked image
//    - extract: DFT the watermarked image, take the magnitude difference at embedding positions,
//      threshold to recover bits (non-blind: needs the source)
// 3. DCT-domain block watermark (Kutter style, JPEG-robust):
//    - split the image into 8x8 blocks, DCT each block
//    - embed 1 bit in each block's mid-frequency coefficients (e.g. (4,1),(3,2),(2,3),(1,4))
//    - quantization index modulation (QIM) style: quantize the coefficient near an even/odd multiple of alpha
//    - extract: check whether the coefficient is closer to an even or odd multiple -> bit (blind, no source needed)
// 4. Robustness attack test suite:
//    - JPEG compression (Quality 90/70/50/30)
//    - Gaussian noise (sigma=5/10/20)
//    - salt-and-pepper noise (density 0.5%/1%/3%)
//    - median filter (3x3/5x5/7x7)
//    - mean filter (3x3/5x5)
//    - rotation (±5°, ±15°, bilinear interp + crop back to original size)
//    - scaling round-trip (0.8x -> 1.25x, 0.5x -> 2.0x)
//    - cropping (5%/10% black border filled back)
//    - brightness adjustment (±20, ±50)
//    - contrast adjustment (factor 0.8 / 1.2)
//    - histogram equalization
//    - combined attack: JPEG Q50 + Gaussian noise sigma=10 + 3x3 median
//
// Usage: watermark.exe [input_image]
#include "../common/nv21_io.hpp"
#include "../common/algo_utils.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include <bitset>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <iomanip>
#include <sstream>

using namespace algo;

// ===========================================================================
// Utility: watermark bit correct recovery rate
// ===========================================================================
static double bitRecoveryRate(const cv::Mat& origMark, const cv::Mat& extractedMark) {
    CV_Assert(origMark.size() == extractedMark.size());
    int correct = 0, total = 0;
    for (int y = 0; y < origMark.rows; ++y) {
        for (int x = 0; x < origMark.cols; ++x) {
            total++;
            bool a = origMark.at<uchar>(y, x) > 0;
            bool b = extractedMark.at<uchar>(y, x) > 0;
            if (a == b) correct++;
        }
    }
    return total ? 100.0 * correct / total : 0.0;
}

// ===========================================================================
// Visible watermark: text + logo + texture
// ===========================================================================
static cv::Mat visibleTextWatermark(const cv::Mat& src, const std::string& text,
                                      double alpha = 0.35) {
    cv::Mat out = src.clone();
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = std::max(0.6, src.cols / 600.0);
    int thickness = 2;
    int base = 0;
    cv::Size ts = cv::getTextSize(text, fontFace, fontScale, thickness, &base);
    int x = src.cols - ts.width - 20;
    int y = src.rows - ts.height - 20;
    cv::Mat overlay = out.clone();
    // shadow
    cv::putText(overlay, text, cv::Point(x + 2, y + ts.height + 2), fontFace,
                fontScale, cv::Scalar(0, 0, 0), thickness + 1, cv::LINE_AA);
    // white text
    cv::putText(overlay, text, cv::Point(x, y + ts.height), fontFace,
                fontScale, cv::Scalar(255, 255, 255, 255), thickness, cv::LINE_AA);
    cv::addWeighted(overlay, alpha, out, 1.0 - alpha, 0, out);
    return out;
}

static cv::Mat visibleLogoWatermark(const cv::Mat& src, const cv::Mat& logo,
                                     double alpha = 0.25) {
    if (logo.empty()) return src.clone();
    cv::Mat out = src.clone();
    int nCols = src.cols / (logo.cols + 10);
    int nRows = src.rows / (logo.rows + 10);
    for (int r = 0; r <= nRows; ++r) {
        for (int c = 0; c <= nCols; ++c) {
            int x = c * (logo.cols + 10);
            int y = r * (logo.rows + 10);
            if (x + logo.cols > src.cols || y + logo.rows > src.rows) continue;
            cv::Rect roi(x, y, logo.cols, logo.rows);
            cv::Mat region = out(roi);
            cv::Mat blended;
            if (logo.channels() == 4) {
                std::vector<cv::Mat> chs;
                cv::split(logo, chs);
                cv::Mat mask;
                cv::threshold(chs[3], mask, 0, 255, cv::THRESH_BINARY);
                cv::Mat rgb;
                std::vector<cv::Mat> rgbChs = {chs[0], chs[1], chs[2]};
                cv::merge(rgbChs, rgb);
                cv::Mat fg;
                cv::addWeighted(rgb, alpha, region, 1.0 - alpha, 0, fg);
                fg.copyTo(region, mask);
            } else {
                cv::addWeighted(region, 1.0 - alpha, logo, alpha, 0, region);
            }
        }
    }
    return out;
}

// Texture watermark: generate a diagonal-line grid overlay (can be used as anti-counterfeit background)
static cv::Mat visibleTextureWatermark(const cv::Mat& src, double alpha = 0.12,
                                        int spacing = 24, int angleDeg = 30) {
    cv::Mat overlay = src.clone();
    double angle = angleDeg * CV_PI / 180.0;
    double tanA = std::tan(angle);
    cv::Scalar color(200, 200, 200);
    if (src.channels() == 1) color = cv::Scalar(180);

    // draw a set of diagonal lines: y = tanA * x + c
    int diag = (int)std::sqrt((double)src.cols * src.cols + (double)src.rows * src.rows);
    for (int c = -diag; c <= diag; c += spacing) {
        std::vector<cv::Point> pts;
        for (int x = -diag; x <= diag; x += 2) {
            int y = (int)(tanA * x + c);
            if (x >= 0 && x < src.cols && y >= 0 && y < src.rows) {
                pts.push_back(cv::Point(x, y));
            }
        }
        if (pts.size() >= 2) {
            for (size_t i = 0; i + 1 < pts.size(); ++i) {
                cv::line(overlay, pts[i], pts[i + 1], color, 1, cv::LINE_AA);
            }
        }
    }
    cv::Mat out;
    cv::addWeighted(overlay, alpha, src, 1.0 - alpha, 0, out);
    return out;
}

// ===========================================================================
// DFT-domain invisible watermark (Cox, non-blind extraction)
// ===========================================================================
static cv::Mat optimalDftPad(const cv::Mat& src) {
    cv::Mat padded;
    int m = cv::getOptimalDFTSize(src.rows);
    int n = cv::getOptimalDFTSize(src.cols);
    cv::copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols,
                        cv::BORDER_REPLICATE);
    return padded;
}

static cv::Mat dftLogMagGray(const cv::Mat& in) {
    cv::Mat g;
    if (in.channels() == 3) cv::cvtColor(in, g, cv::COLOR_BGR2GRAY);
    else g = in.clone();
    g.convertTo(g, CV_32F);
    cv::Mat padded = optimalDftPad(g);
    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);
    cv::dft(complexI, complexI, cv::DFT_COMPLEX_OUTPUT);
    cv::split(complexI, planes);
    cv::Mat mag;
    cv::magnitude(planes[0], planes[1], mag);
    cv::log(mag + 1.0, mag);
    return mag;
}

// Embed: for 3 channels, embed into the Y channel of YCrCb, then convert back to BGR to keep color
static cv::Mat dftWatermarkEmbed(const cv::Mat& src8u, const cv::Mat& mark32,
                                  double alpha = 2.0) {
    cv::Mat work;
    bool isColor = (src8u.channels() == 3);
    cv::Mat gray;
    if (isColor) {
        cv::Mat ycrcb;
        cv::cvtColor(src8u, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> chs(3);
        cv::split(ycrcb, chs);
        gray = chs[0];
        work = src8u.clone();  // placeholder, will be overwritten later
    } else {
        gray = src8u.clone();
    }
    gray.convertTo(gray, CV_32F);
    cv::Mat padded = optimalDftPad(gray);
    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);
    cv::dft(complexI, complexI, cv::DFT_COMPLEX_OUTPUT);
    cv::split(complexI, planes);
    cv::Mat mag;
    cv::magnitude(planes[0], planes[1], mag);
    cv::Mat phase;
    cv::phase(planes[0], planes[1], phase);
    cv::Mat magLog;
    cv::log(mag + 1.0, magLog);
    int H = magLog.rows, W = magLog.cols;
    int cy = H / 2, cx = W / 2;
    int r0 = 32;
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            double v = mark32.at<uchar>(y, x) ? alpha : -alpha;
            int dy = r0 + y;
            int dx = r0 + x;
            if (cy + dy < H && cx + dx < W) magLog.at<float>(cy + dy, cx + dx) += v;
            if (cy - dy >= 0 && cx - dx >= 0) magLog.at<float>(cy - dy, cx - dx) += v;
        }
    }
    cv::exp(magLog, mag);
    cv::Mat re, im;
    cv::polarToCart(mag, phase, re, im, false);
    planes[0] = re;
    planes[1] = im;
    cv::merge(planes, 2, complexI);
    cv::Mat inv;
    cv::idft(complexI, inv, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
    cv::Mat out8u;
    inv.convertTo(out8u, CV_8U);
    cv::Mat cropped = out8u(cv::Rect(0, 0, src8u.cols, src8u.rows)).clone();

    if (isColor) {
        cv::Mat ycrcb;
        cv::cvtColor(src8u, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> chs(3);
        cv::split(ycrcb, chs);
        chs[0] = cropped;
        cv::Mat merged;
        cv::merge(chs, merged);
        cv::Mat bgr;
        cv::cvtColor(merged, bgr, cv::COLOR_YCrCb2BGR);
        return bgr;
    }
    return cropped;
}

static cv::Mat dftWatermarkExtract(const cv::Mat& marked8u, const cv::Mat& ref8u) {
    cv::Mat magMarked = dftLogMagGray(marked8u);
    cv::Mat magRef    = dftLogMagGray(ref8u);
    int H = magMarked.rows, W = magMarked.cols;
    int cy = H / 2, cx = W / 2;
    cv::Mat out(32, 32, CV_8U);
    int r0 = 32;
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            int dy = r0 + y, dx = r0 + x;
            double s = 0;
            int cnt = 0;
            if (cy + dy < H && cx + dx < W) {
                s += (magMarked.at<float>(cy + dy, cx + dx) -
                      magRef.at<float>(cy + dy, cx + dx));
                ++cnt;
            }
            if (cy - dy >= 0 && cx - dx >= 0) {
                s += (magMarked.at<float>(cy - dy, cx - dx) -
                      magRef.at<float>(cy - dy, cx - dx));
                ++cnt;
            }
            s = cnt ? s / cnt : 0;
            out.at<uchar>(y, x) = (s > 0) ? 255 : 0;
        }
    }
    return out;
}

// ===========================================================================
// DCT-domain 8x8 block watermark (QIM quantization index modulation, blind extraction)
// ===========================================================================
// Watermark bit layout: unfold the 32x32 mark into 1024 bits, scan 8x8 blocks in order, 1 bit per block
// Embed into the average of 4 mid-frequency coefficients per block for robustness
// 4 mid-frequency coordinates: (1,5),(2,4),(3,3),(4,2) -> classic ZigZag mid-late stage
struct DctWatermarkCtx {
    int blockSize = 8;
    double alpha = 6.0;       // quantization step; larger => more robust but more distortion
    // 4 mid-frequency coefficient coordinates (u, v)
    std::vector<cv::Point> midFreq = { {1,5},{2,4},{3,3},{4,2},{5,1} };
};

// For an 8x8 float block, compute the average of the 4 mid-frequency coefficients
static double dctBlockMidMean(const cv::Mat& blockDct, const std::vector<cv::Point>& pts) {
    double s = 0;
    for (auto& p : pts) s += blockDct.at<float>(p.y, p.x);
    return s / pts.size();
}

// Shift the 4 mid-frequency coefficients toward the target by delta (keep the overall offset consistent)
static void dctBlockShiftMid(cv::Mat& blockDct, const std::vector<cv::Point>& pts, double delta) {
    for (auto& p : pts) blockDct.at<float>(p.y, p.x) += (float)delta;
}

// QIM quantization: given step alpha, make midMean close to k*alpha (bit=0) or (k+0.5)*alpha (bit=1)
// Method: compute the nearest even multiple q0, then offset by 0 or alpha/2 depending on the bit
static void qimQuantize(cv::Mat& blockDct, const std::vector<cv::Point>& pts,
                        double alpha, bool bit) {
    double m = dctBlockMidMean(blockDct, pts);
    double q = std::round(m / alpha) * alpha;  // nearest integer multiple
    double target;
    if (bit == 0) {
        // Snap to even multiple: if q is already aligned use q, otherwise use q
        target = std::round(m / (2 * alpha)) * 2 * alpha;
    } else {
        // Snap to odd multiple offset by alpha/2: i.e. ..., q-alpha/2, q+alpha/2, ...
        double base = std::round(m / alpha - 0.5) * alpha;
        target = base + alpha / 2.0;
    }
    double delta = target - m;
    dctBlockShiftMid(blockDct, pts, delta);
}

// Blind bit extraction: check whether closer to the even multiple (0) or the center offset by alpha/2 (1)
static bool qimExtractBit(const cv::Mat& blockDct, const std::vector<cv::Point>& pts,
                          double alpha) {
    double m = dctBlockMidMean(blockDct, pts);
    double d0 = std::fabs(m - std::round(m / (2 * alpha)) * 2 * alpha);
    double center = std::round(m / alpha - 0.5) * alpha + alpha / 2.0;
    double d1 = std::fabs(m - center);
    return d1 < d0;
}

// Embed DCT watermark: 32x32 mark -> 1024 bits, needs at least 1024 8x8 blocks
// For color images, embed into the Y channel
static cv::Mat dctWatermarkEmbed(const cv::Mat& src8u, const cv::Mat& mark32,
                                  const DctWatermarkCtx& ctx) {
    CV_Assert(mark32.rows == 32 && mark32.cols == 32);
    const int bitTotal = 32 * 32;

    cv::Mat out = src8u.clone();
    cv::Mat grayF;
    bool isColor = (src8u.channels() == 3);
    std::vector<cv::Mat> ycrcbChs;
    cv::Mat yCh;
    if (isColor) {
        cv::Mat ycrcb;
        cv::cvtColor(src8u, ycrcb, cv::COLOR_BGR2YCrCb);
        ycrcbChs.resize(3);
        cv::split(ycrcb, ycrcbChs);
        yCh = ycrcbChs[0];
        yCh.convertTo(grayF, CV_32F);
    } else {
        out.convertTo(grayF, CV_32F);
    }
    // compute the number of blocks needed
    int bh = grayF.rows / ctx.blockSize;
    int bw = grayF.cols / ctx.blockSize;
    int blockCnt = bh * bw;
    if (blockCnt < bitTotal) {
        std::fprintf(stderr, "dctWatermarkEmbed: need >=%d blocks (%dx%d), have %d\n",
                     bitTotal, bh, bw, blockCnt);
        return src8u.clone();
    }

    // fill bits in order
    std::vector<bool> bits(bitTotal);
    for (int i = 0; i < bitTotal; ++i) {
        int y = i / 32, x = i % 32;
        bits[i] = mark32.at<uchar>(y, x) > 0;
    }

    int bitIdx = 0;
    for (int by = 0; by < bh && bitIdx < bitTotal; ++by) {
        for (int bx = 0; bx < bw && bitIdx < bitTotal; ++bx) {
            cv::Rect roi(bx * ctx.blockSize, by * ctx.blockSize,
                         ctx.blockSize, ctx.blockSize);
            cv::Mat block = grayF(roi);
            cv::Mat blockDct;
            cv::dct(block, blockDct);
            qimQuantize(blockDct, ctx.midFreq, ctx.alpha, bits[bitIdx]);
            cv::Mat blockIdct;
            cv::idct(blockDct, blockIdct);
            blockIdct.copyTo(block);
            bitIdx++;
        }
    }

    cv::Mat gray8u;
    grayF.convertTo(gray8u, CV_8U);
    if (isColor) {
        ycrcbChs[0] = gray8u;
        cv::Mat merged;
        cv::merge(ycrcbChs, merged);
        cv::Mat bgr;
        cv::cvtColor(merged, bgr, cv::COLOR_YCrCb2BGR);
        return bgr;
    }
    return gray8u;
}

// Blind extraction of DCT watermark
static cv::Mat dctWatermarkExtract(const cv::Mat& marked8u, const DctWatermarkCtx& ctx) {
    const int bitTotal = 32 * 32;
    cv::Mat grayF;
    if (marked8u.channels() == 3) {
        cv::Mat ycrcb;
        cv::cvtColor(marked8u, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> chs(3);
        cv::split(ycrcb, chs);
        chs[0].convertTo(grayF, CV_32F);
    } else {
        marked8u.convertTo(grayF, CV_32F);
    }
    int bh = grayF.rows / ctx.blockSize;
    int bw = grayF.cols / ctx.blockSize;
    cv::Mat mark = cv::Mat::zeros(32, 32, CV_8U);
    int bitIdx = 0;
    for (int by = 0; by < bh && bitIdx < bitTotal; ++by) {
        for (int bx = 0; bx < bw && bitIdx < bitTotal; ++bx) {
            cv::Rect roi(bx * ctx.blockSize, by * ctx.blockSize,
                         ctx.blockSize, ctx.blockSize);
            cv::Mat block = grayF(roi);
            cv::Mat blockDct;
            cv::dct(block, blockDct);
            bool bit = qimExtractBit(blockDct, ctx.midFreq, ctx.alpha);
            int y = bitIdx / 32, x = bitIdx % 32;
            mark.at<uchar>(y, x) = bit ? 255 : 0;
            bitIdx++;
        }
    }
    return mark;
}

// ===========================================================================
// Watermark pattern generation: procedural 32x32 (with centered "T" + outer frame + 4 corner dot matrices)
// ===========================================================================
static cv::Mat buildMark(const std::string& seedText = "TRAE-WATERMARK-2026") {
    // Fixed-seed pseudo-random: hash based on seedText
    std::size_t seed = 0;
    for (char c : seedText) seed = seed * 131 + (unsigned char)c;
    cv::RNG rng((uint64_t)seed);
    cv::Mat m = cv::Mat::zeros(32, 32, CV_8U);
    // 1) outer frame
    for (int y = 0; y < 32; ++y) for (int x = 0; x < 32; ++x) {
        if (x == 0 || x == 31 || y == 0 || y == 31)
            m.at<uchar>(y, x) = 255;
    }
    // 2) centered "T"
    for (int y = 4; y <= 8; ++y) for (int x = 4; x <= 27; ++x)
        m.at<uchar>(y, x) = 255;
    for (int y = 8; y <= 27; ++y) for (int x = 14; x <= 17; ++x)
        m.at<uchar>(y, x) = 255;
    // 3) 4 corner 3x3 alignment dots (like QR code)
    auto fillSquare = [&](int ox, int oy) {
        for (int dy = 0; dy < 3; ++dy) for (int dx = 0; dx < 3; ++dx) {
            int yy = oy + dy, xx = ox + dx;
            if (yy >= 0 && yy < 32 && xx >= 0 && xx < 32) {
                bool onEdge = (dx == 0 || dx == 2 || dy == 0 || dy == 2);
                bool center = (dx == 1 && dy == 1);
                if (onEdge || center) m.at<uchar>(yy, xx) = 255;
            }
        }
    };
    fillSquare(3, 3);
    fillSquare(26, 3);
    fillSquare(3, 26);
    fillSquare(26, 26);
    // 4) scatter pseudo-random dots in the remaining area (~20% fill) to increase capacity & randomness
    for (int y = 2; y < 30; ++y) for (int x = 2; x < 30; ++x) {
        if (m.at<uchar>(y, x) == 0 && rng.uniform(0, 100) < 22) {
            m.at<uchar>(y, x) = 255;
        }
    }
    return m;
}

// ===========================================================================
// Robustness attack set (each attack returns a cv::Mat, does not modify the input)
// ===========================================================================
struct AttackResult {
    std::string name;
    cv::Mat attacked;
    double psnrToMarked;   // fidelity vs the watermarked source image
};

static AttackResult makeAttack(const std::string& name, const cv::Mat& marked,
                               const std::function<cv::Mat(const cv::Mat&)>& fn) {
    cv::Mat out = fn(marked);
    double p = psnr(marked, out);
    AttackResult r{name, out, p};
    return r;
}

static std::vector<AttackResult> buildAttackSuite(const cv::Mat& marked) {
    std::vector<AttackResult> res;

    // 1) JPEG compression
    auto jpegFn = [](int q) {
        return [q](const cv::Mat& in) -> cv::Mat {
            std::vector<uchar> buf;
            std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, q};
            cv::imencode(".jpg", in, buf, params);
            return cv::imdecode(buf, cv::IMREAD_COLOR);
        };
    };
    res.push_back(makeAttack("JPEG-Q90", marked, jpegFn(90)));
    res.push_back(makeAttack("JPEG-Q70", marked, jpegFn(70)));
    res.push_back(makeAttack("JPEG-Q50", marked, jpegFn(50)));
    res.push_back(makeAttack("JPEG-Q30", marked, jpegFn(30)));

    // 2) Gaussian noise
    auto gaussFn = [](double sigma) {
        return [sigma](const cv::Mat& in) -> cv::Mat {
            cv::Mat out = in.clone();
            cv::Mat noise(in.size(), in.type());
            cv::randn(noise, 0, sigma);
            cv::add(out, noise, out, cv::noArray(), in.type());
            return out;
        };
    };
    res.push_back(makeAttack("Gauss-σ5", marked, gaussFn(5)));
    res.push_back(makeAttack("Gauss-σ10", marked, gaussFn(10)));
    res.push_back(makeAttack("Gauss-σ20", marked, gaussFn(20)));

    // 3) salt-and-pepper noise
    auto spFn = [](double density) {
        return [density](const cv::Mat& in) -> cv::Mat {
            cv::Mat out = in.clone();
            cv::Mat noise(out.size(), CV_32F);
            cv::randu(noise, 0.0f, 1.0f);
            for (int y = 0; y < out.rows; ++y) {
                for (int x = 0; x < out.cols; ++x) {
                    float v = noise.at<float>(y, x);
                    if (v < density / 2) {
                        if (out.channels() == 3) out.at<cv::Vec3b>(y, x) = cv::Vec3b(0,0,0);
                        else out.at<uchar>(y, x) = 0;
                    } else if (v < density) {
                        if (out.channels() == 3) out.at<cv::Vec3b>(y, x) = cv::Vec3b(255,255,255);
                        else out.at<uchar>(y, x) = 255;
                    }
                }
            }
            return out;
        };
    };
    res.push_back(makeAttack("SP-0.5%", marked, spFn(0.005)));
    res.push_back(makeAttack("SP-1.0%", marked, spFn(0.010)));
    res.push_back(makeAttack("SP-3.0%", marked, spFn(0.030)));

    // 4) median filter
    res.push_back(makeAttack("Median-3x3", marked, [](const cv::Mat& in) {
        cv::Mat out; cv::medianBlur(in, out, 3); return out;
    }));
    res.push_back(makeAttack("Median-5x5", marked, [](const cv::Mat& in) {
        cv::Mat out; cv::medianBlur(in, out, 5); return out;
    }));
    res.push_back(makeAttack("Median-7x7", marked, [](const cv::Mat& in) {
        cv::Mat out; cv::medianBlur(in, out, 7); return out;
    }));

    // 5) mean filter
    res.push_back(makeAttack("Blur-3x3", marked, [](const cv::Mat& in) {
        cv::Mat out; cv::blur(in, out, cv::Size(3, 3)); return out;
    }));
    res.push_back(makeAttack("Blur-5x5", marked, [](const cv::Mat& in) {
        cv::Mat out; cv::blur(in, out, cv::Size(5, 5)); return out;
    }));

    // 6) rotation (crop + resize back to original size)
    auto rotateFn = [](double deg) {
        return [deg](const cv::Mat& in) -> cv::Mat {
            cv::Point2f center((float)in.cols / 2, (float)in.rows / 2);
            cv::Mat M = cv::getRotationMatrix2D(center, deg, 1.0);
            cv::Mat rot;
            cv::warpAffine(in, rot, M, in.size(), cv::INTER_LINEAR,
                           cv::BORDER_REPLICATE);
            return rot;
        };
    };
    res.push_back(makeAttack("Rotate-+5°", marked, rotateFn(+5)));
    res.push_back(makeAttack("Rotate--5°", marked, rotateFn(-5)));
    res.push_back(makeAttack("Rotate-+15°", marked, rotateFn(+15)));
    res.push_back(makeAttack("Rotate--15°", marked, rotateFn(-15)));

    // 7) scaling round-trip
    auto scaleFn = [](double s1, double s2) {
        return [s1, s2](const cv::Mat& in) -> cv::Mat {
            cv::Mat tmp, out;
            cv::resize(in, tmp, cv::Size(), s1, s1, cv::INTER_LINEAR);
            cv::resize(tmp, out, in.size(), 0, 0, cv::INTER_LINEAR);
            return out;
        };
    };
    res.push_back(makeAttack("Scale-0.8→1.25", marked, scaleFn(0.8, 1.0 / 0.8)));
    res.push_back(makeAttack("Scale-0.5→2.0", marked, scaleFn(0.5, 1.0 / 0.5)));

    // 8) cropping + black border filled back to original size
    auto cropFn = [](double marginRatio) {
        return [marginRatio](const cv::Mat& in) -> cv::Mat {
            int mx = (int)(in.cols * marginRatio);
            int my = (int)(in.rows * marginRatio);
            int x = mx, y = my;
            int w = in.cols - 2 * mx, h = in.rows - 2 * my;
            if (w <= 0 || h <= 0) return in.clone();
            cv::Mat roi = in(cv::Rect(x, y, w, h));
            cv::Mat out(in.size(), in.type(), cv::Scalar(0, 0, 0));
            cv::Mat dstRoi = out(cv::Rect(x, y, w, h));
            roi.copyTo(dstRoi);
            return out;
        };
    };
    res.push_back(makeAttack("Crop-5%", marked, cropFn(0.05)));
    res.push_back(makeAttack("Crop-10%", marked, cropFn(0.10)));

    // 9) brightness adjustment
    auto brightnessFn = [](int delta) {
        return [delta](const cv::Mat& in) -> cv::Mat {
            cv::Mat out;
            in.convertTo(out, -1, 1.0, delta);
            return out;
        };
    };
    res.push_back(makeAttack("Brightness-20", marked, brightnessFn(+20)));
    res.push_back(makeAttack("Brightness--20", marked, brightnessFn(-20)));
    res.push_back(makeAttack("Brightness-50", marked, brightnessFn(+50)));
    res.push_back(makeAttack("Brightness--50", marked, brightnessFn(-50)));

    // 10) contrast adjustment
    auto contrastFn = [](double f) {
        return [f](const cv::Mat& in) -> cv::Mat {
            cv::Mat out;
            in.convertTo(out, -1, f, 128 * (1 - f));
            return out;
        };
    };
    res.push_back(makeAttack("Contrast-0.8", marked, contrastFn(0.8)));
    res.push_back(makeAttack("Contrast-1.2", marked, contrastFn(1.2)));

    // 11) histogram equalization (on Y channel)
    res.push_back(makeAttack("HEQ", marked, [](const cv::Mat& in) -> cv::Mat {
        if (in.channels() == 3) {
            cv::Mat ycrcb; cv::cvtColor(in, ycrcb, cv::COLOR_BGR2YCrCb);
            std::vector<cv::Mat> chs(3); cv::split(ycrcb, chs);
            cv::equalizeHist(chs[0], chs[0]);
            cv::Mat merged; cv::merge(chs, merged);
            cv::Mat bgr; cv::cvtColor(merged, bgr, cv::COLOR_YCrCb2BGR);
            return bgr;
        } else {
            cv::Mat out; cv::equalizeHist(in, out); return out;
        }
    }));

    // 12) combined attack: JPEG Q50 + Gaussian σ10 + 3x3 median
    res.push_back(makeAttack("Combo(JPEG50+σ10+Med3)", marked, [](const cv::Mat& in) {
        std::vector<uchar> buf;
        cv::imencode(".jpg", in, buf, {cv::IMWRITE_JPEG_QUALITY, 50});
        cv::Mat a = cv::imdecode(buf, cv::IMREAD_COLOR);
        cv::Mat noise(a.size(), a.type());
        cv::randn(noise, 0, 10);
        cv::Mat b; cv::add(a, noise, b, cv::noArray(), a.type());
        cv::Mat c; cv::medianBlur(b, c, 3);
        return c;
    }));

    return res;
}

// ===========================================================================
// main
// ===========================================================================
int main(int argc, char** argv) {
    std::string inPath = "../../data/images/lena.jpg";
    if (argc > 1) inPath = argv[1];

    cv::Mat src = cv::imread(inPath, cv::IMREAD_COLOR);
    if (src.empty()) src = readNv21Auto(inPath);
    if (src.empty()) {
        log("watermark", "fallback synthetic 512x512");
        src = cv::Mat::zeros(512, 512, CV_8UC3);
        cv::randu(src, 0, 256);
    }
    if (std::max(src.rows, src.cols) > 768) {
        double s = 768.0 / std::max(src.rows, src.cols);
        cv::resize(src, src, cv::Size(), s, s, cv::INTER_AREA);
    }
    ensureDir("../out/algorithms");

    // ========== 1. Visible watermark ==========
    cv::Mat vText   = visibleTextWatermark(src, "TRAE 2026", 0.35);
    cv::Mat logo    = cv::imread("../../data/images/opencv-logo.png", cv::IMREAD_UNCHANGED);
    cv::Mat vLogo   = visibleLogoWatermark(src, logo, 0.20);
    cv::Mat vTex    = visibleTextureWatermark(src, 0.10, 28, 35);
    cv::Mat vTex2   = visibleTextureWatermark(src, 0.08, 20, -25);
    cv::Mat vDual   = visibleTextWatermark(vTex, "Confidential", 0.28);

    cv::imwrite("../out/algorithms/watermark_visible_text.png",  vText);
    cv::imwrite("../out/algorithms/watermark_visible_logo.png",  vLogo);
    cv::imwrite("../out/algorithms/watermark_visible_texture.png", vTex);
    cv::imwrite("../out/algorithms/watermark_visible_dual.png", vDual);

    // ========== 2. Watermark pattern ==========
    cv::Mat mark = buildMark("TRAE-WATERMARK-2026");

    // ========== 3. DFT-domain watermark (non-blind) ==========
    cv::Mat dftMarked = dftWatermarkEmbed(src, mark, 2.0);
    cv::Mat dftExNoAtt = dftWatermarkExtract(dftMarked, src);
    cv::imwrite("../out/algorithms/watermark_dft_marked.png", dftMarked);
    cv::imwrite("../out/algorithms/watermark_dft_extracted_noatt.png", dftExNoAtt);

    // ========== 4. DCT-domain watermark (blind extraction, alpha sweep comparison) ==========
    std::vector<double> alphas = {3.0, 6.0, 9.0, 12.0};
    std::vector<cv::Mat> dctMarkedVec, dctExVec;
    std::vector<double> dctPsnrVec, dctSsimVec, dctRecovVec;
    for (size_t i = 0; i < alphas.size(); ++i) {
        DctWatermarkCtx ctx;
        ctx.alpha = alphas[i];
        cv::Mat m = dctWatermarkEmbed(src, mark, ctx);
        cv::Mat ex = dctWatermarkExtract(m, ctx);
        dctMarkedVec.push_back(m);
        dctExVec.push_back(ex);
        dctPsnrVec.push_back(psnr(src, m));
        dctSsimVec.push_back(ssim(src, m));
        dctRecovVec.push_back(bitRecoveryRate(mark, ex));
        std::ostringstream oss;
        oss << "../out/algorithms/watermark_dct_a" << std::fixed << std::setprecision(0)
            << alphas[i] << "_marked.png";
        cv::imwrite(oss.str(), m);
    }

    // ========== 5. Robustness attack test (on DCT alpha=6) ==========
    DctWatermarkCtx ctxRob;
    ctxRob.alpha = 6.0;
    cv::Mat dctMarkedRob = dctWatermarkEmbed(src, mark, ctxRob);
    auto attacks = buildAttackSuite(dctMarkedRob);

    struct RobustRow {
        std::string name;
        double psnrAtt;     // after attack vs watermarked source
        double recovDct;    // DCT blind extraction recovery rate
        double recovDft;    // DFT non-blind extraction recovery rate
    };
    std::vector<RobustRow> robustTable;

    // record the no-attack baseline first
    {
        cv::Mat eDct = dctWatermarkExtract(dctMarkedRob, ctxRob);
        cv::Mat eDft = dftWatermarkExtract(dftMarked, src);
        robustTable.push_back({"Baseline(noAttack)",
            psnr(src, dctMarkedRob),
            bitRecoveryRate(mark, eDct),
            bitRecoveryRate(mark, eDft)
        });
    }

    // save a few attacked images for display (pick 6 representative attacks)
    std::vector<std::string> saveAttacks = {"JPEG-Q50", "Gauss-σ10", "Median-5x5",
                                             "Rotate-+15°", "Crop-10%", "Combo(JPEG50+σ10+Med3)"};
    std::vector<cv::Mat> attackedShowImgs;
    std::vector<std::string> attackedShowLabels;

    for (auto& att : attacks) {
        cv::Mat eDct = dctWatermarkExtract(att.attacked, ctxRob);
        cv::Mat eDft = dftWatermarkExtract(att.attacked, src);
        robustTable.push_back({att.name, att.psnrToMarked,
                               bitRecoveryRate(mark, eDct),
                               bitRecoveryRate(mark, eDft)});
        if (std::find(saveAttacks.begin(), saveAttacks.end(), att.name) != saveAttacks.end()) {
            attackedShowImgs.push_back(att.attacked);
            std::ostringstream oss;
            oss << att.name << "  DCT:" << std::fixed << std::setprecision(0)
                << bitRecoveryRate(mark, eDct) << "%";
            attackedShowLabels.push_back(oss.str());
            cv::imwrite("../out/algorithms/watermark_att_" + att.name + ".png", att.attacked);
        }
    }

    // ========== 6. Console output ==========
    std::printf("\n========== VISIBLE WATERMARKS ==========\n");
    std::printf("text overlay PSNR/SSIM : %.2f dB / %.4f\n", psnr(src, vText), ssim(src, vText));
    std::printf("logo overlay PSNR/SSIM : %.2f dB / %.4f\n", psnr(src, vLogo), ssim(src, vLogo));
    std::printf("texture   PSNR/SSIM    : %.2f dB / %.4f\n", psnr(src, vTex),  ssim(src, vTex));

    std::printf("\n========== INVISIBLE: DFT (non-blind, alpha=2.0) ==========\n");
    std::printf("PSNR(src, marked) = %.2f dB\n", psnr(src, dftMarked));
    std::printf("SSIM(src, marked) = %.4f\n", ssim(src, dftMarked));
    std::printf("MAE = %.2f,  MSE = %.2f\n", mae(src, dftMarked), mse(src, dftMarked));
    std::printf("mark recovery (no attack) = %.1f%%\n", bitRecoveryRate(mark, dftExNoAtt));

    std::printf("\n========== INVISIBLE: DCT (blind, QIM, alpha sweep) ==========\n");
    std::printf("%-10s  %10s  %10s  %14s\n", "alpha", "PSNR(dB)", "SSIM", "Recovery(%)");
    for (size_t i = 0; i < alphas.size(); ++i) {
        std::printf("%-10.1f  %10.2f  %10.4f  %14.1f\n",
            alphas[i], dctPsnrVec[i], dctSsimVec[i], dctRecovVec[i]);
    }
    std::printf("   note: larger alpha => more robust watermark, but lower PSNR/SSIM and more visible distortion\n");

    std::printf("\n========== ROBUSTNESS TEST (DCT alpha=6.0 vs DFT alpha=2.0) ==========\n");
    std::printf("%-34s  %10s  %12s  %12s\n",
                "Attack", "PSNR_att", "DCT_blind%", "DFT_nonBlind%");
    // sort by DCT blind recovery rate (high to low)
    std::sort(robustTable.begin() + 1, robustTable.end(),
              [](const RobustRow& a, const RobustRow& b){ return a.recovDct > b.recovDct; });
    for (auto& r : robustTable) {
        std::printf("%-34s  %10.2f  %12.1f  %12.1f\n",
                    r.name.c_str(), r.psnrAtt, r.recovDct, r.recovDft);
    }

    // ========== 7. Stitched display images ==========
    // 7a. visible watermark comparison
    cv::Mat markBig, extDftBig, extDct6Big;
    cv::resize(mark, markBig, cv::Size(160, 160), 0, 0, cv::INTER_NEAREST);
    cv::resize(dftExNoAtt, extDftBig, cv::Size(160, 160), 0, 0, cv::INTER_NEAREST);
    cv::resize(dctExVec[1], extDct6Big, cv::Size(160, 160), 0, 0, cv::INTER_NEAREST); // alpha=6

    cv::Mat canvas1 = hstackWithLabels(
        {src, vText, vLogo, vTex, vDual},
        {"input", "visible text", "visible logo", "texture diag", "text+texture"},
        24);
    cv::imwrite("../out/algorithms/watermark_visible_compare.png", canvas1);

    // 7b. DFT vs DCT watermark comparison (incl. source, watermarked, magnified diff, extracted watermark)
    cv::Mat dftDiff, dct6Diff;
    cv::absdiff(src, dftMarked, dftDiff);
    cv::absdiff(src, dctMarkedVec[1], dct6Diff);
    // magnify the diff 8x for visibility
    dftDiff.convertTo(dftDiff, -1, 8.0, 0);
    dct6Diff.convertTo(dct6Diff, -1, 8.0, 0);
    cv::Mat canvas2 = hstackWithLabels(
        {src, dftMarked, dftDiff, markBig, extDftBig,
         dctMarkedVec[1], dct6Diff, extDct6Big},
        {"input", "DFT_marked", "DFT_diff(x8)", "mark_32x32", "DFT_extract",
         "DCT_a6_marked", "DCT_a6_diff(x8)", "DCT_a6_extract"},
        22);
    cv::imwrite("../out/algorithms/watermark_invisible_compare.png", canvas2);

    // 7c. representative robustness-attacked images
    if (!attackedShowImgs.empty()) {
        attackedShowImgs.insert(attackedShowImgs.begin(), dctMarkedRob);
        attackedShowLabels.insert(attackedShowLabels.begin(), "DCT_a6_original");
        cv::Mat canvas3 = hstackWithLabels(attackedShowImgs, attackedShowLabels, 20);
        cv::imwrite("../out/algorithms/watermark_robustness_attacks.png", canvas3);
    }

    // 7d. DCT alpha sweep comparison
    std::vector<cv::Mat> alphaCanvImgs;
    std::vector<std::string> alphaCanvLabels;
    for (size_t i = 0; i < alphas.size(); ++i) {
        alphaCanvImgs.push_back(dctMarkedVec[i]);
        cv::Mat diff; cv::absdiff(src, dctMarkedVec[i], diff);
        diff.convertTo(diff, -1, 8.0, 0);
        alphaCanvImgs.push_back(diff);
        cv::Mat exBig; cv::resize(dctExVec[i], exBig, cv::Size(128, 128), 0, 0, cv::INTER_NEAREST);
        alphaCanvImgs.push_back(exBig);
        std::ostringstream o1, o2, o3;
        o1 << "DCT a=" << alphas[i];
        o2 << "diff x8";
        o3 << "Rec:" << std::fixed << std::setprecision(1) << dctRecovVec[i] << "%";
        alphaCanvLabels.push_back(o1.str());
        alphaCanvLabels.push_back(o2.str());
        alphaCanvLabels.push_back(o3.str());
    }
    cv::Mat canvas4 = hstackWithLabels(alphaCanvImgs, alphaCanvLabels, 18);
    cv::imwrite("../out/algorithms/watermark_dct_alpha_sweep.png", canvas4);

    log("watermark", "wrote results under ../out/algorithms/");
    std::printf("\nDone. All output files saved.\n");

    imshowFit("watermark_visible", canvas1, 1600, 0);
    imshowFit("watermark_invisible", canvas2, 1700, 0);
    return 0;
}
