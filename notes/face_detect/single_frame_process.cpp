#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

#define IMAGE_NV21_PATH "../data/nv21/20240808_085239_105_085239_hdr_4000x3000_0000_0_in_ev0.000.nv21"

// 拉普拉斯锐化函数
cv::Mat laplacianSharpen(const cv::Mat& input, float alpha = 0.5) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat laplacian, sharpened;
    cv::Laplacian(input, laplacian, CV_32F);
    cv::convertScaleAbs(input + alpha * laplacian, sharpened);
    return sharpened;
}

// 非锐化掩蔽函数
cv::Mat unsharpMask(const cv::Mat& input, int kernelSize = 5, float sigma = 1.0, float amount = 1.0) {
    if (input.empty()) return cv::Mat();
    
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(kernelSize, kernelSize), sigma);
    
    cv::Mat sharpened;
    cv::addWeighted(input, 1.0 + amount, blurred, -amount, 0, sharpened);
    return sharpened;
}

int main() {
    // 设置图像尺寸 (实际使用中替换为你的图像尺寸)
    // const int width = 640;
    // const int height = 480;
    
    // // 创建模拟的 NV21 数据 (实际应用中替换为你的真实数据)
    // const size_t nv21Size = width * height * 3 / 2;
    // std::vector<uint8_t> nv21Data(nv21Size);
    
    // // 填充模拟数据 - Y 平面
    // for (int i = 0; i < height; i++) {
    //     for (int j = 0; j < width; j++) {
    //         // 创建简单的渐变图案
    //         nv21Data[i * width + j] = static_cast<uint8_t>((i + j) * 0.2);
    //     }
    // }
    
    // // 填充模拟数据 - VU 平面
    // for (int i = 0; i < height / 2; i++) {
    //     for (int j = 0; j < width; j += 2) {
    //         int index = width * height + i * width + j;
    //         // V 分量
    //         nv21Data[index] = 128 + static_cast<uint8_t>(j * 0.1);
    //         // U 分量
    //         nv21Data[index + 1] = 128 + static_cast<uint8_t>(i * 0.2);
    //     }
    // }

    try {
        // 1. 将 NV21 转换为 BGR
        int width = 4000; // Example width
        int height = 3000; // Example height

        std::ifstream file(IMAGE_NV21_PATH, std::ios::binary);
        std::cout << "IMAGE_NV21_PATH: " << IMAGE_NV21_PATH << std::endl;
        std::vector<unsigned char> nv21_data(width * height * 3 / 2);
        file.read(reinterpret_cast<char*>(nv21_data.data()), nv21_data.size());

        cv::Mat nv21Mat(height + height / 2, width, CV_8UC1, nv21_data.data());

        cv::Mat bgrMat;
        cv::cvtColor(nv21Mat, bgrMat, cv::COLOR_YUV2BGR_NV21);


        // 检查图像是否有效
        if (bgrMat.empty()) {
            std::cerr << "Error: Failed to convert NV21 to BGR" << std::endl;
            return -1;
        }
        imshow("Display window", bgrMat);
        
        // 2. 平滑处理 (降噪)
        cv::Mat smoothed;
        // 使用高斯模糊
        cv::GaussianBlur(bgrMat, smoothed, cv::Size(5, 5), 0);
        
        // // 3. 锐化处理
        // cv::Mat sharpened;
        // // 方法1: 使用拉普拉斯锐化
        // sharpened = laplacianSharpen(smoothed, 0.3);
        
        // // 方法2: 使用非锐化掩蔽 (取消下面一行的注释来使用)
        // // sharpened = unsharpMask(smoothed, 5, 1.0, 1.5);
        
        // 显示结果
        cv::imshow("Original (BGR)", bgrMat);
        cv::imshow("Smoothed", smoothed);
        // cv::imshow("Sharpened", sharpened);
        
        imwrite("Original.png", bgrMat);
        imwrite("Smoothed.png", smoothed);
        // 等待按键
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