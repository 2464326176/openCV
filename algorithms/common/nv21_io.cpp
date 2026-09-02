// algorithms/common/nv21_io.cpp
#include "nv21_io.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

namespace algo {

// ------------------------------------------------------------------ helpers
static std::string lowerStr(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

static std::string basenameOf(const std::string& p) {
    size_t slash = p.find_last_of("/\\");
    return (slash == std::string::npos) ? p : p.substr(slash + 1);
}

static size_t fileSize(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return 0;
    std::streamoff sz = f.tellg();
    return sz > 0 ? (size_t)sz : 0;
}

// Numeric string parsing helper: read [0-9] from `start` in s and return the value.
template <class T>
static T parseNumAfter(const std::string& s, size_t start, T def) {
    size_t end = start;
    while (end < s.size() && s[end] >= '0' && s[end] <= '9') ++end;
    if (end == start) return def;
    try { return (T)std::stod(s.substr(start, end - start)); } catch (...) { return def; }
}

// ------------------------------------------------------------------ parsers

bool parseNv21SizeFromName(const std::string& filename, int& w, int& h) {
    // match "_NNNNxNNNN_" or "_NNNNXNNNN_" (case-insensitive)
    std::string s = lowerStr(filename);
    for (size_t i = 1; i + 6 < s.size(); ++i) {
        if (s[i] == 'x' && s[i - 1] >= '0' && s[i - 1] <= '9') {
            size_t j = i - 1;
            while (j > 0 && s[j - 1] >= '0' && s[j - 1] <= '9') --j;
            size_t k = i + 1;
            while (k < s.size() && s[k] >= '0' && s[k] <= '9') ++k;
            int wv = 0, hv = 0;
            try {
                wv = std::stoi(s.substr(j, i - j));
                hv = std::stoi(s.substr(i + 1, k - i - 1));
            } catch (...) { continue; }
            if (wv > 0 && hv > 0 && wv % 2 == 0 && hv % 2 == 0) {
                w = wv; h = hv; return true;
            }
        }
    }
    return false;
}

int parseIsoFromName(const std::string& filename) {
    std::string s = lowerStr(filename);
    size_t pos = s.find("iso_");
    if (pos == std::string::npos) return 0;
    return (int)parseNumAfter<int>(s, pos + 4, 0);
}

double parseExposureTimeFromName(const std::string& filename) {
    std::string s = lowerStr(filename);
    size_t pos = s.find("et_");
    if (pos == std::string::npos) return 0;
    return parseNumAfter<double>(s, pos + 3, 0.0);
}

int parseEvValueFromName(const std::string& filename) {
    std::string s = lowerStr(filename);
    size_t pos = s.find("ev_");
    if (pos == std::string::npos) return 0;
    size_t p = pos + 3;
    int sign = 1;
    if (p < s.size()) {
        if (s[p] == '-') { sign = -1; ++p; }
        else if (s[p] == '+') { ++p; }
    }
    int v = (int)parseNumAfter<int>(s, p, 0);
    return sign * v;
}

int parseBaseIdFromName(const std::string& filename) {
    std::string s = lowerStr(filename);
    size_t pos = s.find("base_");
    if (pos == std::string::npos) return -1;
    return (int)parseNumAfter<int>(s, pos + 5, -1);
}

double parseAnalogGainFromName(const std::string& filename) {
    std::string s = lowerStr(filename);
    size_t pos = s.find("ag_");
    if (pos == std::string::npos) return 0.0;
    return parseNumAfter<double>(s, pos + 3, 0.0);
}

double parseDigitalGainFromName(const std::string& filename) {
    std::string s = lowerStr(filename);
    size_t pos = s.find("dg_");
    if (pos == std::string::npos) return 0.0;
    return parseNumAfter<double>(s, pos + 3, 0.0);
}

void parseAllMetaFromName(YuvFrame& frame) {
    std::string name = basenameOf(frame.path);
    parseNv21SizeFromName(name, frame.width, frame.height);
    frame.iso = parseIsoFromName(name);
    frame.exposure_time_us = parseExposureTimeFromName(name);
    frame.ev_value = parseEvValueFromName(name);
    frame.base_id = parseBaseIdFromName(name);
    frame.analog_gain = parseAnalogGainFromName(name);
    frame.digital_gain = parseDigitalGainFromName(name);
}

YuvFormat guessFormatFromName(const std::string& filename) {
    std::string s = lowerStr(basenameOf(filename));
    if (s.find(".nv12") != std::string::npos) return YuvFormat::NV12;
    if (s.find(".i420") != std::string::npos ||
        s.find(".iyuv") != std::string::npos ||
        s.find("_i420") != std::string::npos) {
        return YuvFormat::I420;
    }
    if (s.find(".nv21") != std::string::npos) return YuvFormat::NV21;
    // default: if no other extension and the name contains "yuv" without vu/uv keywords, use NV21
    return YuvFormat::NV21;
}

// ------------------------------------------------------------------ reads

// Copy NV12/NV21/I420 Y + UV (or U+V) data compactly into the buffer according to stride,
// total buffer length = w*h*3/2.
static bool readCompactBuffer(const YuvFrame& frame, std::vector<uint8_t>& compact) {
    size_t need = yuv420ByteCount(frame.width, frame.height, frame.fmt);
    compact.assign(need, 0);
    size_t fsize = frame.file_size ? frame.file_size : fileSize(frame.path);
    if (fsize < need) {
        std::cerr << "[nv21_io] " << frame.path
                  << " size=" << fsize << " < need=" << need
                  << " (w=" << frame.width << ",h=" << frame.height << ")\n";
        return false;
    }
    int w = frame.width, h = frame.height;
    int sy = (frame.stride_y > w) ? frame.stride_y : w;
    int suv = (frame.stride_uv > w / 2) ? frame.stride_uv : sy;
    std::ifstream f(frame.path, std::ios::binary);
    if (!f) return false;

    // --- Y plane ---
    if (sy == w) {
        f.read(reinterpret_cast<char*>(compact.data()), (std::streamsize)w * h);
    } else {
        std::vector<char> rowY(sy);
        for (int r = 0; r < h && f; ++r) {
            f.read(rowY.data(), sy);
            std::memcpy(compact.data() + (size_t)r * w, rowY.data(), w);
        }
    }
    uint8_t* uvBuf = compact.data() + (size_t)w * h;
    size_t uvSizeBytes = 0;
    if (frame.fmt == YuvFormat::NV21 || frame.fmt == YuvFormat::NV12) {
        // NV: interleaved UV plane = 2 * w/2 * h/2 bytes (one UV/VU pair per two Y samples)
        uvSizeBytes = (size_t)(w / 2) * (h / 2) * 2;
        if (suv == sy && sy == w) {
            f.read(reinterpret_cast<char*>(uvBuf), (std::streamsize)uvSizeBytes);
        } else {
            // UV row has suv * 2 bytes? No: suv is counted in "row bytes" and equals the Y row stride
            std::vector<char> rowUV(suv);
            for (int r = 0; r < h / 2 && f; ++r) {
                f.read(rowUV.data(), suv);
                std::memcpy(uvBuf + (size_t)r * w, rowUV.data(), w);
            }
        }
    } else {
        // I420: first U plane ( (w/2)*(h/2) ), then V plane (same size)
        size_t uSize = (size_t)(w / 2) * (h / 2);
        size_t vSize = uSize;
        uvSizeBytes = uSize + vSize;
        // Read U plane
        if (suv == sy && sy == w) {
            // stride equals width: U plane is compact
            f.read(reinterpret_cast<char*>(uvBuf), (std::streamsize)uSize);
            f.read(reinterpret_cast<char*>(uvBuf + uSize), (std::streamsize)vSize);
        } else {
            std::vector<char> rowUV(suv);
            for (int r = 0; r < h / 2 && f; ++r) {
                f.read(rowUV.data(), suv);
                std::memcpy(uvBuf + (size_t)r * (w / 2), rowUV.data(), w / 2);
            }
            uint8_t* vBuf = uvBuf + uSize;
            for (int r = 0; r < h / 2 && f; ++r) {
                f.read(rowUV.data(), suv);
                std::memcpy(vBuf + (size_t)r * (w / 2), rowUV.data(), w / 2);
            }
        }
    }
    (void)uvSizeBytes;
    return (bool)f;
}

cv::Mat readYuv420(const YuvFrame& frame) {
    if (frame.width <= 0 || frame.height <= 0) return cv::Mat();
    int w = frame.width, h = frame.height;
    // If compact (stride == width), read a single buffer directly and use cvtColor.
    int sy = frame.stride_y ? frame.stride_y : w;
    int suv = frame.stride_uv ? frame.stride_uv : sy;
    if (sy == w && (suv == w || suv == 0)) {
        size_t need = yuv420ByteCount(w, h, frame.fmt);
        size_t fsize = frame.file_size ? frame.file_size : fileSize(frame.path);
        if (fsize < need) {
            std::cerr << "[nv21_io] " << frame.path << " size=" << fsize
                      << " < need=" << need << "\n";
            return cv::Mat();
        }
        std::ifstream f(frame.path, std::ios::binary);
        if (!f) return cv::Mat();
        if (frame.fmt == YuvFormat::NV21) {
            cv::Mat buf(h + h / 2, w, CV_8UC1);
            f.read(reinterpret_cast<char*>(buf.data), (std::streamsize)need);
            if (!f) return cv::Mat();
            cv::Mat bgr; cv::cvtColor(buf, bgr, cv::COLOR_YUV2BGR_NV21);
            return bgr;
        } else if (frame.fmt == YuvFormat::NV12) {
            cv::Mat buf(h + h / 2, w, CV_8UC1);
            f.read(reinterpret_cast<char*>(buf.data), (std::streamsize)need);
            if (!f) return cv::Mat();
            cv::Mat bgr; cv::cvtColor(buf, bgr, cv::COLOR_YUV2BGR_NV12);
            return bgr;
        } else {
            // I420 / IYUV: planes are Y (w*h), U ((w/2)*(h/2)), V ((w/2)*(h/2))
            cv::Mat buf(h + h / 2, w, CV_8UC1);
            // Layout in buf:
            //   Y rows 0..h-1, U rows h..h+h/4-1, V rows h+h/4..h+h/2-1 (when w even)
            // OpenCV YUV2BGR_YV12 = Y then V then U. I420 = Y then U then V.
            // So I420 must be rearranged into YV12 layout to use COLOR_YUV2BGR_YV12,
            // writing the conversion ourselves here is more robust.
            uint8_t* y = buf.data;
            uint8_t* u = buf.data + (size_t)w * h;
            uint8_t* v = u + (size_t)(w / 2) * (h / 2);
            f.read(reinterpret_cast<char*>(y), (std::streamsize)w * h);
            f.read(reinterpret_cast<char*>(u), (std::streamsize)(w / 2) * (h / 2));
            f.read(reinterpret_cast<char*>(v), (std::streamsize)(w / 2) * (h / 2));
            if (!f) return cv::Mat();
            cv::Mat bgr; cv::cvtColor(buf, bgr, cv::COLOR_YUV2BGR_I420);
            return bgr;
        }
    }
    // stride padding or stride != width: fall back to compact copy
    std::vector<uint8_t> compact;
    if (!readCompactBuffer(frame, compact)) return cv::Mat();
    cv::Mat buf(h + h / 2, w, CV_8UC1, compact.data());
    cv::Mat bgr;
    switch (frame.fmt) {
        case YuvFormat::NV21: cv::cvtColor(buf, bgr, cv::COLOR_YUV2BGR_NV21); break;
        case YuvFormat::NV12: cv::cvtColor(buf, bgr, cv::COLOR_YUV2BGR_NV12); break;
        case YuvFormat::I420: cv::cvtColor(buf, bgr, cv::COLOR_YUV2BGR_I420); break;
    }
    return bgr.clone();
}

cv::Mat readYuv420(const std::string& path, int width, int height,
                   YuvFormat fmt, int stride_y, int stride_uv) {
    YuvFrame f;
    f.fmt = fmt; f.width = width; f.height = height;
    f.stride_y = stride_y; f.stride_uv = stride_uv;
    f.path = path; f.file_size = fileSize(path);
    return readYuv420(f);
}

cv::Mat readNv21Auto(const std::string& path) {
    std::string name = basenameOf(path);
    YuvFrame f; f.path = path; f.file_size = fileSize(path);
    f.fmt = guessFormatFromName(name);
    parseAllMetaFromName(f);
    if (f.width <= 0 || f.height <= 0) {
        std::cerr << "[nv21_io] cannot parse size from name: " << path << "\n";
        return cv::Mat();
    }
    return readYuv420(f);
}

// ------------------------------------------------------------------ writes

bool writeNv21(const std::string& path, const cv::Mat& bgr) {
    return writeYuv420(path, bgr, YuvFormat::NV21);
}

bool writeYuv420(const std::string& path, const cv::Mat& bgr, YuvFormat fmt) {
    if (bgr.empty() || bgr.channels() != 3) return false;
    int w = bgr.cols, h = bgr.rows;
    if (w % 2 || h % 2) return false;

    cv::Mat yuv;
    if (fmt == YuvFormat::NV21 || fmt == YuvFormat::NV12) {
        // convert to I420 first, then reassemble
        cv::cvtColor(bgr, yuv, cv::COLOR_BGR2YUV_I420);
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f) return false;
        const uint8_t* Y = yuv.data;
        const uint8_t* U = Y + (size_t)w * h;
        const uint8_t* V = U + (size_t)(w / 2) * (h / 2);
        f.write(reinterpret_cast<const char*>(Y), (std::streamsize)w * h);
        // UV/VU interleaved: each row UV has w bytes (w/2 U + w/2 V interleaved)
        for (int r = 0; r < h / 2; ++r) {
            std::vector<uint8_t> row(w);
            for (int c = 0; c < w / 2; ++c) {
                if (fmt == YuvFormat::NV21) {
                    row[2 * c + 0] = V[r * (w / 2) + c];
                    row[2 * c + 1] = U[r * (w / 2) + c];
                } else {
                    row[2 * c + 0] = U[r * (w / 2) + c];
                    row[2 * c + 1] = V[r * (w / 2) + c];
                }
            }
            f.write(reinterpret_cast<const char*>(row.data()), w);
        }
        return (bool)f;
    } else {
        // I420: write BGR->YUV_I420 directly
        cv::cvtColor(bgr, yuv, cv::COLOR_BGR2YUV_I420);
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f) return false;
        f.write(reinterpret_cast<const char*>(yuv.data),
                (std::streamsize)yuv.total());
        return (bool)f;
    }
}

// ------------------------------------------------------------------ Y plane

cv::Mat readYuv420Y(const YuvFrame& frame) {
    if (frame.width <= 0 || frame.height <= 0) return cv::Mat();
    int sy = frame.stride_y > frame.width ? frame.stride_y : frame.width;
    std::ifstream f(frame.path, std::ios::binary);
    if (!f) return cv::Mat();
    cv::Mat y(frame.height, frame.width, CV_8UC1);
    if (sy == frame.width) {
        f.read(reinterpret_cast<char*>(y.data),
               (std::streamsize)y.total());
    } else {
        std::vector<char> row(sy);
        for (int r = 0; r < frame.height && f; ++r) {
            f.read(row.data(), sy);
            std::memcpy(y.ptr(r), row.data(), frame.width);
        }
    }
    return y;
}

cv::Mat readYuv420Y(const std::string& path, int width, int height,
                    YuvFormat fmt, int stride_y) {
    YuvFrame f;
    f.fmt = fmt; f.width = width; f.height = height;
    f.stride_y = stride_y; f.path = path;
    f.file_size = fileSize(path);
    return readYuv420Y(f);
}

// ------------------------------------------------------------------ planes

YuvPlanes readYuv420Planes(const YuvFrame& frame) {
    YuvPlanes out;
    if (frame.width <= 0 || frame.height <= 0) return out;
    int w = frame.width, h = frame.height;
    out.Y = readYuv420Y(frame);
    if (out.Y.empty()) return out;
    size_t skipY = (size_t)(frame.stride_y ? frame.stride_y : w) * h;
    // read UV/VU/U+V directly
    std::ifstream f(frame.path, std::ios::binary);
    if (!f) return out;
    f.seekg((std::streamoff)skipY);
    int suv = frame.stride_uv ? frame.stride_uv
                              : (frame.stride_y ? frame.stride_y : w);
    int uvRows = h / 2;
    int uvRowW = (frame.fmt == YuvFormat::I420) ? (w / 2) : w;
    auto readRowUV = [&](std::vector<uint8_t>& storage, int bytesPerRow) {
        storage.resize(bytesPerRow);
        // actually read suv bytes
        if (suv == bytesPerRow) {
            f.read(reinterpret_cast<char*>(storage.data()), bytesPerRow);
        } else {
            std::vector<char> tmp(suv);
            f.read(tmp.data(), suv);
            std::memcpy(storage.data(), tmp.data(), bytesPerRow);
        }
    };
    if (frame.fmt == YuvFormat::NV21 || frame.fmt == YuvFormat::NV12) {
        out.UV_or_U = cv::Mat(h / 2, w / 2, CV_8UC2);
        for (int r = 0; r < uvRows; ++r) {
            std::vector<uint8_t> row; readRowUV(row, w);
            // row holds interleaved [VU ...] or [UV ...]; copy directly into the CV_8UC2 row
            std::memcpy(out.UV_or_U.ptr(r), row.data(), w);
        }
    } else {
        out.UV_or_U = cv::Mat(h / 2, w / 2, CV_8UC1);
        out.V        = cv::Mat(h / 2, w / 2, CV_8UC1);
        for (int r = 0; r < uvRows; ++r) {
            std::vector<uint8_t> row; readRowUV(row, w / 2);
            std::memcpy(out.UV_or_U.ptr(r), row.data(), w / 2);
        }
        for (int r = 0; r < uvRows; ++r) {
            std::vector<uint8_t> row; readRowUV(row, w / 2);
            std::memcpy(out.V.ptr(r), row.data(), w / 2);
        }
    }
    return out;
}

// ------------------------------------------------------------------ dirs

static void listDir(const std::string& dir, std::vector<std::string>& out) {
    out.clear();
#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    std::string pattern = dir + "\\*";
    HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        out.push_back(dir + "\\" + fd.cFileName);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR* d = opendir(dir.c_str());
    if (!d) return;
    struct dirent* e;
    while ((e = readdir(d))) {
        if (e->d_type == DT_DIR) continue;
        out.push_back(dir + "/" + e->d_name);
    }
    closedir(d);
#endif
}

std::vector<LoadedFrame> loadYuv420Dir(const std::string& dir, bool sortByName) {
    std::vector<std::string> files;
    listDir(dir, files);
    if (sortByName) std::sort(files.begin(), files.end(),
        [](const std::string& a, const std::string& b) {
            return lowerStr(basenameOf(a)) < lowerStr(basenameOf(b));
        });
    std::vector<LoadedFrame> out;
    for (const auto& p : files) {
        std::string bn = lowerStr(basenameOf(p));
        if (bn.find(".nv21") == std::string::npos &&
            bn.find(".nv12") == std::string::npos &&
            bn.find(".i420") == std::string::npos &&
            bn.find(".iyuv") == std::string::npos &&
            bn.find(".yuv")  == std::string::npos) continue;
        YuvFrame f; f.path = p; f.file_size = fileSize(p);
        f.fmt = guessFormatFromName(bn);
        parseAllMetaFromName(f);
        if (f.width <= 0 || f.height <= 0) {
            std::cerr << "[nv21_io] skip (no size in name): " << p << "\n";
            continue;
        }
        cv::Mat bgr = readYuv420(f);
        if (bgr.empty()) {
            std::cerr << "[nv21_io] skip (read fail): " << p << "\n";
            continue;
        }
        LoadedFrame lf; lf.bgr = bgr; lf.meta = f; out.push_back(std::move(lf));
    }
    return out;
}

std::vector<LoadedFrame> loadNv21Dir(const std::string& dir, bool sortByName) {
    return loadYuv420Dir(dir, sortByName);
}

// ------------------------------------------------------------------ tools

size_t yuv420ByteCount(int w, int h, YuvFormat fmt) {
    (void)fmt;
    return (size_t)w * (size_t)h * 3 / 2;
}

const char* formatName(YuvFormat fmt) {
    switch (fmt) {
        case YuvFormat::NV21: return "NV21";
        case YuvFormat::NV12: return "NV12";
        case YuvFormat::I420: return "I420";
    }
    return "Unknown";
}

} // namespace algo
