// LEARN: L1 Mat 鍒涘缓涓庣被鍨?// OFFICIAL: samples/cpp/tutorial_code/core/mat_the_basic_image_container/mat_the_basic_image_container.cpp銆乵at_operations.cpp
// THEORY: docs/ch01_core.md 搂2.1-2.4
// TASK: Mat(type,size) 澶氱被鍨嬬煩闃碉紱clone/copyTo/ROI/isContinuous锛涙敼 ROI 楠岃瘉鍏变韩瀛樺偍
#include <opencv2/opencv.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // 鏄惧紡鏋勯€犱笉鍚岀被鍨?灏哄鐭╅樀
    Mat f = Mat(3, 3, CV_32FC1, Scalar(1.5f));
    Mat u = Mat(2, 2, CV_8UC3, Scalar(10, 20, 30));
    Mat z = Mat::zeros(4, 4, CV_8UC1);
    Mat e = Mat::ones(3, 1, CV_64FC1);

    dbgMatInfo("f32", f);
    dbgMatInfo("u8c3", u);
    dbgMatInfo("zeros", z);
    dbgMatInfo("ones64", e);
    dbgPrint("f continuous", (int)f.isContinuous());

    // 寮曠敤璁℃暟涓庢繁娴呮嫹璐濓細= 鏄祬鎷疯礉锛宑lone() 鎵嶆槸娣辨嫹璐?    Mat src = imread(getImagePath("lena.jpg"));
    if (src.empty()) { logInfo("imread failed"); return -1; }

    Mat shallow = src;            // 鍏变韩鍚屼竴鍧楀唴瀛?    Mat deep    = src.clone();    // 娣辨嫹璐?    Mat copyto;
    src.copyTo(copyto);           // 娣辨嫹璐?
    // ROI锛歊ect 鍒涘缓瑙嗗浘锛屽叡浜師鍥惧唴瀛?    Rect roi(80, 80, 100, 100);
    Mat roiView = src(roi);
    roiView.setTo(Scalar(0, 0, 255)); // 鏀?ROI 浼氭敼鍘熷浘
    logInfo("set ROI red -> shallow = src, ROI 鍏变韩鍘熷浘鍐呭瓨");
    dbgPixel("src@ROI", src, 130, 130);     // (0,0,255)
    dbgPixel("deep@ROI", deep, 130, 130);   // 鍘熷鍊?    dbgPixel("copyto@ROI", copyto, 130, 130); // 鍘熷鍊?    dbgPixel("shallow@ROI", shallow, 130, 130); // (0,0,255)

    dbgShowMany({"src(ROI red)", "deep", "copyto"},
                {src, deep, copyto}, 0);
    return 0;
}
