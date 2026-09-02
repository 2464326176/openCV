# notes/image_process/CMakeLists.txt
# 毛星云体例学习笔记 - 图像处理基础子项目.
#
# 构建方式 A (通过根 CMake 开关 BUILD_NOTES):
#   cmake -B build -G "MinGW Makefiles" -DBUILD_MAIN=OFF -DBUILD_NOTES=ON -DCMAKE_BUILD_TYPE=Release ..
#   cmake --build build -j
#
# 构建方式 B (standalone, 直接在本子目录下):
#   cmake -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="../../mingw-build/x64/mingw/staticlib" -DCMAKE_BUILD_TYPE=Release .
#   cmake --build build -j
#
# 说明: 子目录下每个带 main() 的 .cpp 都会单独编为一个 note_imgproc_<stem>.exe 可执行文件,
#       共享 draw.h / mat.h 等头文件与 OpenCV 链接.

cmake_minimum_required(VERSION 3.15)
project(note_image_process LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 独立构建时, 允许用户用 -DCMAKE_PREFIX_PATH 或 -DOpenCV_DIR 传入 OpenCV 路径 ---
if(NOT OpenCV_FOUND)
    # 回退到项目根自带的 mingw 静态库 (若子目录相对项目根为 notes/image_process → ../../)
    if(NOT DEFINED OpenCV_DIR OR NOT IS_DIRECTORY "${OpenCV_DIR}")
        get_filename_component(_SELF_PROJ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
        set(OpenCV_DIR "${_SELF_PROJ_ROOT}/mingw-build/x64/mingw/staticlib" CACHE PATH "" FORCE)
    endif()
    find_package(OpenCV REQUIRED)
    message(STATUS "[note_imgproc] OpenCV_DIR    = ${OpenCV_DIR}")
    message(STATUS "[note_imgproc] OpenCV_VERSION = ${OpenCV_VERSION}")
    message(STATUS "[note_imgproc] OpenCV_LIBS    = ${OpenCV_LIBS}")
endif()

# --- 头文件搜索路径: 本子目录 + 项目根 common/ ---
get_filename_component(_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../common" ABSOLUTE)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/basic_drawing
    ${CMAKE_CURRENT_SOURCE_DIR}/mat
    ${CMAKE_CURRENT_SOURCE_DIR}/image
    ${CMAKE_CURRENT_SOURCE_DIR}/morphology
    ${CMAKE_CURRENT_SOURCE_DIR}/process
    ${_COMMON_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

# --- 共享库代码 (不含 main): draw.h/cpp, mat.h/cpp, common/opencv_utils.cpp ---
set(NOTE_IMGPROC_SHARED_SOURCES
    basic_drawing/draw.cpp
    mat/mat.cpp
    ${_COMMON_DIR}/opencv_utils.cpp
)
add_library(note_imgproc_shared STATIC ${NOTE_IMGPROC_SHARED_SOURCES})
target_link_libraries(note_imgproc_shared PUBLIC ${OpenCV_LIBS})
target_include_directories(note_imgproc_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/basic_drawing
    ${CMAKE_CURRENT_SOURCE_DIR}/mat
    ${_COMMON_DIR}
)

# --- 可执行目标清单 (每个带 main() 的 .cpp 单独一个 exe) ---
set(_EXEC_SOURCES
    basic_drawing/draw.cpp              # 注意: draw.cpp 可能没有 main(), 下文会自动过滤
    mat/mat1.cpp
    mat/mat_operate.cpp
    image/add_image.cpp
    image/color_reduce.cpp
    image/display_image.cpp
    image/image.cpp
    image/read_image.cpp
    image/region_of_interest.cpp
    image/split_merge.cpp
    morphology/filter.cpp
    morphology/flood_fill.cpp
    morphology/image_operation1.cpp
    morphology/image_operation2.cpp
    morphology/resize.cpp
    process/filter_process.cpp
    process/morph_op_process.cpp
)

# --- 运行目录: 所有 exe 都输出到 build/note_imgproc/, 便于相对路径访问 data/ ---
set(_RUNTIME_DIR "${CMAKE_BINARY_DIR}/note_imgproc")
file(MAKE_DIRECTORY "${_RUNTIME_DIR}")

foreach(_src ${_EXEC_SOURCES})
    # 先扫描源码里是不是真的有 "int main(" (排除 draw.cpp/mat.cpp 这种只有函数定义的共享文件)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${_src}" _src_content)
    if(NOT "${_src_content}" MATCHES "int[ \t\r\n]+main[ \t\r\n]*\\(")
        message(STATUS "[note_imgproc] SKIP (no main): ${_src}")
        continue()
    endif()
    get_filename_component(_stem ${_src} NAME_WE)
    get_filename_component(_dir  ${_src} DIRECTORY)
    # 避免同名冲突 (不同子目录可能有同名 stem, 这里用 dir 前缀取末级)
    get_filename_component(_dirtail ${_dir} NAME)
    set(_target "note_imgproc_${_dirtail}_${_stem}")

    add_executable(${_target} ${_src})
    target_link_libraries(${_target} PRIVATE note_imgproc_shared ${OpenCV_LIBS})
    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY         "${_RUNTIME_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${_RUNTIME_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_RUNTIME_DIR}"
        DEBUG_POSTFIX ""
    )
    if(NOT MSVC)
        target_compile_options(${_target} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    endif()
    list(APPEND _built_targets "${_target}")
endforeach()

message(STATUS "[note_imgproc] ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "[note_imgproc] built exe count  = ${_built_targets}")
message(STATUS "[note_imgproc] runtime dir       = ${_RUNTIME_DIR}")
