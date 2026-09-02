//! [includes]
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

using namespace cv;
//! [includes]

#define IMAGE_PATH "../../static/data/lena.jpg"

int main1()
{
    std::string image_path(IMAGE_PATH);
    Mat img = imread(image_path, IMREAD_COLOR);

    if(img.empty())
    {
        std::cout << "Could not read the image: " << image_path << std::endl;
        return 1;
    }

    imshow("Display window", img);
    int k = waitKey(0);

    if(k == 's')
    {
        imwrite("starry_night.png", img);
    }
    return 0;
}