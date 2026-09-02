//********************
// Author:  yh
// Time:    2022/8/4.
//  Five filters compared: box / mean / Gaussian / median / bilateral
//  - Linear filters: boxFilter, blur, GaussianBlur
//  - Non-linear filters: medianBlur (removes salt-and-pepper), bilateralFilter (edge-preserving)
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main() {
    Mat src = imread("../data/images/OIP.png");
    if (src.empty()) {
        cout << "could not load image.." << endl;
        return -1;
    }
    imshow("original image", src);

    // 1. Box filter: equal kernel weights; equals mean filter when normalize=true
    Mat boxDst;
    boxFilter(src, boxDst, -1, Size(5, 5));
    imshow("box filter", boxDst);

    // 2. Mean filter: normalized box; simple and fast but blurs edges
    Mat meanDst;
    blur(src, meanDst, Size(5, 5), Point(-1, -1));
    imshow("mean filter", meanDst);

    // 3. Gaussian filter: max weight at center, better edge-preserving than mean; standard Canny preprocessing
    Mat gaussDst;
    GaussianBlur(src, gaussDst, Size(5, 5), 0, 0);
    imshow("gaussian filter", gaussDst);

    // 4. Median filter: take neighborhood median; the nemesis of salt-and-pepper noise
    Mat medianDst;
    medianBlur(src, medianDst, 5);
    imshow("median filter", medianDst);

    // 5. Bilateral filter: spatial + range double weighting; denoises while preserving edges
    Mat bilateralDst;
    bilateralFilter(src, bilateralDst, 5, 10.0, 2.5);
    imshow("bilateral filter", bilateralDst);

    waitKey(0);
    return 0;
}
