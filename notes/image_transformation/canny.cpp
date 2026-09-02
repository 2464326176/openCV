#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
int main()
{
    String imgPath = "../static/img/yuhang.jpg";
    String imageName(imgPath.c_str());

    Mat image, edges;
    image = imread(imageName, IMREAD_COLOR);

    if(image.empty())
    {
        return -1;
    }

    cvtColor(image, edges, COLOR_BGR2GRAY);
    imshow("gray", edges);
        
    blur(edges, edges, Size(3, 3));  // 3x3 kernel for denoising
    /*
     * Canny
     * 3rd param: first (lower) hysteresis threshold
     * 4th param: second (upper) hysteresis threshold
     * the smaller one links edges; the larger controls strong-edge seeds; ratio usually 2:1~3:1
     * 5th param: aperture size of the Sobel operator
     */

    Canny(edges, edges, 3, 9, 3);

    namedWindow("display window", WINDOW_KEEPRATIO);
    imshow("display window", edges);  
    waitKey(0);

    return 0;
}
