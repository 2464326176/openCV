void readImage() {
    const Mat srcImage = imread("../data/images/3.jpg");

    imshow("original image", srcImage);
    waitKey(0);
}

void ImageErosion() {
    const Mat srcImage = imread("../data/images/OIP.png");
    imshow("original image", srcImage);

    Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
    Mat dstImage;
    erode(srcImage, dstImage, element);
    imshow("erosion effect image", dstImage);

    imwrite("../data/images/erosion effect image.jpg", dstImage);
    waitKey(0);
}

void ImageBlur() {

    const Mat srcImage = imread("../data/images/OIP.png");
    imshow("original image", srcImage);

    Mat dstImage;
    blur(srcImage, dstImage, Size(7, 7));
    imshow("blur effect image", dstImage);

    imwrite("../data/images/blur effect image.jpg", dstImage);
    waitKey(0);

}

void ImageCanny() {
    const Mat srcImage = imread("../data/images/OIP.png");
    imshow("original image", srcImage);

    Mat dstImage, grayImage, edge;

    dstImage.create(srcImage.size(), srcImage.type());
    cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);

    blur(grayImage, edge, Size(3, 3));
    Canny(edge, edge, 3, 9, 3);
    imshow("canny effect image", edge);
    

    Canny(srcImage, dstImage, 3, 9, 3);
    imshow("canny effect image1", dstImage);
    imwrite("../data/images/canny effect image.jpg", dstImage);
    waitKey(0);
}

void ImageVideo() {
    VideoCapture video(0);
    Mat frame, edge, gray;
    while (true) {
        video >> frame;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        blur(gray, edge, Size(7, 7));
        Canny(edge, edge, 0, 100, 3);
        imshow("video edge image", edge);
        waitKey(30);
    }
    video.release();
}

/**
 * @brief create alpha mask
 * @param mat input image blur 255, green left light right dark horizontal change, red top light bottom dark vertical change, alpha average of horizontal and vertical gradual change
 * @return none
 * @note Mat type | at<> template parameter
 * - `CV_8UC1` | `uchar`
 * - `CV_8UC3` | `Vec3b`
 * - `CV_8UC4` | `Vec4b`
 * - `CV_16UC1` | `ushort`
 * - `CV_32FC1` | `float`
 * - `CV_64FC1` | `double`
 * - `CV_64FC3` | `Vec3d`
 * - `CV_64FC4` | `Vec4d`
 */
void CreateAlphaMask(Mat &mat) {
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            Vec4b &bgra = mat.at<Vec4b>(i, j);
            bgra[0] = UCHAR_MAX;
            bgra[1] = saturate_cast<uchar>((float(mat.cols - j)) / float(mat.cols) * UCHAR_MAX);
            bgra[2] = saturate_cast<uchar>((float(mat.rows - i)) / float(mat.rows) * UCHAR_MAX);
            bgra[3] = saturate_cast<uchar>((bgra[1] + bgra[2]) * 0.5);
        }
    }
}

void ImageAlphaMask() {
    Mat mat(480, 640, CV_8UC4);
    CreateAlphaMask(mat);

    std::vector<int> compression_params;
    compression_params.push_back(IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

    imshow("alpha mask image", mat);
    imwrite("../data/images/alpha mask image.png", mat, compression_params);
    waitKey(0);
}

void ImageAddWeighted() {
    Mat srcImage = imread("../data/images/VCG2.jpg");
    Mat logoImage = imread("../data/images/VCG4.jpg");

    if (srcImage.empty() || logoImage.empty()) {
        std::cerr << "Error: Failed to load images." << std::endl;
        return;
    }

    if (srcImage.size() != logoImage.size() || srcImage.type() != logoImage.type()) {
        resize(logoImage, logoImage, srcImage.size());
        if (srcImage.type() != logoImage.type()) {
            logoImage.convertTo(logoImage, srcImage.type());
        }
    }

    struct WeightParams {
        int maxAlphaValue;
        const char *windowName;
        Mat srcImage;
        Mat logoImage;
    };

    WeightParams *weightParams = new WeightParams{
        100,
        "alpha mask image",
        srcImage,
        logoImage
    };

    auto onTrackbar = [](int alphaValue, void *userdata) {
        WeightParams *params = static_cast<WeightParams *>(userdata);

        double dAlphaValue = static_cast<double>(alphaValue) / params->maxAlphaValue;
        double dBetaValue = 1.0 - dAlphaValue;

        Mat dstImage;
        addWeighted(params->srcImage, dAlphaValue,
                    params->logoImage, dBetaValue,
                    0.0, dstImage);

        imshow(params->windowName, dstImage);
    };

    const char *windowName = weightParams->windowName;
    namedWindow(windowName, WINDOW_AUTOSIZE);

    int nAlphaValueSlider = 70;
    createTrackbar("Alpha Value", windowName,
                   &nAlphaValueSlider, weightParams->maxAlphaValue,
                   onTrackbar, weightParams);

    onTrackbar(nAlphaValueSlider, weightParams);

    waitKey(0);
    delete weightParams;
}

int main() {
    // readImage();
    // ImageErosion();
    // ImageBlur();
    // ImageCanny();
    // ImageVideo();
    // ImageAlphaMask();
    ImageAddWeighted();
    return 0;
}
