cmake_minimum_required(VERSION 3.24)
project(openCv VERSION 0.1.0 LANGUAGES C CXX)

# ===========================================================================
#  Quick build guide (this project has 100+ executable targets):
#
#    build.bat              # Build ALL targets in parallel (recommended)
#    build.bat --target xxx # Build a single target
#    build.bat --configure  # Reconfigure cmake
#
#  Or manually:
#    cmake --build build -j                # Parallel build (use all cores)
#    cmake --build build -j --target xxx   # Single target
#
#  To build only specific layers:
#    cmake -S . -B build -DLEARN_LAYER=L2 -DALGO_MODULE="hdr;denoise_single"
# ===========================================================================

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This repo is built against a MinGW-built OpenCV. The only MinGW build
# available here installed the static libs (x64/mingw/staticlib) and NOT the
# shared ones (x64/mingw/lib). OpenCV's top-level OpenCVConfig.cmake looks in
# .../lib by default (OpenCV_STATIC=OFF) and reports "no binaries compatible"
# when only staticlib exists. Force static so find_package() resolves the
# staticlib path. Override with -DOpenCV_STATIC=OFF if you supply a shared build.
if(NOT DEFINED OpenCV_STATIC)
    set(OpenCV_STATIC ON)
endif()

# OpenCV lookup priority (unchanged):
#   1) User-specified -DOpenCV_DIR=... (highest priority)
#   2) Bundled mingw static libs at mingw-build/x64/mingw/staticlib (if present)
#   3) Otherwise fall back to find_package to locate a system-installed OpenCV
set(_LOCAL_OPENCV "${CMAKE_CURRENT_SOURCE_DIR}/mingw-build/x64/mingw/staticlib")
if(NOT DEFINED OpenCV_DIR AND EXISTS "${_LOCAL_OPENCV}")
    set(OpenCV_DIR "${_LOCAL_OPENCV}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find OpenCV (you may set OpenCV_DIR to the dir containing OpenCVConfig.cmake)
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# ===========================================================================
#  Resource relative paths
#
#  Every build/<group>/ is ../../ relative to the source root (build lives
#  inside the source tree), so LEARN_*/ALGO_* macros are defined as relative
#  paths (no hardcoded absolute paths / drive letters).
# ===========================================================================
file(RELATIVE_PATH RES_REL "${CMAKE_BINARY_DIR}/learn" "${CMAKE_SOURCE_DIR}")
set(PROJ_ROOT   "${RES_REL}/")
set(DATA_ROOT  "${RES_REL}/data")
set(MODELS_ROOT "${RES_REL}/models")

# ===========================================================================
#  Global shared lib: common/opencv_utils.cpp
#  Provides getProjectRoot / getImagePath / dbg* utilities for all targets.
# ===========================================================================
add_library(opencv_demo_common STATIC ${CMAKE_SOURCE_DIR}/common/opencv_utils.cpp)
target_include_directories(opencv_demo_common PUBLIC
    ${CMAKE_SOURCE_DIR}/common ${OpenCV_INCLUDE_DIRS})
target_link_libraries(opencv_demo_common PUBLIC ${OpenCV_LIBS})
target_compile_definitions(opencv_demo_common PUBLIC
    LEARN_PROJECT_ROOT="${PROJ_ROOT}"
    LEARN_DATA_ROOT="${DATA_ROOT}"
    LEARN_MODELS_ROOT="${MODELS_ROOT}")

# ===========================================================================
#  Helper: create a single executable from a .cpp with common settings
#
#  Usage in sub-CMakeLists.txt:
#    add_demo_exe(my_target path/to/src.cpp CXX_STD OUTPUT_DIR [EXTRA_DEFS ...])
#
#  The target name uses the full qualified name (e.g. learn_01_hello_imread)
#  while OUTPUT_NAME is set to just the stem. RUNTIME_OUTPUT_DIRECTORY goes
#  to build/<OUTPUT_DIR>/.
# ===========================================================================
function(add_demo_exe TARGET_NAME SOURCE_FILE CXX_STD OUTPUT_DIR)
    add_executable(${TARGET_NAME} "${SOURCE_FILE}")
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD ${CXX_STD} CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}"
        DEBUG_POSTFIX "")
    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/common
        ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} PRIVATE opencv_demo_common ${OpenCV_LIBS})
    target_compile_definitions(${TARGET_NAME} PRIVATE
        LEARN_PROJECT_ROOT="${PROJ_ROOT}"
        LEARN_DATA_ROOT="${DATA_ROOT}"
        LEARN_MODELS_ROOT="${MODELS_ROOT}"
        ${ARGN})
    if(NOT MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    endif()
    message(STATUS "[${OUTPUT_DIR}] + exe ${TARGET_NAME} <- ${SOURCE_FILE}")
endfunction()

# ===========================================================================
#  Build switches
# ===========================================================================
option(BUILD_MAIN "Build root openCv main executable" ON)
option(BUILD_LEARN "Build learn layered exercises" ON)
option(BUILD_ALGORITHMS "Build algorithms demo set" ON)
option(BUILD_NOTES "Build notes/image_process exercises" ON)

# Fine-grained switches
set(LEARN_LAYER "ALL" CACHE STRING "Which layer to build: L0,L1,L2,L3,L4,L5 or ALL")
set_property(CACHE LEARN_LAYER PROPERTY STRINGS ALL L0 L1 L2 L3 L4 L5)

set(ALGO_MODULE "hdr;denoise_single" CACHE STRING
    "Algorithms modules to build (ALL or semicolon-separated list)")
set_property(CACHE ALGO_MODULE PROPERTY STRINGS
    ALL denoise_single denoise_multi hdr night_scene beauty watermark
    edge_detection morphology segmentation feature_detection stereo
    deblur template_matching inpaint hough_transform frequency_domain optical_flow)

# ===========================================================================
#  Root main target: openCv.cpp
# ===========================================================================
if(BUILD_MAIN)
    set(_root_src "${CMAKE_SOURCE_DIR}/openCv.cpp")
    if(EXISTS "${_root_src}")
        add_executable(openCv "${_root_src}")
        set_target_properties(openCv PROPERTIES
            CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/main"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/main"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/main"
            DEBUG_POSTFIX "")
        target_include_directories(openCv PRIVATE
            ${CMAKE_SOURCE_DIR}/common ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(openCv PRIVATE opencv_demo_common ${OpenCV_LIBS})
        target_compile_definitions(openCv PRIVATE
            LEARN_PROJECT_ROOT="${PROJ_ROOT}"
            LEARN_DATA_ROOT="${DATA_ROOT}"
            LEARN_MODELS_ROOT="${MODELS_ROOT}")
        if(NOT MSVC)
            target_compile_options(openCv PRIVATE -Wall -Wextra -Wno-unused-parameter)
        endif()
        message(STATUS "[main] + exe openCv <- ${_root_src}")
    else()
        message(WARNING "BUILD_MAIN=ON but openCv.cpp not found")
    endif()
endif()

# ===========================================================================
#  Sub-projects: each group has its own CMakeLists.txt with explicit sources
# ===========================================================================
if(BUILD_LEARN)
    add_subdirectory(learn)
endif()

if(BUILD_ALGORITHMS)
    add_subdirectory(algorithms)
endif()

if(BUILD_NOTES)
    add_subdirectory(notes)
endif()