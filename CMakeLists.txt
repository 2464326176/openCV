cmake_minimum_required(VERSION 3.24)
project(openCv VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
#  Unified build convention (this file is the single source of truth; all
#  sub-directories follow it and no longer maintain their own GLOB logic)
#
#  1) Auto-discovery: the root CMake scans every *.cpp in the repo and groups
#     them by top-level directory. Any new top-level demo dir (with *.cpp +
#     int main) is compiled automatically without editing this file.
#  2) Artifact naming: each *.cpp containing int main -> one executable whose
#     on-disk name equals the .cpp basename (OUTPUT_NAME = stem). The CMake
#     target name is <group>_<stem> to stay globally unique.
#  3) Output dir: build/<group>/  (the root main target goes to build/main/).
#  4) Duplicate handling: two .cpp files with the same stem inside one group
#     trigger a FATAL_ERROR asking you to rename for uniqueness.
#     Same stem across different groups (different build sub-dirs) is allowed,
#     disambiguated at the target level by the <group>_ prefix.
#  5) Resource paths: at runtime the exe lives in build/<group>/, which is
#     always ../../ relative to the source root, so LEARN_*/ALGO_* macros are
#     defined as relative paths (no hardcoded absolute paths / drive letters).
#  6) Shared sources: *.cpp without main in a group are auto-aggregated into
#     that group's static lib demo_<group>_common; the common/ dir becomes the
#     global shared lib opencv_demo_common (linked by every executable).
# ===========================================================================

# Resource relative path: every build/<group>/ is ../../ relative to the source root (build lives inside the source tree)
file(RELATIVE_PATH _RES_REL "${CMAKE_BINARY_DIR}/learn" "${CMAKE_SOURCE_DIR}")
set(_PROJ_ROOT   "${_RES_REL}/")
set(_DATA_ROOT  "${_RES_REL}/data")
set(_MODELS_ROOT "${_RES_REL}/models")

# ---- Global shared lib: common/opencv_utils.cpp (provides getProjectRoot/getImagePath, etc.) ----
add_library(opencv_demo_common STATIC ${CMAKE_SOURCE_DIR}/common/opencv_utils.cpp)
target_include_directories(opencv_demo_common PUBLIC
    ${CMAKE_SOURCE_DIR}/common ${OpenCV_INCLUDE_DIRS})
target_link_libraries(opencv_demo_common PUBLIC ${OpenCV_LIBS})
target_compile_definitions(opencv_demo_common PUBLIC
    LEARN_PROJECT_ROOT="${_PROJ_ROOT}"
    LEARN_DATA_ROOT="${_DATA_ROOT}"
    LEARN_MODELS_ROOT="${_MODELS_ROOT}")

# Collect all sub-directories (incl. nested) of a dir for target_include_directories
function(demo_collect_incs OUT_VAR BASE_DIR)
    file(GLOB_RECURSE _entries LIST_DIRECTORIES true "${BASE_DIR}/*")
    set(_incs "${CMAKE_SOURCE_DIR}/common")
    list(APPEND _incs ${OpenCV_INCLUDE_DIRS})
    foreach(_e ${_entries})
        if(IS_DIRECTORY "${_e}")
            list(APPEND _incs "${_e}")
        endif()
    endforeach()
    set(${OUT_VAR} "${_incs}" PARENT_SCOPE)
endfunction()

# Generic: process one group (recursively scans the whole group by default;
# SUBDIRS can restrict the scan to specific sub-ranges)
#   demo_process_group(<group_dir> <group_name> <cxx_std> [SUBDIRS dir1 dir2 ...])
function(demo_process_group GROUP_DIR GROUP_NAME STD)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SUBDIRS)
    cmake_parse_arguments(DPG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_cpps "")
    if(DPG_SUBDIRS)
        foreach(_sd ${DPG_SUBDIRS})
            file(GLOB_RECURSE _found "${GROUP_DIR}/${_sd}/*.cpp")
            list(APPEND _cpps ${_found})
        endforeach()
    else()
        file(GLOB_RECURSE _cpps "${GROUP_DIR}/*.cpp")
    endif()

    # Classify: *.cpp with int main -> executable; otherwise -> shared source
    set(_exes "")
    set(_shared "")
    foreach(_f ${_cpps})
        file(READ "${_f}" _c)
        if(_c MATCHES "int[ \t\r\n]+main[ \t\r\n]*\\(")
            list(APPEND _exes "${_f}")
        else()
            list(APPEND _shared "${_f}")
        endif()
    endforeach()

    # Group shared lib (sources without main)
    set(_grp_lib "")
    if(_shared)
        add_library(demo_${GROUP_NAME}_common STATIC ${_shared})
        target_include_directories(demo_${GROUP_NAME}_common PUBLIC
            ${GROUP_DIR} ${CMAKE_SOURCE_DIR}/common ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(demo_${GROUP_NAME}_common PUBLIC ${OpenCV_LIBS} opencv_demo_common)
        target_compile_definitions(demo_${GROUP_NAME}_common PUBLIC
            LEARN_PROJECT_ROOT="${_PROJ_ROOT}"
            LEARN_DATA_ROOT="${_DATA_ROOT}"
            LEARN_MODELS_ROOT="${_MODELS_ROOT}")
        set_target_properties(demo_${GROUP_NAME}_common PROPERTIES
            CXX_STANDARD ${STD} CXX_STANDARD_REQUIRED ON)
        set(_grp_lib demo_${GROUP_NAME}_common)
    endif()

    demo_collect_incs(_incs "${GROUP_DIR}")

    set(_used "")
    foreach(_f ${_exes})
        get_filename_component(_stem "${_f}" NAME_WE)
        if(_stem IN_LIST _used)
            message(FATAL_ERROR
                "Duplicate target stem '${_stem}' in group '${GROUP_NAME}'. "
                "Rename one source file so stems are unique within the group: ${_f}")
        endif()
        list(APPEND _used "${_stem}")

        set(_tgt "${GROUP_NAME}_${_stem}")
        add_executable(${_tgt} "${_f}")
        set_target_properties(${_tgt} PROPERTIES
            CXX_STANDARD ${STD} CXX_STANDARD_REQUIRED ON
            OUTPUT_NAME "${_stem}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${GROUP_NAME}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${GROUP_NAME}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${GROUP_NAME}"
            DEBUG_POSTFIX "")
        target_include_directories(${_tgt} PRIVATE ${_incs})
        target_link_libraries(${_tgt} PRIVATE ${_grp_lib} opencv_demo_common ${OpenCV_LIBS})
        target_compile_definitions(${_tgt} PRIVATE
            LEARN_PROJECT_ROOT="${_PROJ_ROOT}"
            LEARN_DATA_ROOT="${_DATA_ROOT}"
            LEARN_MODELS_ROOT="${_MODELS_ROOT}"
            ALGO_PROJECT_ROOT="${_PROJ_ROOT}"
            ALGO_DATA_ROOT="${_DATA_ROOT}"
            ALGO_MODELS_ROOT="${_MODELS_ROOT}")
        if(MSVC)
            target_compile_options(${_tgt} PRIVATE /W3 /permissive-)
        else()
            target_compile_options(${_tgt} PRIVATE -Wall -Wextra -Wno-unused-parameter)
        endif()
        message(STATUS "[${GROUP_NAME}] + exe ${_stem} <- ${_f}")
    endforeach()
    message(STATUS "[${GROUP_NAME}] built exes (${GROUP_NAME}): ${_exes}")
endfunction()

# ---- Build switches (BUILD_*) ----
# Known groups keep explicit switches; any future new dir auto-generates a
# BUILD_<GROUP> switch (default OFF).
option(BUILD_MAIN "Build root openCv main executable" ON)
option(BUILD_LEARN "Build learn layered exercises" OFF)
option(BUILD_ALGORITHMS "Build algorithms demo set" OFF)
option(BUILD_NOTES "Build notes/image_process exercises" OFF)

# Fine-grained switches for learn / algorithms (originally in sub-CMakeLists,
# promoted to the root so auto-discovery can use them)
set(LEARN_LAYER "ALL" CACHE STRING "Which layer to build: L0,L1,L2,L3,L4,L5 or ALL")
set_property(CACHE LEARN_LAYER PROPERTY STRINGS ALL L0 L1 L2 L3 L4 L5)
set(ALGO_MODULE "hdr;denoise_single" CACHE STRING
    "Algorithms modules to build (ALL or semicolon-separated list)")
set_property(CACHE ALGO_MODULE PROPERTY STRINGS
    ALL denoise_single denoise_multi hdr night_scene beauty watermark
    edge_detection morphology segmentation feature_detection stereo
    deblur template_matching inpaint hough_transform frequency_domain optical_flow)

# ---- Root main target: openCv.cpp (formerly main.cpp) ----
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
        target_include_directories(openCv PRIVATE ${CMAKE_SOURCE_DIR}/common ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(openCv PRIVATE opencv_demo_common ${OpenCV_LIBS})
        target_compile_definitions(openCv PRIVATE
            LEARN_PROJECT_ROOT="${_PROJ_ROOT}"
            LEARN_DATA_ROOT="${_DATA_ROOT}"
            LEARN_MODELS_ROOT="${_MODELS_ROOT}")
        if(NOT MSVC)
            target_compile_options(openCv PRIVATE -Wall -Wextra -Wno-unused-parameter)
        endif()
        message(STATUS "[main] + exe openCv <- ${_root_src}")
    else()
        message(WARNING "BUILD_MAIN=ON but openCv.cpp not found")
    endif()
endif()

# ---- learn: layer filtering (LEARN_LAYER) ----
if(BUILD_LEARN)
    set(_all_layers L0_intro L1_core L2_imgproc L3_features_video L4_detect_calib L5_ml_gapi)
    if(LEARN_LAYER STREQUAL "ALL")
        set(_active ${_all_layers})
    else()
        set(_map_L0 L0_intro)
        set(_map_L1 L1_core)
        set(_map_L2 L2_imgproc)
        set(_map_L3 L3_features_video)
        set(_map_L4 L4_detect_calib)
        set(_map_L5 L5_ml_gapi)
        set(_active ${_map_${LEARN_LAYER}})
    endif()
    message(STATUS "learn: LEARN_LAYER=${LEARN_LAYER} active=${_active}")
    demo_process_group(${CMAKE_SOURCE_DIR}/learn learn 11 SUBDIRS ${_active})
endif()

# ---- algorithms: module filtering (ALGO_MODULE) + fixed shared lib ----
if(BUILD_ALGORITHMS)
    set(_all_modules denoise_single denoise_multi hdr night_scene beauty watermark
        edge_detection morphology segmentation feature_detection stereo
        deblur template_matching inpaint hough_transform frequency_domain optical_flow)
    if(ALGO_MODULE STREQUAL "ALL")
        set(_active ${_all_modules})
    else()
        set(_active ${ALGO_MODULE})
    endif()
    message(STATUS "[algorithms] ALGO_MODULE=${ALGO_MODULE} active=${_active}")

    # Fixed shared lib: common/* + hdr/hdr_pipeline.cpp (C++17)
    set(_algo_shared
        ${CMAKE_SOURCE_DIR}/algorithms/common/nv21_io.cpp
        ${CMAKE_SOURCE_DIR}/algorithms/common/algo_utils.cpp
        ${CMAKE_SOURCE_DIR}/algorithms/common/single_denoise.cpp
        ${CMAKE_SOURCE_DIR}/algorithms/hdr/hdr_pipeline.cpp)
    add_library(demo_algorithms_common STATIC ${_algo_shared})
    target_include_directories(demo_algorithms_common PUBLIC
        ${CMAKE_SOURCE_DIR}/algorithms/common
        ${CMAKE_SOURCE_DIR}/algorithms/hdr
        ${CMAKE_SOURCE_DIR}/common ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(demo_algorithms_common PUBLIC ${OpenCV_LIBS} opencv_demo_common)
    target_compile_definitions(demo_algorithms_common PUBLIC
        LEARN_PROJECT_ROOT="${_PROJ_ROOT}" LEARN_DATA_ROOT="${_DATA_ROOT}" LEARN_MODELS_ROOT="${_MODELS_ROOT}"
        ALGO_PROJECT_ROOT="${_PROJ_ROOT}" ALGO_DATA_ROOT="${_DATA_ROOT}" ALGO_MODELS_ROOT="${_MODELS_ROOT}")
    set_target_properties(demo_algorithms_common PROPERTIES
        CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

    demo_collect_incs(_algo_incs "${CMAKE_SOURCE_DIR}/algorithms")
    set(_used "")
    foreach(_m ${_active})
        set(_src "${CMAKE_SOURCE_DIR}/algorithms/${_m}/${_m}.cpp")
        if(NOT EXISTS "${_src}")
            message(WARNING "[algorithms] module ${_m} has no ${_m}.cpp, skip")
            continue()
        endif()
        set(_stem "${_m}")
        if(_stem IN_LIST _used)
            message(FATAL_ERROR "Duplicate algorithm module '${_stem}'")
        endif()
        list(APPEND _used "${_stem}")
        set(_tgt "algorithms_${_stem}")
        add_executable(${_tgt} "${_src}")
        set_target_properties(${_tgt} PROPERTIES
            CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON
            OUTPUT_NAME "${_stem}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/algorithms"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/algorithms"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/algorithms"
            DEBUG_POSTFIX "")
        target_include_directories(${_tgt} PRIVATE ${_algo_incs})
        target_link_libraries(${_tgt} PRIVATE demo_algorithms_common opencv_demo_common ${OpenCV_LIBS})
        target_compile_definitions(${_tgt} PRIVATE
            LEARN_PROJECT_ROOT="${_PROJ_ROOT}" LEARN_DATA_ROOT="${_DATA_ROOT}" LEARN_MODELS_ROOT="${_MODELS_ROOT}"
            ALGO_PROJECT_ROOT="${_PROJ_ROOT}" ALGO_DATA_ROOT="${_DATA_ROOT}" ALGO_MODELS_ROOT="${_MODELS_ROOT}")
        if(MSVC)
            target_compile_options(${_tgt} PRIVATE /W3 /permissive-)
        else()
            target_compile_options(${_tgt} PRIVATE -Wall -Wextra -Wno-unused-parameter)
        endif()
        message(STATUS "[algorithms] + exe ${_stem} <- ${_src}")
    endforeach()
endif()

# ---- notes: generic whole-group auto-discovery (C++14) ----
if(BUILD_NOTES)
    demo_process_group(${CMAKE_SOURCE_DIR}/notes notes 14)
endif()

# ---- Future new dirs: auto-discovery, zero CMake edits ----
# Any top-level dir not handled explicitly above is treated as a group and
# compiled automatically as long as it contains *.cpp.
# The switch defaults to OFF; enable with -DBUILD_<GROUP>=ON.
set(_EXCLUDE_DIRS build .git docs images models mingw-build)
file(GLOB _top LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/*)
foreach(_e ${_top})
    if(NOT IS_DIRECTORY "${_e}")
        continue()
    endif()
    get_filename_component(_grp "${_e}" NAME)
    if(_grp IN_LIST _EXCLUDE_DIRS)
        continue()
    endif()
    if(_grp STREQUAL "common" OR _grp STREQUAL "learn"
       OR _grp STREQUAL "algorithms" OR _grp STREQUAL "notes")
        continue()
    endif()
    string(TOUPPER "${_grp}" _grp_up)
    option(BUILD_${_grp_up} "Build ${_grp} group" OFF)
    if(BUILD_${_grp_up})
        message(STATUS "[auto] discovered group '${_grp}' (BUILD_${_grp_up}=ON)")
        demo_process_group("${_e}" "${_grp}" 11)
    else()
        message(STATUS "[auto] group '${_grp}' found but BUILD_${_grp_up}=OFF (skip; enable with -DBUILD_${_grp_up}=ON)")
    endif()
endforeach()
