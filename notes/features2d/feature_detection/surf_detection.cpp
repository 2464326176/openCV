#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/features2d.hpp"

using namespace cv;
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    String imageName("../static/gril/0.jpg");
    if (argc > 1)
        imageName = argv[1];

    Mat src = imread( imageName, IMREAD_GRAYSCALE );
    if ( src.empty() )
    {
        cout << "Could not open or find the image!\n" << endl;
        cout << "Usage: " << argv[0] << " <Input image>" << endl;
        return -1;
    }

    //-- Step 1: Detect the keypoints using SIFT Detector
    int nfeatures = 0;
    Ptr<SIFT> detector = SIFT::create( nfeatures );
    std::vector<KeyPoint> keypoints;
    detector->detect( src, keypoints );

    //-- Draw keypoints
    Mat img_keypoints;
    drawKeypoints( src, keypoints, img_keypoints );

    //-- Show detected (drawn) keypoints
    imshow("SIFT Keypoints", img_keypoints );

    waitKey();
    return 0;
}

