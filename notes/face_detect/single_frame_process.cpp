#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

#define IMAGE_NV21_PATH "../data/nv21/20240808_085239_105_085239_hdr_4000x3000_0000_0_in_ev0.000.nv21"

// Laplacian sharpening function
cv::Mat laplacianSharpen(const cv::Mat& input, float alpha = 0.5) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat laplacian, sharpened;
    cv::Laplacian(input, laplacian, CV_32F);
    cv::convertScaleAbs(input + alpha * laplacian, sharpened);
    return sharpened;
}

// Unsharp masking function
cv::Mat unsharpMask(const cv::Mat& input, int kernelSize = 5, float sigma = 1.0, float amount = 1.0) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(kernelSize, kernelSize), sigma);
    
    cv::Mat sharpened;
    cv::addWeighted(input, 1.0 + amount, blurred, -amount, 0, sharpened);
    return sharpened;
}

int main() {
    // set image size (replace with your image size in real use)
    // const int width = 640;
    // const int height = 480;
    
    // // create simulated NV21 data (replace with your real data in practice)
    // const size_t nv21Size = width * height * 3 / 2;
    // std::vector<uint8_t> nv21Data(nv21Size);
    
    // // fill simulated data - Y plane
    // for (int i = 0; i < height; i++) {
    //     for (int j = 0; j < width; j++) {
    //         // create a simple gradient pattern
    //         nv21Data[i * width + j] = static_cast<uint8_t>((i + j) * 0.2);
    //     }
    // }
    
    // // fill simulated data - VU plane
    // for (int i = 0; i < height / 2; i++) {
    //     for (int j = 0; j < width; j += 2) {
    //         int index = width * height + i * width + j;
    //         // V component
    //         nv21Data[index] = 128 + static_cast<uint8_t>(j * 0.1);
    //         // U component
    //         nv21Data[index + 1] = 128 + static_cast<uint8_t>(i * 0.2);
    //     }
    // }

    try {
        // 1. Convert NV21 to BGR
        int width = 4000; // Example width
        int height = 3000; // Example height

        std::ifstream file(IMAGE_NV21_PATH, std::ios::binary);
        std::cout << "IMAGE_NV21_PATH: " << IMAGE_NV21_PATH << std::endl;
        std::vector<unsigned char> nv21_data(width * height * 3 / 2);
        file.read(reinterpret_cast<char*>(nv21_data.data()), nv21_data.size());

        cv::Mat nv21Mat(height + height / 2, width, CV_8UC1, nv21_data.data());

        cv::Mat bgrMat;
        cv::cvtColor(nv21Mat, bgrMat, cv::COLOR_YUV2BGR_NV21);


        // check whether the image is valid
        if (bgrMat.empty()) {
            std::cerr << "Error: Failed to convert NV21 to BGR" << std::endl;
            return -1;
        }
        imshow("Display window", bgrMat);
        
        // 2. Smoothing (denoising)
        cv::Mat smoothed;
        // use Gaussian blur
        cv::GaussianBlur(bgrMat, smoothed, cv::Size(5, 5), 0);
        
        // // 3. Sharpening
        // cv::Mat sharpened;
        // // Method 1: Laplacian sharpening
        // sharpened = laplacianSharpen(smoothed, 0.3);
        
        // // Method 2: unsharp masking (uncomment the line below to use)
        // // sharpened = unsharpMask(smoothed, 5, 1.0, 1.5);
        
        // show results
        cv::imshow("Original (BGR)", bgrMat);
        cv::imshow("Smoothed", smoothed);
        // cv::imshow("Sharpened", sharpened);
        
        imwrite("Original.png", bgrMat);
        imwrite("Smoothed.png", smoothed);
        // wait for a key
        cv::waitKey(0);
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV Exception: " << e.what() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Standard Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
