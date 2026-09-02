# algorithms/CMakeLists.txt
# 常用图像算法 demo 集合:
#   - common/        静态库 algo_common: NV21 IO / 评估指标 / 单帧降噪算法库 / HDR pipeline
#   - denoise_single 单帧降噪: 4 类噪声 × 11 种算法 × 参数扫描
#   - denoise_multi  多帧降噪: ECC/Affine/Homography 配准 + 4 种融合策略
#   - hdr            HDR: 2 种 CRF + 2 种 Merge + 7 种 Tonemap + Mertens 曝光融合
#   - night_scene    夜景: 10+ 增强算法 (LIME/RetinexTV/暗通道/MSRCR 等) + NR-IQA
#   - beauty         美颜: 三融合皮肤mask/自适应强度/频率分离磨皮/五官保留
#   - watermark      水印: 可见 3 类 + DFT 非盲 + DCT QIM 盲提取 + 30+ 鲁棒性攻击
#   - edge_detection 边缘检测: Sobel/Scharr/Prewitt/Laplacian/LoG/DoG/Canny(+Otsu自动阈值)
#   - morphology     形态学: 腐蚀/膨胀/开闭/形态梯度/顶帽黑帽/hit-or-miss + 结构元素与核大小对比
#   - segmentation   分割: Otsu/自适应阈值/KMeans量化/MeanShift/GrabCut/Watershed/连通域
#   - feature_detection 特征检测与匹配: Harris/Shi-Tomasi/FAST/ORB/BRISK/AKAZE + 单应性匹配
#   - stereo         立体匹配: StereoBM / SGBM 视差估计 + 伪彩与无参考统计
#   - deblur         去模糊: 合成运动/散焦退化 + Wiener/RL 反卷积 + 锐化基线
#   - template_matching 模板匹配: 全部 TM_ 方法 + 多尺度定位
#   - inpaint        图像修复: Telea / Navier-Stokes 划痕文本去除
#   - hough_transform 霍夫变换: HoughLinesP 直线 + HoughCircles 圆
#   - frequency_domain 频域: DFT 幅值谱 + 低通/高通/陷波带阻去周期噪声
#   - optical_flow   光流: Lucas-Kanade 稀疏 + Farneback 稠密速度场
#
# 构建开关 (在根 cmake 调用时传入):
#   cmake -DBUILD_ALGORITHMS=ON -DALGO_MODULE=ALL ..
#   cmake -DBUILD_ALGORITHMS=ON -DALGO_MODULE=hdr ..
# ALGO_MODULE 默认仅构建 hdr + denoise_single (HDR 与单帧降噪相对独立、可一次跑通)

cmake_minimum_required(VERSION 3.15)

# ------------------ 配置: 输出目录 & 数据路径 -------------------------------
get_filename_component(ALGO_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(ALGO_DATA_ROOT   "${ALGO_PROJECT_ROOT}/data")
set(ALGO_MODELS_ROOT "${ALGO_PROJECT_ROOT}/models")

# ------------------ 公共静态库 algo_common --------------------------------
# 所有子模块的 main 都链接到该静态库, 避免重复编译公共工具
set(ALGO_COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/nv21_io.cpp          # YUV420 (NV21/NV12/I420) IO + 元数据解析
    ${CMAKE_CURRENT_SOURCE_DIR}/common/algo_utils.cpp       # 评估指标(PSNR/SSIM/MAE/MSE/MS-SSIM/LOE/NIQE)+色彩统计+增强+配准+可视化
    ${CMAKE_CURRENT_SOURCE_DIR}/common/single_denoise.cpp   # 11 种单帧降噪 + 4 种噪声合成
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/hdr_pipeline.cpp        # CRF/Merge/7 Tonemap/Mertens 融合
)
set(ALGO_COMMON_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/common/nv21_io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/algo_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/single_denoise.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr/hdr_pipeline.hpp
)
add_library(algo_common STATIC ${ALGO_COMMON_SOURCES} ${ALGO_COMMON_HEADERS})
target_include_directories(algo_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/hdr
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${OpenCV_INCLUDE_DIRS})
target_link_libraries(algo_common PUBLIC ${OpenCV_LIBS})

# C++ 标准: 使用 C++17, 允许 std::string_view / structured bindings / if constexpr 等
target_compile_features(algo_common PUBLIC cxx_std_17)
set_target_properties(algo_common PROPERTIES CXX_EXTENSIONS OFF)

# 适度警告 (MSVC/GCC/Clang 分别处理)
if(MSVC)
    target_compile_options(algo_common PRIVATE /W3 /permissive-)
else()
    target_compile_options(algo_common PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

target_compile_definitions(algo_common PUBLIC
    ALGO_PROJECT_ROOT="${ALGO_PROJECT_ROOT}/"
    ALGO_DATA_ROOT="${ALGO_DATA_ROOT}"
    ALGO_MODELS_ROOT="${ALGO_MODELS_ROOT}")

# ------------------ 模块开关 -----------------------------------------------
set(ALGO_MODULE "hdr;denoise_single" CACHE STRING
    "Modules to build. Choices: ALL or semicolon-separated list: denoise_single;denoise_multi;hdr;night_scene;beauty;watermark;edge_detection;morphology;segmentation;feature_detection;stereo;deblur;template_matching;inpaint;hough_transform;frequency_domain;optical_flow")
set_property(CACHE ALGO_MODULE PROPERTY STRINGS
    ALL denoise_single denoise_multi hdr night_scene beauty watermark
        edge_detection morphology segmentation feature_detection stereo
        deblur template_matching inpaint hough_transform frequency_domain optical_flow)

set(_all_modules
    denoise_single denoise_multi hdr night_scene beauty watermark
    edge_detection morphology segmentation feature_detection stereo
    deblur template_matching inpaint hough_transform frequency_domain optical_flow)
if(ALGO_MODULE STREQUAL "ALL")
    set(_active ${_all_modules})
else()
    foreach(m ${ALGO_MODULE})
        list(APPEND _active ${m})
    endforeach()
endif()
message(STATUS "[algorithms] ALGO_MODULE='${ALGO_MODULE}', active='${_active}'")
message(STATUS "[algorithms] data   root = ${ALGO_DATA_ROOT}")
message(STATUS "[algorithms] models root = ${ALGO_MODELS_ROOT}")

set(_runtime_dir "${CMAKE_BINARY_DIR}/algorithms")
file(MAKE_DIRECTORY "${_runtime_dir}")

# 输出目录也预先创建, 避免各 exe 第一次运行时 ensureDir 报错 (保险)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/out/algorithms")

# ------------------ 每个模块一个独立可执行 ----------------------------------
# 约定: 子目录下有 main.cpp 即构建 algo_<module> 可执行
set(_modules_with_src "")
foreach(_m ${_active})
    set(_src ${CMAKE_CURRENT_SOURCE_DIR}/${_m}/main.cpp)
    if(NOT EXISTS ${_src})
        message(WARNING "[algorithms] ${_m} has no main.cpp, skip")
        continue()
    endif()
    add_executable(algo_${_m} ${_src})
    target_link_libraries(algo_${_m} PRIVATE algo_common)
    target_include_directories(algo_${_m} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/${_m})
    # 继承 C++17 标准 (algo_common 已声明 PUBLIC, 此处保险)
    target_compile_features(algo_${_m} PRIVATE cxx_std_17)
    # 可执行输出目录: 无论 Debug/Release 都落到同一目录, 方便 ../data / ../out 相对路径
    set_target_properties(algo_${_m} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY         "${_runtime_dir}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${_runtime_dir}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_runtime_dir}"
        # Windows 下避免生成 Debug 加 _d 后缀的 exe 名
        DEBUG_POSTFIX ""
    )
    if(MSVC)
        target_compile_options(algo_${_m} PRIVATE /W3 /permissive-)
    else()
        target_compile_options(algo_${_m} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    endif()
    list(APPEND _modules_with_src ${_m})
endforeach()

message(STATUS "[algorithms] built targets = ${_modules_with_src}")
