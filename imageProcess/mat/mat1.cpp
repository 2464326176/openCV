#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
    Mat srcImage = imread("../static/data/blox.jpg");
    imshow("Display Image window", srcImage);
    /* 1 */
    //Mat M;
    //M.create(4, 4, CV_8UC2);
    /* 2 */
    //Mat M(2, 2, CV_8UC3, Scalar(0, 0, 255));

    /*int sz[3] = {3, 3, 3};
    Mat M(3, sz, CV_8UC1, Scalar::all(0));*/

//    cout << "M = " << M << endl;
    /*Mat M = (Mat_<double>(3,3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    Mat_<double> 对应	CV_64F； 	Mat_<uchar>对应	CV_8U
    Mat_<char>对应	CV_8S；	Mat_<int>对应	CV_32S
    Mat_<float>对应	CV_32F；	Mat_<double>对应	CV_64F*/

    waitKey(0);
    return 0;
}