// algorithms/common/nv21_io.hpp
// NV21 / NV12 / I420 (YUV420) raw reader & writer, 配套文件名元数据解析.
//
// 本工具统一了 Android 相机流水线常见的 3 种 YUV420 格式:
//
//   NV21 : YYYY... V U V U V U V U ...  (Y + VU 交错平面, Android 主相机默认)
//   NV12 : YYYY... U V U V U V U V ...  (Y + UV 交错平面, 高通/编码常用)
//   I420 : YYYY... U U ... V V ...      (3 个独立平面, Y+U+V, AOSP 编码中间格式)
//
// 每类格式的紧凑布局 (stride_y == width) 所需字节数均为 w*h*3/2,
// 但平面结构不同, 转换到 BGR 的 cvtColor code 也不同.
//
// 工程数据中常见 stride padding (例如文件名标注 3264x2448, 但实际行步长更大),
// 本工具同时支持:
//   1) 紧排布 (默认): 仅需 width/height
//   2) 显式 stride: 调用方传入 stride_y
// 当原始文件字节数略大于紧凑理论字节数时, 多余尾部字节将被忽略
// (往往是相机驱动追加的 metadata, 非像素数据)。
#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace algo {

// YUV420 格式类型枚举。
enum class YuvFormat {
    NV21 = 0, // Y + VU 交错 (默认)
    NV12 = 1, // Y + UV 交错
    I420 = 2, // Y + U + V (IYUV, 平面式)
};

// YUV420 原始帧描述: 格式 + 尺寸 + stride + 路径 + 可选元数据。
struct YuvFrame {
    YuvFormat fmt = YuvFormat::NV21;
    int width = 0;            // 有效图像宽度 (偶数)
    int height = 0;           // 有效图像高度 (偶数)
    int stride_y = 0;         // Y 平面行步长 (>= width), 0 表示等于 width
    int stride_uv = 0;        // UV (或 U/V) 行步长, 0 表示等于 stride_y
    std::string path;         // 原始文件路径
    size_t file_size = 0;     // 文件实际字节数
    // 可选: 文件名里解析到的拍摄参数 (未解析到为 0 / 默认)
    int iso = 0;              // ISO 灵敏度
    double exposure_time_us = 0; // 曝光时间 (us)
    int ev_value = 0;         // EV 值 (整数档, -8/-4/0/+2 ...)
    int base_id = 0;          // "base_0 / base_1" 基准帧 ID
    double analog_gain = 0;   // 从文件名 ag_XXX 中解析 (0 = 未知)
    double digital_gain = 0;  // dg_XXX
};

// 兼容老名称。
using Nv21Frame = YuvFrame;

// 从文件名解析宽高, 匹配 "_WWWWxHHHH_" / "_WWWWXHHHH_" (大小写不敏感)。
// 宽高必须都为偶数, 成功时返回 true。
bool parseNv21SizeFromName(const std::string& filename, int& w, int& h);

// 从文件名解析 ISO: 匹配 "iso_XXX". 失败返回 0。
int parseIsoFromName(const std::string& filename);
// 从文件名解析曝光时间 us: 匹配 "et_XXXXXX" (数字串)。失败返回 0。
double parseExposureTimeFromName(const std::string& filename);
// 从文件名解析 EV 值: 匹配 "ev_+/-N"。失败返回 0。
int parseEvValueFromName(const std::string& filename);
// 从文件名解析 base_0 / base_1。失败返回 -1。
int parseBaseIdFromName(const std::string& filename);
// 从文件名解析 analog_gain (ag_XX) / digital_gain (dg_XX)。失败返回 0.0。
double parseAnalogGainFromName(const std::string& filename);
double parseDigitalGainFromName(const std::string& filename);
// 统一解析: 填充 frame.iso / exposure_time_us / ev_value / base_id / gain。
void parseAllMetaFromName(YuvFrame& frame);

// 检测扩展名 / 内容并决定 YuvFormat:
//   .NV21 或含 vu 字样 -> NV21
//   .NV12 / .uv     -> NV12
//   .I420 / .IYUV / .YUV + YUV420 planar 推断 -> I420
YuvFormat guessFormatFromName(const std::string& filename);

// ---------------------------------------------------------------
// 读: YUV raw → BGR / Y / (RGB)
// ---------------------------------------------------------------

// 读取一帧 YUV420 raw, 输出 BGR Mat。
//   若 stride_y > width 则按 stride 模式逐行紧凑拷贝;
//   多余尾部字节自动忽略。返回非空 Mat 表示成功。
cv::Mat readYuv420(const YuvFrame& frame);

// 便捷: 已知 width/height + path + 可选格式/stride 一次性读出 BGR。
cv::Mat readYuv420(const std::string& path, int width, int height,
                   YuvFormat fmt = YuvFormat::NV21,
                   int stride_y = 0, int stride_uv = 0);

// 兼容老 API (NV21 默认)。
inline cv::Mat readNv21(const YuvFrame& frame) { return readYuv420(frame); }
inline cv::Mat readNv21(const std::string& path, int w, int h, int stride_y = 0) {
    return readYuv420(path, w, h, YuvFormat::NV21, stride_y, 0);
}

// 自动检测 (用文件名解析宽高与格式)。
cv::Mat readNv21Auto(const std::string& path);

// 把 BGR Mat 写成 NV21 / NV12 / I420 raw (紧排布)。
bool writeNv21(const std::string& path, const cv::Mat& bgr);
bool writeYuv420(const std::string& path, const cv::Mat& bgr, YuvFormat fmt);

// 批量加载某目录下所有 YUV420 (依据文件名解析宽高)。
//   返回 (BGR, meta) 列表, 加载失败的文件会被跳过并在日志中提示。
struct LoadedFrame {
    cv::Mat bgr;
    YuvFrame meta;
};
std::vector<LoadedFrame> loadNv21Dir(const std::string& dir,
                                     bool sortByName = true);
// 同上, 但支持所有 YuvFormat。
std::vector<LoadedFrame> loadYuv420Dir(const std::string& dir,
                                        bool sortByName = true);

// ---------------------------------------------------------------
// 直接 Y / UV 平面提取
// ---------------------------------------------------------------

// 从 YUV420 直接提取 Y 平面 (单通道 CV_8UC1), 比 BGR 转换快很多。
cv::Mat readYuv420Y(const YuvFrame& frame);
cv::Mat readYuv420Y(const std::string& path, int width, int height,
                    YuvFormat fmt = YuvFormat::NV21,
                    int stride_y = 0);

// 兼容老 API。
inline cv::Mat readNv21Y(const YuvFrame& frame) { return readYuv420Y(frame); }
inline cv::Mat readNv21Y(const std::string& path, int w, int h, int stride_y = 0) {
    return readYuv420Y(path, w, h, YuvFormat::NV21, stride_y);
}

// 从 YUV420 同时提取 Y + UV (或 U+V) 平面.
//   - NV12/NV21: UV 输出为 CV_8UC2, size = (w/2, h/2)
//   - I420: U 与 V 输出为 CV_8UC1, size = (w/2, h/2)
struct YuvPlanes {
    cv::Mat Y;                  // (w,h) CV_8UC1
    cv::Mat UV_or_U;            // NV: (w/2, h/2) CV_8UC2; I420: U, (w/2,h/2) CV_8UC1
    cv::Mat V;                  // 仅 I420 非空, (w/2,h/2) CV_8UC1
};
YuvPlanes readYuv420Planes(const YuvFrame& frame);

// ---------------------------------------------------------------
// 工具函数
// ---------------------------------------------------------------

// 紧凑布局下, 某 format 的理论字节数。
size_t yuv420ByteCount(int w, int h, YuvFormat fmt = YuvFormat::NV21);
inline size_t nv21ByteCount(int w, int h) { return (size_t)w * (size_t)h * 3 / 2; }

// 从格式名枚举转字符串, 用于日志打印。
const char* formatName(YuvFormat fmt);

} // namespace algo
