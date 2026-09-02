// LEARN: L4 solvePnP 浣嶅Э浼拌
// OFFICIAL: tutorial_code/calib3d/real_time_pose_estimation/src/main_detection.cpp
// THEORY: docs/ch07_calib3d_stitching.md 搂11
// TASK: 鐢ㄥ悎鎴?4 涓?3D 鐐?+ 鎶曞奖寰楀埌鐨?2D 鐐硅窇 solvePnP 寰?R/t锛屽啀 projectPoints 楠岃瘉閲嶆姇褰?#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv_utils.h>

using namespace cv;

int main() {
    // 1) 鍚堟垚 3D 鐗╀綋鐐癸紙涓€涓?0.1m 绔嬫柟浣撶殑 4 涓《鐐癸級
    std::vector<Point3f> objPts = {
        {0, 0, 0}, {0.1f, 0, 0}, {0.1f, 0.1f, 0}, {0, 0.1f, 0}};

    // 2) 宸茬煡鍐呭弬 K + 澶栧弬鐪熷€?    Mat K = (Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
    Mat Rvec = (Mat_<double>(3, 1) << 0.1, 0.2, 0.05);
    Mat tvec = (Mat_<double>(3, 1) << 0.0, 0.0, 0.6);

    // 3) 鐢ㄧ湡鍊兼姇褰辩敓鎴?2D 鍍忕礌鐐?    std::vector<Point2f> imgPts;
    projectPoints(objPts, Rvec, tvec, K, noArray(), imgPts);
    logInfo("synthetic 2D projections: %zu pts", imgPts.size());

    // 4) solvePnP 浠?2D-3D 瀵瑰簲鍥炴眰澶栧弬
    Mat rvec_est, tvec_est;
    bool ok = solvePnP(objPts, imgPts, K, noArray(), rvec_est, tvec_est, false, SOLVEPNP_ITERATIVE);
    logInfo("solvePnP -> %d", ok ? 1 : 0);
    if (!ok) { logInfo("solvePnP failed"); return -1; }
    logInfo("R_true=(%.3f,%.3f,%.3f) R_est=(%.3f,%.3f,%.3f)",
            Rvec.at<double>(0), Rvec.at<double>(1), Rvec.at<double>(2),
            rvec_est.at<double>(0), rvec_est.at<double>(1), rvec_est.at<double>(2));
    logInfo("t_true=(%.3f,%.3f,%.3f) t_est=(%.3f,%.3f,%.3f)",
            tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2),
            tvec_est.at<double>(0), tvec_est.at<double>(1), tvec_est.at<double>(2));

    // 5) 閲嶆姇褰遍獙璇?    std::vector<Point2f> reproj;
    projectPoints(objPts, rvec_est, tvec_est, K, noArray(), reproj);
    double err = norm(imgPts, reproj, NORM_L2);
    logInfo("reprojection L2 error=%.6f px", err);
    return 0;
}
