// LEARN: L0 VideoWriter write and read back
// OFFICIAL: samples/cpp/videowriter_basic.cpp, tutorial_code/videoio/video-write/video-write.cpp
// THEORY: docs/ch08_gui_gapi_gpu.md §videoio
// TASK: write AVI from static image sequence, then read back with VideoCapture to verify frame count
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat base = imread(getImagePath("lena.jpg"));
    if (base.empty()) {
        logInfo("lena.jpg missing, using synthetic frames");
        base = makeSyntheticTestImage(320, 240);
    }
    if (base.channels() == 1) cvtColor(base, base, COLOR_GRAY2BGR);

    const std::string outPath = "l0_08_test.avi";
    const int fps = 10;
    const int frameCount = 20;
    Size frameSize(base.cols, base.rows);

    VideoWriter writer(outPath, VideoWriter::fourcc('M','J','P','G'), fps, frameSize, true);
    if (!writer.isOpened()) {
        logInfo("VideoWriter failed: %s", outPath.c_str());
        return -1;
    }

    for (int i = 0; i < frameCount; ++i) {
        Mat frame = base.clone();
        putText(frame, "frame " + std::to_string(i), Point(20, 40),
                FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 2);
        writer.write(frame);
    }
    writer.release();
    logInfo("wrote %d frames -> %s", frameCount, outPath.c_str());

    VideoCapture cap(outPath);
    if (!cap.isOpened()) {
        logInfo("read back failed: %s", outPath.c_str());
        return -1;
    }
    int readCount = 0;
    Mat frame;
    while (cap.read(frame)) ++readCount;
    logInfo("read back frames: %d (expected %d)", readCount, frameCount);

    cap.set(CAP_PROP_POS_FRAMES, 0);
    if (cap.read(frame) && !frame.empty()) dbgShow("L0_08 videowriter", frame, 1);
    return readCount == frameCount ? 0 : -1;
}
