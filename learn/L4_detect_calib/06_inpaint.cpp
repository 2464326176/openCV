// LEARN: L4 鍥惧儚淇 inpaint
// OFFICIAL: samples/cpp/inpaint.cpp
// THEORY: docs/ch06_objdetect_photo.md 搂6.22
// TASK: 鍦?lena 涓儴鐢讳竴鏉＄櫧鑹叉í绾夸綔涓哄緟淇鍖哄煙锛岀敤 INPAINT_TELEA 涓?INPAINT_NS 涓ょ绠楁硶鍒嗗埆淇瀵规瘮
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    Mat img = imread(getImagePath("lena.jpg"));
    if (img.empty()) { logInfo("imread failed: lena.jpg"); return -1; }
    dbgMatInfo("img", img);

    // 1) 鏋勯€犳崯鍧?mask锛氫腑闂翠竴鏉℃í鍚戠櫧绾?+ 鍙充笂瑙掓柟鍧?    Mat mask(img.size(), CV_8UC1, Scalar(0));
    rectangle(mask, Rect(0, img.rows / 2 - 4, img.cols, 8), Scalar(255), -1);
    rectangle(mask, Rect(img.cols - 80, 10, 70, 70), Scalar(255), -1);

    Mat damaged;
    img.copyTo(damaged, mask.inv());     // mask 澶勫彉榛?    // 鎶?mask 澶勭敾涓婂櫔鐐癸紙妯℃嫙鎹熷潖锛夛紝渚夸簬鐪嬩慨澶嶆晥鏋?    damaged.setTo(Scalar(255, 255, 255), mask);

    // 2) 涓ょ淇绠楁硶瀵规瘮
    Mat telea, ns;
    inpaint(damaged, mask, telea, 5, INPAINT_TELEA);
    inpaint(damaged, mask, ns, 5, INPAINT_NS);
    logInfo("INPAINT_TELEA done; INPAINT_NS done");

    dbgShowMany({"L4_06 damaged", "L4_06 telea", "L4_06 ns"},
                {damaged, telea, ns}, 0);
    return 0;
}
