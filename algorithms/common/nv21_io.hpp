// algorithms/common/nv21_io.hpp
// NV21 / NV12 / I420 (YUV420) raw reader & writer, with filename metadata parsing.
//
// This utility unifies the 3 common YUV420 formats in Android camera pipelines:
//
//   NV21 : YYYY... V U V U V U V U ...  (Y + interleaved VU plane, Android main camera default)
//   NV12 : YYYY... U V U V U V U V ...  (Y + interleaved UV plane, common in Qualcomm/encoding)
//   I420 : YYYY... U U ... V V ...      (3 separate planes, Y+U+V, AOSP encoding intermediate format)
//
// For a compact layout (stride_y == width) every format needs w*h*3/2 bytes,
// but the plane structures differ, so the cvtColor code to BGR differs too.
//
// Real engineering data often has stride padding (e.g. filename says 3264x2448 but the actual row stride is larger),
// and this utility supports both:
//   1) compact layout (default): only width/height needed
//   2) explicit stride: caller passes stride_y
// When the raw file has a few extra bytes beyond the compact theoretical size, the extra tail is ignored
// (usually camera-driver metadata, not pixel data).
#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace algo {

// YUV420 format type enumeration.
enum class YuvFormat {
    NV21 = 0, // Y + interleaved VU (default)
    NV12 = 1, // Y + interleaved UV
    I420 = 2, // Y + U + V (IYUV, planar)
};

// YUV420 raw frame descriptor: format + size + stride + path + optional metadata.
struct YuvFrame {
    YuvFormat fmt = YuvFormat::NV21;
    int width = 0;            // active image width (even)
    int height = 0;           // active image height (even)
    int stride_y = 0;         // Y plane row stride (>= width), 0 means equals width
    int stride_uv = 0;        // UV (or U/V) row stride, 0 means equals stride_y
    std::string path;         // raw file path
    size_t file_size = 0;     // actual file byte count
    // optional: capture parameters parsed from the filename (0/default if not found)
    int iso = 0;              // ISO sensitivity
    double exposure_time_us = 0; // exposure time (us)
    int ev_value = 0;         // EV value (integer stops, -8/-4/0/+2 ...)
    int base_id = 0;          // "base_0 / base_1" reference frame ID
    double analog_gain = 0;   // parsed from filename ag_XXX (0 = unknown)
    double digital_gain = 0;  // dg_XXX
};

// compatibility alias for the old name.
using Nv21Frame = YuvFrame;

// Parse width/height from the filename, matching "_WWWWxHHHH_" / "_WWWWXHHHH_" (case-insensitive).
// Both width and height must be even; returns true on success.
bool parseNv21SizeFromName(const std::string& filename, int& w, int& h);

// Parse ISO from the filename: matches "iso_XXX". Returns 0 on failure.
int parseIsoFromName(const std::string& filename);
// Parse exposure time (us) from the filename: matches "et_XXXXXX" (digit string). Returns 0 on failure.
double parseExposureTimeFromName(const std::string& filename);
// Parse EV value from the filename: matches "ev_+/-N". Returns 0 on failure.
int parseEvValueFromName(const std::string& filename);
// Parse base_0 / base_1 from the filename. Returns -1 on failure.
int parseBaseIdFromName(const std::string& filename);
// Parse analog_gain (ag_XX) / digital_gain (dg_XX) from the filename. Returns 0.0 on failure.
double parseAnalogGainFromName(const std::string& filename);
double parseDigitalGainFromName(const std::string& filename);
// Unified parsing: fill frame.iso / exposure_time_us / ev_value / base_id / gain.
void parseAllMetaFromName(YuvFrame& frame);

// Detect extension / content and decide the YuvFormat:
//   .NV21 or contains "vu" -> NV21
//   .NV12 / .uv     -> NV12
//   .I420 / .IYUV / .YUV with YUV420 planar inference -> I420
YuvFormat guessFormatFromName(const std::string& filename);

// ---------------------------------------------------------------
// read: YUV raw -> BGR / Y / (RGB)
// ---------------------------------------------------------------

// Read one YUV420 raw frame, output a BGR Mat.
//   If stride_y > width, copy row-by-row in stride mode;
//   extra tail bytes are ignored automatically. A non-empty Mat means success.
cv::Mat readYuv420(const YuvFrame& frame);

// Convenience: read BGR in one call with known width/height + path + optional format/stride.
cv::Mat readYuv420(const std::string& path, int width, int height,
                   YuvFormat fmt = YuvFormat::NV21,
                   int stride_y = 0, int stride_uv = 0);

// compatibility wrapper for the old API (NV21 default).
inline cv::Mat readNv21(const YuvFrame& frame) { return readYuv420(frame); }
inline cv::Mat readNv21(const std::string& path, int w, int h, int stride_y = 0) {
    return readYuv420(path, w, h, YuvFormat::NV21, stride_y, 0);
}

// Auto-detect (parse width/height and format from the filename).
cv::Mat readNv21Auto(const std::string& path);

// Write a BGR Mat to NV21 / NV12 / I420 raw (compact layout).
bool writeNv21(const std::string& path, const cv::Mat& bgr);
bool writeYuv420(const std::string& path, const cv::Mat& bgr, YuvFormat fmt);

// Batch-load all YUV420 files in a directory (parse width/height from filenames).
//   Returns a (BGR, meta) list; files that fail to load are skipped and logged.
struct LoadedFrame {
    cv::Mat bgr;
    YuvFrame meta;
};
std::vector<LoadedFrame> loadNv21Dir(const std::string& dir,
                                     bool sortByName = true);
// same as above, but supports all YuvFormat.
std::vector<LoadedFrame> loadYuv420Dir(const std::string& dir,
                                        bool sortByName = true);

// ---------------------------------------------------------------
// direct Y / UV plane extraction
// ---------------------------------------------------------------

// Extract the Y plane directly from YUV420 (single-channel CV_8UC1), much faster than BGR conversion.
cv::Mat readYuv420Y(const YuvFrame& frame);
cv::Mat readYuv420Y(const std::string& path, int width, int height,
                    YuvFormat fmt = YuvFormat::NV21,
                    int stride_y = 0);

// compatibility wrapper for the old API.
inline cv::Mat readNv21Y(const YuvFrame& frame) { return readYuv420Y(frame); }
inline cv::Mat readNv21Y(const std::string& path, int w, int h, int stride_y = 0) {
    return readYuv420Y(path, w, h, YuvFormat::NV21, stride_y);
}

// Extract Y + UV (or U+V) planes together from YUV420.
//   - NV12/NV21: UV output is CV_8UC2, size = (w/2, h/2)
//   - I420: U and V output are CV_8UC1, size = (w/2, h/2)
struct YuvPlanes {
    cv::Mat Y;                  // (w,h) CV_8UC1
    cv::Mat UV_or_U;            // NV: (w/2, h/2) CV_8UC2; I420: U, (w/2,h/2) CV_8UC1
    cv::Mat V;                  // non-empty for I420 only, (w/2,h/2) CV_8UC1
};
YuvPlanes readYuv420Planes(const YuvFrame& frame);

// ---------------------------------------------------------------
// utility functions
// ---------------------------------------------------------------

// Theoretical byte count for a format under a compact layout.
size_t yuv420ByteCount(int w, int h, YuvFormat fmt = YuvFormat::NV21);
inline size_t nv21ByteCount(int w, int h) { return (size_t)w * (size_t)h * 3 / 2; }

// Convert the format enum to a string, for logging.
const char* formatName(YuvFormat fmt);

} // namespace algo
