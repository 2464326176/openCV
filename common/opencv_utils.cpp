//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//
#include "opencv_utils.h"

#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>
#include <cstdio>
#include <cstdarg>

// logInfo: printf-style log line, gated by DEBUG_ENABLE.
// When debug is off it compiles to nothing (no evaluation of arguments' side effects
// beyond the variadic call, which is acceptable for logging).
#if DEBUG_ENABLE
void logInfo(const char *fmt, ...) {
    // Build a timestamp prefix: HH:MM:SS.mmm
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    char ts[32];
    std::snprintf(ts, sizeof(ts), "%02d:%02d:%02d.%03lld",
                  tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
                  (long long)now_ms.count());

    // Format the user message into a buffer, then print with prefix.
    va_list args;
    va_start(args, fmt);
    int need = std::vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (need > 0) {
        std::string msg(static_cast<size_t>(need) + 1, '\0');
        va_start(args, fmt);
        std::vsnprintf(&msg[0], msg.size(), fmt, args);
        va_end(args);
        msg.pop_back(); // drop the trailing null
        std::fprintf(stdout, "[echo][%s] %s\n", ts, msg.c_str());
    } else {
        std::fprintf(stdout, "[echo][%s] \n", ts);
    }
}
#else
void logInfo(const char *fmt, ...) {
    (void)fmt;
    // No-op when debug is disabled.
}
#endif

void displayMat(std::string winName, Mat m) {
    namedWindow(winName, WINDOW_AUTOSIZE );
    imshow(winName, m);
}


// Convert a raw NV21 YUV420SP file to a BGR8UC3 Mat.
//
// NV21 layout:
//   Y  plane = width * height bytes
//   VU plane = width * (height / 2) bytes (V/U interleaved)
//   Total    = width * height * 3 / 2 bytes
//
// If the file is >= the expected size (may have trailing padding/metadata),
// take the first width*height*3/2 bytes.
void nv21_to_bgr(int width, int height, std::string filepath) {
    if (width <= 0 || height <= 0 || (width % 2) != 0 || (height % 2) != 0) {
        DBG_LOG("nv21_to_bgr: invalid size %dx%d (must be positive even)", width, height);
        return;
    }
    const size_t expected = (size_t)width * height * 3 / 2;
    std::ifstream fs(filepath, std::ios::binary);
    if (!fs) {
        DBG_LOG("nv21_to_bgr: cannot open %s", filepath.c_str());
        return;
    }
    // Obtain the actual available byte count
    fs.seekg(0, std::ios::end);
    std::streamsize fileSize = fs.tellg();
    fs.seekg(0, std::ios::beg);
    if (fileSize < (std::streamsize)expected) {
        DBG_LOG("nv21_to_bgr: %s too small: %lld bytes < %zu expected",
                filepath.c_str(), (long long)fileSize, expected);
        return;
    }
    // Read into a compact (h + h/2, w) CV_8U Mat and convert to BGR in one cvtColor call.
    cv::Mat buf(height + height / 2, width, CV_8U);
    fs.read(reinterpret_cast<char*>(buf.data), expected);
    std::streamsize got = fs.gcount();
    if (got < (std::streamsize)expected) {
        DBG_LOG("nv21_to_bgr: read short %lld bytes", (long long)got);
        return;
    }
    cv::Mat bgr;
    cv::cvtColor(buf, bgr, cv::COLOR_YUV2BGR_NV21);

    // Show the result directly (convenient for quick demo calls). Downscale to max edge 1600 if too large.
    const int maxEdge = 1600;
    cv::Mat show;
    int maxSide = std::max(bgr.cols, bgr.rows);
    if (maxSide > maxEdge) {
        double scale = (double)maxEdge / maxSide;
        cv::resize(bgr, show, cv::Size(), scale, scale, cv::INTER_AREA);
    } else {
        show = bgr;
    }
    std::string win = "nv21_to_bgr: " + filepath;
    cv::imshow(win, show);
    cv::waitKey(1);
}

static bool pathExists(const std::string& path) {
    std::ifstream f(path.c_str(), std::ios::binary);
    return f.good();
}

std::string resolveExistingPath(const std::vector<std::string>& candidates) {
    for (const auto& p : candidates) {
        if (!p.empty() && pathExists(p)) return p;
    }
    return candidates.empty() ? std::string() : candidates.front();
}

std::string getProjectRoot() {
    return resolveExistingPath({
        LEARN_PROJECT_ROOT,
        "../",
        "../../",
        "../../../",
        "../../../../",
        "."
    });
}

std::string getDataRoot() {
    return resolveExistingPath({
        LEARN_DATA_ROOT,
        getProjectRoot() + "data",
        "../data",
        "../../data",
        "../../../data",
        "../../../../data",
        "data"
    });
}

std::string getModelsRoot() {
    return resolveExistingPath({
        LEARN_MODELS_ROOT,
        getProjectRoot() + "models",
        "../models",
        "../../models",
        "../../../models",
        "../../../../models",
        "models"
    });
}

std::string getImagePath(const std::string& imageName) {
    const std::string dataRoot = getDataRoot();
    std::vector<std::string> candidates;
    if (imageName.find('/') != std::string::npos ||
        imageName.find('\\') != std::string::npos) {
        candidates = {
            dataRoot + "/" + imageName,
            std::string(IMG_PATH) + "/" + imageName,
            "../data/" + imageName,
            "../../data/" + imageName,
            "../../../data/" + imageName
        };
    } else {
        candidates = {
            dataRoot + "/images/" + imageName,
            dataRoot + "/" + imageName,
            std::string(IMG_PATH) + "/images/" + imageName,
            "../data/images/" + imageName,
            "../../data/images/" + imageName,
            "../../../data/images/" + imageName
        };
    }
    return resolveExistingPath(candidates);
}

std::string getModelPath(const std::string& modelName) {
    const std::string modelsRoot = getModelsRoot();
    return resolveExistingPath({
        modelsRoot + "/" + modelName,
        "../models/" + modelName,
        "../../models/" + modelName,
        "../../../models/" + modelName,
        getProjectRoot() + "models/" + modelName
    });
}

Mat makeSyntheticTestImage(int width, int height) {
    Mat img(height, width, CV_8UC3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            img.at<Vec3b>(y, x) = Vec3b(
                (uchar)((x * 255) / std::max(1, width - 1)),
                (uchar)((y * 255) / std::max(1, height - 1)),
                (uchar)(((x + y) * 255) / std::max(1, width + height - 2)));
        }
    }
    rectangle(img, Rect(width / 4, height / 4, width / 2, height / 2),
              Scalar(255, 255, 255), 2);
    return img;
}

// ==================== Basic debug utilities implementation ====================

static std::string typeName(int type) {
    std::string depth;
    switch (CV_MAT_DEPTH(type)) {
        case CV_8U:  depth = "CV_8U";  break;
        case CV_8S:  depth = "CV_8S";  break;
        case CV_16U: depth = "CV_16U"; break;
        case CV_16S: depth = "CV_16S"; break;
        case CV_32S: depth = "CV_32S"; break;
        case CV_32F: depth = "CV_32F"; break;
        case CV_64F: depth = "CV_64F"; break;
        default:     depth = "?";      break;
    }
    return depth + "C" + std::to_string(CV_MAT_CN(type));
}

void dbgMatInfo(const std::string& tag, const Mat& m) {
    if (m.empty()) {
        DBG_LOG("[%s] Mat is EMPTY", tag.c_str());
        return;
    }
    DBG_LOG("[%s] size=%dx%d ch=%d type=%s depth=%d continuous=%d total=%lld",
            tag.c_str(), m.cols, m.rows, m.channels(), typeName(m.type()).c_str(),
            m.depth(), m.isContinuous(), (long long)m.total());
}

void dbgPixel(const std::string& tag, const Mat& m, int x, int y) {
    if (m.empty() || x < 0 || y < 0 || x >= m.cols || y >= m.rows) {
        DBG_LOG("[%s] dbgPixel invalid coords (%d,%d) on %dx%d",
                tag.c_str(), x, y, m.cols, m.rows);
        return;
    }
    std::stringstream ss;
    ss << "[" << tag << "] (" << x << "," << y << ") = ";
    if (m.channels() == 1) {
        ss << (double)m.at<uchar>(y, x);
    } else {
        Vec3b p = m.at<Vec3b>(y, x);
        ss << "(" << (int)p[0] << "," << (int)p[1] << "," << (int)p[2] << ")";
    }
    DBG_LOG("%s", ss.str().c_str());
}

void dbgStats(const std::string& tag, const Mat& m) {
    if (m.empty()) { DBG_LOG("[%s] dbgStats: EMPTY", tag.c_str()); return; }
    Mat tmp;
    m.convertTo(tmp, CV_32F);
    double minv, maxv;
    Mat meanv, stdv;
    meanStdDev(tmp, meanv, stdv);
    minMaxLoc(tmp, &minv, &maxv);
    DBG_LOG("[%s] min=%.3f max=%.3f mean=%.3f std=%.3f",
            tag.c_str(), minv, maxv, meanv.at<double>(0, 0), stdv.at<double>(0, 0));
}

void dbgStatsROI(const std::string& tag, const Mat& m, const Rect& roi) {
    if (m.empty()) { DBG_LOG("[%s] dbgStatsROI: EMPTY", tag.c_str()); return; }
    Rect r = roi & Rect(0, 0, m.cols, m.rows);
    if (r.area() <= 0) { DBG_LOG("[%s] dbgStatsROI: invalid roi", tag.c_str()); return; }
    dbgStats(tag, m(r));
}

bool dbgCheck(const std::string& tag, const Mat& m, bool expectContinuous) {
    if (m.empty()) {
        DBG_LOG("[%s] CHECK FAILED: Mat is EMPTY", tag.c_str());
        return false;
    }
    if (m.cols <= 0 || m.rows <= 0) {
        DBG_LOG("[%s] CHECK FAILED: invalid size %dx%d", tag.c_str(), m.cols, m.rows);
        return false;
    }
    if (expectContinuous && !m.isContinuous()) {
        DBG_LOG("[%s] CHECK FAILED: not continuous", tag.c_str());
        return false;
    }
    return true;
}

bool dbgSave(const std::string& tag, const Mat& m, const std::string& dir) {
    if (m.empty()) { DBG_LOG("[%s] dbgSave: EMPTY, skip", tag.c_str()); return false; }
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ts;
    ts << std::put_time(std::localtime(&t), "%H%M%S");
    std::string path = dir + "/" + tag + "_" + ts.str() + ".png";
    bool ok = imwrite(path, m);
    DBG_LOG("[%s] dbgSave -> %s %s", tag.c_str(), path.c_str(), ok ? "OK" : "FAILED");
    return ok;
}

void dbgShow(const std::string& winName, const Mat& m, int delay) {
    if (m.empty()) { DBG_LOG("[%s] dbgShow: EMPTY", winName.c_str()); return; }
    namedWindow(winName, WINDOW_AUTOSIZE);
    imshow(winName, m);
    if (delay >= 0) waitKey(delay);
}

void dbgShowMany(const std::vector<std::string>& names,
                 const std::vector<Mat>& imgs, int delay) {
    if (names.size() != imgs.size()) {
        DBG_LOG("dbgShowMany: names(%zu) != imgs(%zu)", names.size(), imgs.size());
        return;
    }
    for (size_t i = 0; i < imgs.size(); ++i) {
        dbgShow(names[i], imgs[i], -1);
    }
    waitKey(delay);
}

double dbgTime(const std::string& label) {
    static std::map<std::string, std::chrono::high_resolution_clock::time_point> starts;
    auto now = std::chrono::high_resolution_clock::now();
    double ms = 0.0;
    auto it = starts.find(label);
    if (it == starts.end()) {
        DBG_LOG("[time:%s] start", label.c_str());
    } else {
        ms = std::chrono::duration<double, std::milli>(now - it->second).count();
        DBG_LOG("[time:%s] +%.3f ms (total since start)", label.c_str(), ms);
    }
    starts[label] = now;
    return ms;
}

void dbgPrint(const std::string& tag, int v)        { DBG_LOG("[%s] %d",   tag.c_str(), v); }
void dbgPrint(const std::string& tag, double v)     { DBG_LOG("[%s] %.4f", tag.c_str(), v); }
void dbgPrint(const std::string& tag, const std::string& v) { DBG_LOG("[%s] %s", tag.c_str(), v.c_str()); }
void dbgPrint(const std::string& tag, const Size& s) { DBG_LOG("[%s] %dx%d", tag.c_str(), s.width, s.height); }
void dbgPrint(const std::string& tag, const Rect& r) { DBG_LOG("[%s] x=%d y=%d w=%d h=%d", tag.c_str(), r.x, r.y, r.width, r.height); }
void dbgPrint(const std::string& tag, const Point& p){ DBG_LOG("[%s] (%d,%d)", tag.c_str(), p.x, p.y); }


int64_t getCurrentTimeMs() { 
    return (int64_t)(getTickCount() / getTickFrequency() * 1000);
}