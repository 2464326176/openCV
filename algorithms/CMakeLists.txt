# ============================================================================
#  algorithms/CMakeLists.txt - Algorithm demo modules
#
#  Module filtering via ALGO_MODULE cache variable (set in root):
#    ALL                               - build all modules
#    "hdr;denoise_single"              - selected modules only (default)
#
#  All exe boilerplate (includes/links/defines/warnings/output dir) is
#  applied by the root add_demo_exe() helper; the aggregate custom target
#  "algorithms" (created in the root) builds every module at once.
# ============================================================================

# ---- Module filtering ----
set(_all_modules
    denoise_single denoise_multi hdr night_scene beauty watermark
    edge_detection morphology segmentation feature_detection stereo
    deblur template_matching inpaint hough_transform frequency_domain
    optical_flow sharpen demosaic color_transfer)

if(ALGO_MODULE STREQUAL "ALL")
    set(_active ${_all_modules})
else()
    set(_active ${ALGO_MODULE})
endif()

list(LENGTH _active _n)
message(STATUS "[algorithms] ALGO_MODULE=${ALGO_MODULE} active=${_active} (${_n} modules)")

# ---- Shared library: common utilities + hdr_pipeline ----
# Its PUBLIC compile definitions (LEARN_*/ALGO_* roots) propagate to every
# algorithm executable linked against it, so no per-target DEFS are needed.
add_library(demo_algorithms_common STATIC
    ${CMAKE_SOURCE_DIR}/algorithms/common/nv21_io.cpp
    ${CMAKE_SOURCE_DIR}/algorithms/common/algo_utils.cpp
    ${CMAKE_SOURCE_DIR}/algorithms/common/single_denoise.cpp
    ${CMAKE_SOURCE_DIR}/algorithms/hdr/hdr_pipeline.cpp)

target_include_directories(demo_algorithms_common PUBLIC
    ${CMAKE_SOURCE_DIR}/algorithms/common
    ${CMAKE_SOURCE_DIR}/algorithms/hdr
    ${CMAKE_SOURCE_DIR}/common
    ${OpenCV_INCLUDE_DIRS})
target_link_libraries(demo_algorithms_common PUBLIC ${OpenCV_LIBS} opencv_demo_common)
target_compile_definitions(demo_algorithms_common PUBLIC
    LEARN_PROJECT_ROOT="${PROJ_ROOT}" LEARN_DATA_ROOT="${DATA_ROOT}" LEARN_MODELS_ROOT="${MODELS_ROOT}"
    ALGO_PROJECT_ROOT="${PROJ_ROOT}" ALGO_DATA_ROOT="${DATA_ROOT}" ALGO_MODELS_ROOT="${MODELS_ROOT}")
set_target_properties(demo_algorithms_common PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

# ---- Module include dirs (one level: algorithms/<module>/) ----
file(GLOB _algo_dirs LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")
set(_algo_incs)
foreach(_e ${_algo_dirs})
    if(IS_DIRECTORY "${_e}")
        list(APPEND _algo_incs "${_e}")
    endif()
endforeach()

# ---- Module executables ----
foreach(_m ${_active})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_m}/${_m}.cpp")
        message(WARNING "[algorithms] module ${_m} has no ${_m}.cpp, skip")
        continue()
    endif()
    add_demo_exe(algorithms_${_m} "${_m}/${_m}.cpp" 17 algorithms
        LIBS demo_algorithms_common
        INCS ${_algo_incs})
endforeach()
