//********************
// Author:  yh
// Time:    2022/8/5.
//  官方 Stitcher 一键全景拼接
//  对应官方示例: stitching.cpp（简化版）/ stitching_detailed.cpp（完整调参版）
//  用法: 传 2 张及以上有重叠的图像路径
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    vector<Mat> images;

    // 默认用两张测试图；也可从命令行传入多张
    if (argc > 2) {
        for (int i = 1; i < argc; i++) images.push_back(imread(argv[i]));
    } else {
        images.push_back(imread("../data/images/VCG1.jpg"));
        images.push_back(imread("../data/images/VCG2.jpg"));
    }
    for (size_t i = 0; i < images.size(); i++)
        if (images[i].empty()) { cout << "image " << i << " empty" << endl; return -1; }

    // 创建拼接器并执行
    Ptr<Stitcher> stitcher = Stitcher::create(Stitcher::PANORAMA);
    Mat pano;
    Stitcher::Status status = stitcher->stitch(images, pano);

    if (status != Stitcher::OK) {
        cout << "stitching failed: " << (int)status << endl;
        return -1;
    }
    imshow("panorama", pano);
    imwrite("panorama.png", pano);
    waitKey(0);
    return 0;
}
