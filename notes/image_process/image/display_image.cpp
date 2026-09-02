#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

int main() {
    const char *imageName = "./data/HappyFish.jpg";
    Mat image = imread(imageName, IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "Could not open or find the image: " << imageName << std::endl;
        return -1;
    }

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