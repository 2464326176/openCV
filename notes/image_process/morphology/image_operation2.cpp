//********************
// Author:  yh
// Time:    2022/8/4.
//  形态学运算 + 阈值化 + 图像金字塔 演示
//  - 形态学：dilate / erode / morphologyEx (开/闭/梯度/顶帽/黑帽)
//  - 阈值化：threshold (固定 + Otsu) / adaptiveThreshold (自适应)
//  - 金字塔：pyrDown / pyrUp
//********************
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

static Mat src;

// 形态学七运算：用统一接口 morphologyEx 演示
void morphologyDemo() {
    Mat kernel = getStructuringElement(MORPH_RECT, Size(15, 15));

    Mat dilateDst, erodeDst, openDst, closeDst, gradDst, tophatDst, blackhatDst;
    dilate(src, dilateDst, kernel);                       // 膨胀：扩张亮区
    erode(src, erodeDst, kernel);                         // 腐蚀：收缩亮区
    morphologyEx(src, openDst,    MORPH_OPEN,    kernel); // 开运算：去细小亮斑
    morphologyEx(src, closeDst,   MORPH_CLOSE,   kernel); // 闭运算：填小孔洞
    morphologyEx(src, gradDst,    MORPH_GRADIENT,kernel); // 形态学梯度：膨胀-腐蚀
    morphologyEx(src, tophatDst,   MORPH_TOPHAT,  kernel); // 顶帽：原图-开运算
    morphologyEx(src, blackhatDst,MORPH_BLACKHAT, kernel);// 黑帽：闭运算-原图

    imshow("dilate",     dilateDst);
    imshow("erode",      erodeDst);
    imshow("open",       openDst);
    imshow("close",      closeDst);
    imshow("gradient",   gradDst);
    imshow("tophat",     tophatDst);
    imshow("blackhat",   blackhatDst);
}

// 阈值化：固定阈值 + Otsu 自动阈值 + 自适应阈值
void thresholdDemo() {
    Mat gray, binary, otsu, adapt;

    cvtColor(src, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 0);   // 先降噪，避免阈值化碎裂

    // 固定阈值二值化
    threshold(gray, binary, 119, 255, THRESH_BINARY);

    // Otsu 大津法自动求全局阈值
    threshold(gray, otsu, 0, 255, THRESH_BINARY | THRESH_OTSU);

    // 自适应阈值：邻域加权均值 - C，克服光照不均
    adaptiveThreshold(gray, adapt, 255,
                      ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, 2);

    imshow("binary",  binary);
    imshow("otsu",    otsu);
    imshow("adaptive", adapt);
}

// 图像金字塔：先高斯模糊再下采样 / 插值上采样
void pyramidDemo() {
    Mat down, up;
    pyrDown(src, down, Size(src.cols / 2, src.rows / 2));  // 下采样：尺寸减半
    pyrUp(src,   up,   Size(src.cols * 2, src.rows * 2));  // 上采样：放大但模糊

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
