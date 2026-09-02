// LEARN: L3 Kalman 婊ゆ尝锛?D 鍖€閫燂級
// OFFICIAL: samples/cpp/kalman.cpp
// THEORY: docs/ch04_video.md 搂Kalman
// TASK: 鐘舵€乕x,y,vx,vy] 瑙傛祴[x,y]锛屾ā鎷熻娴嬪簭鍒楋紝predict/correct锛岀敾杞ㄨ抗
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    KalmanFilter KF(4, 2, 0);
    KF.transitionMatrix = (Mat_<float>(4, 4) <<
        1, 0, 1, 0,
        0, 1, 0, 1,
        0, 0, 1, 0,
        0, 0, 0, 1);
    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
    setIdentity(KF.measurementNoiseCov, Scalar::all(2.5));
    setIdentity(KF.errorCovPost, Scalar::all(1));

    Mat canvas(400, 600, CV_8UC3, Scalar(0, 0, 0));
    RNG rng(12345);
    Point2f truePos(50, 100), vel(2.0f, 1.0f);
    Point2f estPos;

    for (int i = 0; i < 200; ++i) {
        truePos += vel;
        Point2f meas(truePos.x + (float)rng.gaussian(2),
                     truePos.y + (float)rng.gaussian(2));

        Mat pred = KF.predict();
        Point2f predPos(pred.at<float>(0), pred.at<float>(1));

        Mat measMat = (Mat_<float>(2, 1) << meas.x, meas.y);
        Mat corr = KF.correct(measMat);
        estPos = Point2f(corr.at<float>(0), corr.at<float>(1));

        circle(canvas, truePos, 1, Scalar(255, 255, 255), -1); // 鐧斤細鐪熷€?        circle(canvas, meas,   1, Scalar(0, 0, 255),     -1); // 绾細瑙傛祴
        circle(canvas, estPos, 1, Scalar(0, 255, 0),     -1); // 缁匡細婊ゆ尝
        if (i % 20 == 0) {
            imshow("L3_22 kalman", canvas);
            if (waitKey(10) == 27) break;
        }
    }
    putText(canvas, "white=true red=meas green=kalman", Point(10, 30),
            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
    dbgShow("L3_22 kalman", canvas, 0);
    return 0;
}
