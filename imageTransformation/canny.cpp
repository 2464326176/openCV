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
        
    blur(edges, edges, Size(3, 3));  //3x3 内核降噪
    /*
     * canny
     * 第三个参数 第一个滞后性阈值
     * 第四个参数 第二个滞后性阈值
     * 较小的用于边缘连接 较大的控制强边缘的初始段一般在（2:1~3:1）
     * 第五个参数 Sobel算子的孔径大小
     */

    Canny(edges, edges, 3, 9, 3);

    namedWindow("display window", WINDOW_KEEPRATIO);
    imshow("display window", edges);  
    waitKey(0);

    return 0;
}