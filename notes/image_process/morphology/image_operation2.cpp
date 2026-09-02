//********************
// Author:  yh
// Time:    2022/8/4.
//  Morphology + thresholding + image pyramids demo
//  - Morphology: dilate / erode / morphologyEx (open/close/gradient/top-hat/black-hat)
//  - Thresholding: threshold (fixed + Otsu) / adaptiveThreshold (adaptive)
//  - Pyramids: pyrDown / pyrUp
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

static Mat src;

// Seven morphological operations, demoed via the unified morphologyEx interface
void morphologyDemo() {
    Mat kernel = getStructuringElement(MORPH_RECT, Size(15, 15));

    Mat dilateDst, erodeDst, openDst, closeDst, gradDst, tophatDst, blackhatDst;
    dilate(src, dilateDst, kernel);                       // dilation: expand bright regions
    erode(src, erodeDst, kernel);                         // erosion: shrink bright regions
    morphologyEx(src, openDst,    MORPH_OPEN,    kernel); // opening: remove small bright spots
    morphologyEx(src, closeDst,   MORPH_CLOSE,   kernel); // closing: fill small holes
    morphologyEx(src, gradDst,    MORPH_GRADIENT,kernel); // morphological gradient: dilation - erosion
    morphologyEx(src, tophatDst,   MORPH_TOPHAT,  kernel); // top-hat: src - opening
    morphologyEx(src, blackhatDst,MORPH_BLACKHAT, kernel);// black-hat: closing - src

    imshow("dilate",     dilateDst);
    imshow("erode",      erodeDst);
    imshow("open",       openDst);
    imshow("close",      closeDst);
    imshow("gradient",   gradDst);
    imshow("tophat",     tophatDst);
    imshow("blackhat",   blackhatDst);
}

// Thresholding: fixed threshold + Otsu auto-threshold + adaptive threshold
void thresholdDemo() {
    Mat gray, binary, otsu, adapt;

    cvtColor(src, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 0);   // denoise first to avoid fragmented thresholding

    // fixed-threshold binarization
    threshold(gray, binary, 119, 255, THRESH_BINARY);

    // Otsu's method finds the global threshold automatically
    threshold(gray, otsu, 0, 255, THRESH_BINARY | THRESH_OTSU);

    // adaptive threshold: neighborhood weighted mean - C, overcomes uneven illumination
    adaptiveThreshold(gray, adapt, 255,
                      ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, 2);

    imshow("binary",  binary);
    imshow("otsu",    otsu);
    imshow("adaptive", adapt);
}

// Image pyramids: Gaussian blur then downsample / interpolate then upsample
void pyramidDemo() {
    Mat down, up;
    pyrDown(src, down, Size(src.cols / 2, src.rows / 2));  // downsample: halve the size
    pyrUp(src,   up,   Size(src.cols * 2, src.rows * 2));  // upsample: enlarge but blurry

    imshow("pyrDown", down);
    imshow("pyrUp",   up);
}

int main() {
    src = imread("../data/images/OIP.png");
    if (src.empty()) {
        cout << "could not load image.." << endl;
        return -1;
    }
    imshow("original image", src);

    morphologyDemo();
    thresholdDemo();
    pyramidDemo();

    waitKey(0);
    return 0;
}
