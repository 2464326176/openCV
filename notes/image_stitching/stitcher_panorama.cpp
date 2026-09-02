//********************
// Author:  yh
// Time:    2022/8/5.
//  Official one-shot panorama stitching via Stitcher
//  Official example: stitching.cpp (simplified) / stitching_detailed.cpp (full tuning)
//  Usage: pass 2 or more overlapping image paths
//********************
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    vector<Mat> images;

    // Use two test images by default; or pass several from the command line
    if (argc > 2) {
        for (int i = 1; i < argc; i++) images.push_back(imread(argv[i]));
    } else {
        images.push_back(imread("../data/images/VCG1.jpg"));
        images.push_back(imread("../data/images/VCG2.jpg"));
    }
    for (size_t i = 0; i < images.size(); i++)
        if (images[i].empty()) { cout << "image " << i << " empty" << endl; return -1; }

    // Create the stitcher and run it
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
