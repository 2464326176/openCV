//********************
// Author:  yh
// Photo effects and decolorization:
//  ① decolor high-quality decolorization (preserves local contrast; outputs gray + color-boosted image)
//  ② edgePreservingFilter edge-preserving smoothing (skin softening / denoising)
//  ③ detailEnhance detail enhancement
//  ④ pencilSketch pencil sketch
//  ⑤ stylization stylization (cartoon / oil painting)
//  Official examples: tutorial_code/photo/decolorization/decolor.cpp
//                    tutorial_code/photo/non_photorealistic_rendering/npr_demo.cpp
//  Usage: photo_effects <image>
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
    decolor(src, gray, color_boost);                 // ① high-quality decolorization

    Mat epf_norm, epf_rec;
    edgePreservingFilter(src, epf_norm, 1);          // ②a edge-preserving smoothing (normalized convolution)
    edgePreservingFilter(src, epf_rec, 2);           // ②b edge-preserving smoothing (recursive filter)

    Mat detail;
    detailEnhance(src, detail);                      // ③ detail enhancement

    Mat sketch, color_sketch;
    pencilSketch(src, sketch, color_sketch, 10, 0.1f, 0.03f);  // ④ pencil / color sketch

    Mat stylized;
    stylization(src, stylized);                      // ⑤ stylization

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
