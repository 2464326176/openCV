// LEARN: L5 ANN_MLP 绁炵粡缃戠粶
// OFFICIAL: samples/cpp/neural_network.cpp
// THEORY: docs/ch05_ml.md 搂ANN
// TASK: 鍚堟垚 XOR 鏁版嵁锛岃缁冧袱灞?MLP锛屽彲瑙嗗寲鍐崇瓥杈圭晫
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>
using namespace cv;
using namespace cv::ml;

int main() {
    Mat samples = (Mat_<float>(4,2) << 0,0, 0,1, 1,0, 1,1);
    Mat labels  = (Mat_<float>(4,1) << 0, 1, 1, 0);

    Ptr<ANN_MLP> ann = ANN_MLP::create();
    Mat layers = (Mat_<int>(1,3) << 2, 4, 1);
    ann->setLayerSizes(layers);
    ann->setActivationFunction(ANN_MLP::SIGMOID_SYM, 1, 1);
    ann->setTrainMethod(ANN_MLP::BACKPROP);
    ann->setBackpropWeightScale(0.1);
    ann->setBackpropMomentumScale(0.1);
    Ptr<TrainData> td = TrainData::create(samples, ROW_SAMPLE, labels);
    ann->train(td);

    const int w = 400, h = 400;
    Mat canvas(h, w, CV_8UC3, Scalar(0,0,0));
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            Mat m = (Mat_<float>(1,2) << j/(float)w, i/(float)h);
            Mat r; ann->predict(m, r);
            canvas.at<Vec3b>(i,j) = r.at<float>(0) > 0.5f ? Vec3b(0,100,0) : Vec3b(100,0,0);
        }
    for (int i = 0; i < 4; ++i)
        circle(canvas, Point((int)(samples.at<float>(i,0)*w),(int)(samples.at<float>(i,1)*h)), 5,
               labels.at<float>(i) > 0.5f ? Scalar(255,255,255) : Scalar(0,0,255), -1);

    dbgShow("L5_06 ANN MLP XOR", canvas, 0);
    return 0;
}
