//********************
// Author:  yh 
// Time:    2021/5/12.
// 
//********************
//
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat srcImage;
const int alpha_max = 5;
const int beta_max = 125;
int alpha, beta;

String imageName("../static/data/lena.jpg");

static void onTrackbar(int, void *) {
    Mat new_image = Mat::zeros(srcImage.size(), srcImage.type());

    for (int y = 0; y < srcImage.rows; ++y) {
        for (int x = 0; x < srcImage.cols; ++x) {
            for (int c = 0; c < 3; ++c) {
                new_image.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( srcImage.at<Vec3b>(y,x)[c] ) + beta );
            }
        }
    }

    imshow("New Image", new_image);
}


int main() {
    srcImage = imread(imageName);

    alpha = 1;
    beta = 0;

    namedWindow("alpha", 1);
    namedWindow("new image", 1);

    createTrackbar("contrast", "new image", &alpha, alpha_max, onTrackbar);
    createTrackbar("brightness", "new image", &alpha, beta_max, onTrackbar);

    imshow("origin image", srcImage);
    imshow("new image", srcImage);
    waitKey();

    return 0;
}