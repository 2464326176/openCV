// LEARN: L5 DNN handwritten digit inference bridge
// OFFICIAL: samples/cpp/digits_lenet.cpp
// THEORY: docs/ch05_ml.md §5.9
// TASK: If no LeNet model, demonstrate blobFromImage + Net API; explain the ml vs dnn boundary
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>
#include <opencv_utils.h>

using namespace cv;
using namespace cv::dnn;

int main() {
    Mat digit = Mat::zeros(28, 28, CV_8U);
    circle(digit, Point(14, 14), 8, Scalar(255), -1);
    Mat input;
    resize(digit, input, Size(28, 28));

    std::string modelPath = getModelPath("lenet_digit.onnx");
    if (!std::ifstream(modelPath).good()) {
        logInfo("lenet_digit.onnx missing -> demonstrate blob + Net API only");
        Mat blob = blobFromImage(input, 1.0 / 255.0, Size(28, 28), Scalar(), false, false);
        logInfo("blob shape: %dx%dx%dx%d", blob.size[0], blob.size[1], blob.size[2], blob.size[3]);
        logInfo("classic ml path: HOG/Sobel -> SVM/KNN (see L5_02_svm_intro)");
        dbgShow("L5_13 digits dnn (synthetic digit)", input, 0);
        return 0;
    }

    Net net = readNet(modelPath);
    Mat blob = blobFromImage(input, 1.0 / 255.0, Size(28, 28), Scalar(), false, false);
    net.setInput(blob);
    Mat prob = net.forward();
    Point classId;
    double conf;
    minMaxLoc(prob.reshape(1, 1), nullptr, &conf, nullptr, &classId);
    logInfo("predicted digit=%d conf=%.3f", classId.x, conf);
    dbgShow("L5_13 digits dnn", input, 0);
    return 0;
}
