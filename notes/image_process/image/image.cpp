/*
 * @Author: yh
 * @Date: 2022/8/5 0:57
 * @Description: 
 * @FilePath: image.cpp
 */
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int ImageFusion() {
    // Load images (use absolute paths or ensure relative paths are correct)
    Mat src = imread("../static/data/lena.jpg");
    Mat winLogo = imread("../static/data/WindowsLogo.jpg");
    Mat linuxLogo = imread("../static/data/LinuxLogo.jpg");
    Mat mask = imread("../static/data/WindowsLogo.jpg", IMREAD_GRAYSCALE);
    
    // Check whether images loaded successfully
    if (src.empty() || winLogo.empty() || linuxLogo.empty() || mask.empty()) {
        cout << "Could not open or find the images!" << endl;
        return -1;
    }
    
    Mat linearBlend;
    double alpha = 0.5;
    double beta = 1.0 - alpha;
    vector<Mat> dest;
    split(winLogo, dest);
    
    // Compute ROI position
    Point2i p2i;
    p2i.x = src.cols / 2 - winLogo.cols / 2;
    p2i.y = src.rows / 2 - winLogo.rows / 2;
    
    // Ensure ROI stays within the image bounds
    if (p2i.x < 0 || p2i.y < 0 || 
        p2i.x + winLogo.cols > src.cols || 
        p2i.y + winLogo.rows > src.rows) {
        cout << "Logo is too large for the source image!" << endl;
        return -1;
    }
    
    Mat imageROI = src(Rect(p2i.x, p2i.y, winLogo.cols, winLogo.rows));
    
    // Method 1: linear blending with addWeighted (commented out)
    // addWeighted(imageROI, alpha, winLogo, beta, 0, imageROI);
    
    // Method 2: masked copy with copyTo (fixed version)
    // Use dest[1] (green channel) as the mask to copy winLogo onto imageROI
    // Note: copyTo's second argument is the mask, must be an 8-bit single-channel image
    Mat maskChannel = dest[1]; // green channel as mask
    winLogo.copyTo(imageROI, maskChannel);
    
    // Method 3: use green channel as mask to copy a single channel of winLogo to ROI
    // To copy only the green channel:
    // dest[1].copyTo(imageROI, maskChannel);
    
    imshow("Blended Lena", src);
    waitKey(0);
    destroyAllWindows();
    
    return 0;
}

int main() {
    ImageFusion();
    return 0;
}