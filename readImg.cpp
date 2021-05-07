#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
int main()
{
    String imageName("./data/HappyFish.jpg");

    Mat image;
    image = imread(imageName, IMREAD_COLOR);
    if(image.empty())
    {
        return -1;
    }
    
    namedWindow("display window", WINDOW_AUTOSIZE);
    imshow("display window", image);
    waitKey(0);

    return 0;
}