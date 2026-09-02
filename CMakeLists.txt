cmake_minimum_required(VERSION 3.24)
project(openCv VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

set(OpenCV_DIR mingw-build/x64/mingw/staticlib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/common SRC_LIST)

# Declare the executable target built from your sources.
# 默认构建 openCv 主可执行; 若只需要构建 learn / algorithms 子项目,
# 可关闭 -DBUILD_MAIN=OFF 跳过此目标.
option(BUILD_MAIN "Build the legacy openCv executable" ON)
if(BUILD_MAIN)
    add_executable(openCv ${SRC_LIST}
            notes/image_process/morphology/resize.cpp)
    target_link_libraries(openCv PRIVATE ${OpenCV_LIBS})
endif()

# ---- learn: 分层练习 (默认关闭) ----
# 启用后 add_subdirectory(learn), 通过 LEARN_LAYER 控制只编译一层.
option(BUILD_LEARN "Build learn layered exercises" OFF)
if(BUILD_LEARN)
    add_subdirectory(learn)
endif()

# ---- algorithms: 常用算法 demo (单/多帧降噪, HDR, 夜景, 美颜, 水印) ----
# 启用后 add_subdirectory(algorithms), 通过 ALGO_MODULE 控制只编译部分模块.
option(BUILD_ALGORITHMS "Build algorithms demo set" OFF)
if(BUILD_ALGORITHMS)
    add_subdirectory(algorithms)
endif()

# ---- notes: 毛星云体例学习笔记 (legacy image_process 子项目) ----
# 启用后 add_subdirectory(notes/image_process), 每个带 main 的 .cpp 单独一个 exe.
option(BUILD_NOTES "Build legacy notes/image_process exercises" OFF)
if(BUILD_NOTES)
    add_subdirectory(notes/image_process)
endif()

