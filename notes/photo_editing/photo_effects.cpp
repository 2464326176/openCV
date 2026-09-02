//********************
// Author:  yh
// 照片特效与去色：
//  ① decolor 高质量去色（保留局部对比度，输出灰度 + 彩色增强图）
//  ② edgePreservingFilter 保边平滑（磨皮/降噪）
//  ③ detailEnhance 细节增强
//  ④ pencilSketch 素描
//  ⑤ stylization 风格化（卡通/油画）
//  对应官方示例: tutorial_code/photo/decolorization/decolor.cpp
//               tutorial_code/photo/non_photorealistic_rendering/npr_demo.cpp
//  使用：photo_effects <图片>
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    if (argc < 2) { cout << "用法: photo_effects <图片>" << endl; return -1; }
    Mat src = imread(argv[1], IMREAD_COLOR);
    if (src.empty()) { cout << "图像加载失败" << endl; return -1; }

    Mat gray, color_boost;
    decolor(src, gray, color_boost);                 // ① 高质量去色

    Mat epf_norm, epf_rec;
    edgePreservingFilter(src, epf_norm, 1);          // ②a 保边平滑(归一化卷积)
    edgePreservingFilter(src, epf_rec, 2);           // ②b 保边平滑(递归滤波)

    Mat detail;
    detailEnhance(src, detail);                      // ③ 细节增强

    Mat sketch, color_sketch;
    pencilSketch(src, sketch, color_sketch, 10, 0.1f, 0.03f);  // ④ 素描/彩色素描

    Mat stylized;
    stylization(src, stylized);                      // ⑤ 风格化

    imshow("source", src);
    imshow("decolor_gray", gray);
    imshow("decolor_boost", color_boost);
    imshow("edge_preserve", epf_rec);
    imshow("detail", detail);
    imshow("sketch", sketch);
    imshow("color_sketch", color_sketch);
    imshow("stylized", stylized);

    imwrite("../out/photo_decolor_gray.png", gray);
    imwrite("../out/photo_sketch.png", sketch);
    imwrite("../out/photo_stylized.png", stylized);
    waitKey(0);
    return 0;
}
