# ============================================================================
#  algorithms/CMakeLists.txt - Algorithm demo modules
#
#  Module filtering via ALGO_MODULE cache variable (set in root):
#    ALL                               - build all modules
#    "hdr;denoise_single"              - selected modules only (default)
# ============================================================================

# ---- Module filtering ----
set(_all_modules
    denoise_single denoise_multi hdr night_scene beauty watermark
    edge_detection morphology segmentation feature_detection stereo
    deblur template_matching inpaint hough_transform frequency_domain optical_flow)

if(ALGO_MODULE STREQUAL "ALL")
    set(_active ${_all_modules})
else()
    set(_active ${ALGO_MODULE})
endif()

message(STATUS "[algorithms] ALGO_MODULE=${ALGO_MODULE} active=${_active}")

# ---- Shared library: common utilities + hdr_pipeline ----
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

# ---- Collect include directories for algorithm targets ----
file(GLOB_RECURSE _algo_entries LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")
set(_algo_incs "${CMAKE_SOURCE_DIR}/common" ${OpenCV_INCLUDE_DIRS})
foreach(_e ${_algo_entries})
    if(IS_DIRECTORY "${_e}")
        list(APPEND _algo_incs "${_e}")
    endif()
endforeach()

# ---- Module executables ----
set(_algo_defs
    LEARN_PROJECT_ROOT="${PROJ_ROOT}" LEARN_DATA_ROOT="${DATA_ROOT}" LEARN_MODELS_ROOT="${MODELS_ROOT}"
    ALGO_PROJECT_ROOT="${PROJ_ROOT}" ALGO_DATA_ROOT="${DATA_ROOT}" ALGO_MODELS_ROOT="${MODELS_ROOT}")

foreach(_m ${_active})
    set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${_m}/${_m}.cpp")
    if(NOT EXISTS "${_src}")
        message(WARNING "[algorithms] module ${_m} has no ${_m}.cpp, skip")
        continue()
    endif()

    add_executable(algorithms_${_m} "${_src}")
    set_target_properties(algorithms_${_m} PROPERTIES
        CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME "${_m}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/algorithms"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/algorithms"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/algorithms"
        DEBUG_POSTFIX "")
    target_include_directories(algorithms_${_m} PRIVATE ${_algo_incs})
    target_link_libraries(algorithms_${_m} PRIVATE
        demo_algorithms_common opencv_demo_common ${OpenCV_LIBS})
    target_compile_definitions(algorithms_${_m} PRIVATE ${_algo_defs})
    if(NOT MSVC)
        target_compile_options(algorithms_${_m} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    endif()
    message(STATUS "[algorithms] + exe ${_m} <- ${_src}")
endforeach()