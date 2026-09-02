#include "opencv2/photo.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <vector>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

#define IMAGE_PATH "../../static/testdata/cv/hdr/exposures/"
#define IMAGE_NV21_PATH "../img/20240808_085239_105_085239_hdr_4000x3000_0000_0_in_ev0.000.nv21"
void loadExposureSeq(String, vector<Mat>&, vector<float>&);


// NV21转BGR工具函数
Mat convertNV21toBGR(const Mat& nv21, int width, int height) {
    // 创建目标Mat
    Mat bgr(height, width, CV_8UC3);
    
    // 使用OpenCV颜色转换
    cvtColor(nv21, bgr, COLOR_YUV2BGR_NV21);
    
    return bgr;
}

int main(int argc, char** argv)
{
    CommandLineParser parser(argc, argv, "{@input | | Input directory that contains NV21 images and exposure times. }");
    
    // 获取输入路径
    String path = parser.get<String>("@input");
    if (path.empty()) {
        std::cerr << "Please specify input directory" << std::endl;
        return -1;
    }

    //! [Load images and exposure times]
    vector<Mat> images;
    vector<float> times;
    // 修改调用以支持NV21
    loadExposureSeq(path, images, times);
    //! [Load images and exposure times]


    //! [Estimate camera response]
    Mat response;
    Ptr<CalibrateDebevec> calibrate = createCalibrateDebevec();
    calibrate->process(images, response, times);
    //! [Estimate camera response]

    //! [Make HDR image]
    Mat hdr;
    Ptr<MergeDebevec> merge_debevec = createMergeDebevec();
    merge_debevec->process(images, hdr, times, response);
    //! [Make HDR image]

    //! [Tonemap HDR image]
    Mat ldr;
    Ptr<Tonemap> tonemap = createTonemap(2.2f);
    tonemap->process(hdr, ldr);
    //! [Tonemap HDR image]

    //! [Perform exposure fusion]
    Mat fusion;
    Ptr<MergeMertens> merge_mertens = createMergeMertens();
    merge_mertens->process(images, fusion);
    //! [Perform exposure fusion]

    //! [Write results]
    imwrite("fusion.png", fusion * 255);
    imwrite("ldr.png", ldr * 255);
    imwrite("hdr.hdr", hdr);

    //! [Write results]

    return 0;
}

void loadExposureSeq(String path, vector<Mat>& images, vector<float>& times)
{
    std::cout << "Loading NV21 images from: " << path << std::endl;
    path = path + "/";
    ifstream list_file((path + "list.txt").c_str());
    
    if (!list_file.is_open()) {
        std::cerr << "Error opening list.txt" << std::endl;
        return;
    }

    string name;
    float val;
    int width, height;  // 新增宽高参数
    
    while (list_file >> name >> val >> width >> height) {
        // 读取NV21二进制文件
        std::ifstream file(path + name, std::ios::binary | std::ios::ate);
        if (!file) {
            std::cerr << "Error opening: " << name << std::endl;
            continue;
        }
        
        // 计算文件大小并读取
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size)) {
            std::cerr << "Read error: " << name << std::endl;
            continue;
        }
        
        // 创建NV21格式的Mat (单通道)
        Mat nv21Mat(height + height/2, width, CV_8UC1, buffer.data());
        
        // 转换为BGR
        Mat bgrMat = convertNV21toBGR(nv21Mat, width, height);
        
        if (bgrMat.empty()) {
            std::cerr << "Conversion failed: " << name << std::endl;
            continue;
        }
        
        images.push_back(bgrMat.clone());  // 克隆数据确保安全
        times.push_back(1 / val);
        std::cout << "Loaded: " << name << " size: " << width << "x" << height << std::endl;
    }
    list_file.close();
}
