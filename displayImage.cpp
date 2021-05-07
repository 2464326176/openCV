#include <opencv2/opencv.hpp>
using namespace cv;

int main()
{
    char* imageName = "./data/HappyFish.jpg";
    Mat image = imread(imageName, IMREAD_COLOR);

    Mat gray_image;

    cvtColor(image, gray_image, COLOR_BGR2GRAY);
    imwrite("./image/gray_image.jpg", gray_image);

    namedWindow(imageName, WINDOW_AUTOSIZE);
    namedWindow("gray image", WINDOW_AUTOSIZE);

    imshow(imageName, image);
    imshow("gray image", gray_image);
    waitKey(0);
    return 0;
}