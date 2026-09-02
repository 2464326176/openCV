/*
 * @Author: yh
 * @Date: 2022/8/2 0:46
 * @Description: 
 * @FilePath: mat.cpp
 */
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int mat_text() {
    // 1. Create a 2x2 matrix with 3 channels (8-bit unsigned), initialized to (0, 0, 255) — i.e. red in BGR
    Mat M(2, 2, CV_8UC3, Scalar(0, 0, 255));
    cout << "M = " << M << endl;

    // 2. Create a matrix with more than two dimensions.
    //    Pass the number of dimensions and a pointer to an array containing the size of each dimension.
    int matSize[3] = {2, 2, 2};
    Mat L(3, matSize, CV_8UC1, Scalar::all(0));
    Mat L_2d = L.reshape(1, matSize[0] * matSize[1]); // 行数=深度*行，列数=原始列
    cout << L_2d << endl;

    // 3. Load an image and create a deep copy (clone) of it.
    //    Note: Mat mtx(img) would be a shallow copy (shared data); use .clone() for an independent copy.
    Mat img = imread("../data/images/lena.jpg");
    if (img.empty()) {
        cerr << "Error: Failed to load image." << endl;
        return 0;
    }
    Mat mtx = img.clone();
    cout << "mtx = " << mtx << endl;

    // 4. Reallocate M to a 4x4 matrix with 2 channels (8-bit unsigned).
    //    Prefer the explicit macro CV_8UC2 over CV_8UC(2) for clarity.
    M.create(4, 4, CV_8UC2);
    cout << "M = " << M << endl;

    // 5. Create special matrices: identity, ones, and zeros (64-bit float).
    Mat E = Mat::eye(4, 4, CV_64F);
    Mat O = Mat::ones(4, 4, CV_64F);
    Mat Z = Mat::zeros(4, 4, CV_64F);
    cout << "E = " << E << endl;
    cout << "O = " << O << endl;
    cout << "Z = " << Z << endl;

    // 6. Initialize a 3x3 matrix using a comma-separated list (sharpening kernel).
    Mat C = (Mat_<double>(3, 3) << 0, -1, 0,
                                -1,  5, -1,
                                0, -1, 0);
    cout << "C = " << C << endl;

    // 7. Clone a specific row from matrix C.
    Mat RowClone = C.row(1).clone();
    cout << "RowClone = " << RowClone << endl;
    return 0;
}

int main() {
    mat_text();
    return 0;
}