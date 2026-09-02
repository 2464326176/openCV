#ifndef OPENCV_UNITS_H
#define OPENCV_UNITS_H
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
using namespace cv;

// Default relative paths (overridden at configure time for learn targets).
#ifndef LEARN_PROJECT_ROOT
#define LEARN_PROJECT_ROOT "../"
#endif
#ifndef LEARN_DATA_ROOT
#define LEARN_DATA_ROOT "../data"
#endif
#ifndef LEARN_MODELS_ROOT
#define LEARN_MODELS_ROOT "../models"
#endif

#define IMG_PATH_DATA "../static"
#define IMG_PATH LEARN_DATA_ROOT

#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE 1
#endif

#if DEBUG_ENABLE
#define DBG_LOG(fmt, ...) \
    do { fprintf(stdout, "[DBG][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(0)
#else
#define DBG_LOG(fmt, ...) do {} while(0)
#endif

void logInfo(const char *fmt, ...);
void displayMat(std::string winName, Mat m);
void nv21_to_bgr(int w, int h, std::string filepath);

// Resolve the first existing path from candidates (relative to CWD or absolute).
std::string resolveExistingPath(const std::vector<std::string>& candidates);

// Project-relative resource helpers (learn exercises).
std::string getProjectRoot();
std::string getDataRoot();
std::string getModelsRoot();
std::string getImagePath(const std::string& imageName);
std::string getModelPath(const std::string& modelName);

// Create a synthetic BGR test image when data files are unavailable.
Mat makeSyntheticTestImage(int width = 640, int height = 480);

void dbgMatInfo(const std::string& tag, const Mat& m);
void dbgPixel(const std::string& tag, const Mat& m, int x, int y);
void dbgStats(const std::string& tag, const Mat& m);
void dbgStatsROI(const std::string& tag, const Mat& m, const Rect& roi);
bool dbgCheck(const std::string& tag, const Mat& m, bool expectContinuous = false);
bool dbgSave(const std::string& tag, const Mat& m, const std::string& dir = ".");
void dbgShow(const std::string& winName, const Mat& m, int delay = 0);
void dbgShowMany(const std::vector<std::string>& names,
                 const std::vector<Mat>& imgs, int delay = 0);
double dbgTime(const std::string& label = "default");
void dbgPrint(const std::string& tag, int v);
void dbgPrint(const std::string& tag, double v);
void dbgPrint(const std::string& tag, const std::string& v);
void dbgPrint(const std::string& tag, const Size& s);
void dbgPrint(const std::string& tag, const Rect& r);
void dbgPrint(const std::string& tag, const Point& p);

int64_t getCurrentTimeMs();

#endif //OPENCV_UNITS_H
