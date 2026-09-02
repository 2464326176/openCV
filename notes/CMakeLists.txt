# ============================================================================
#  notes/CMakeLists.txt - Notes / image processing exercises
# ============================================================================

# ---- Collect all subdirectories for include paths ----
file(GLOB_RECURSE _notes_entries LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")
set(_notes_incs "${CMAKE_SOURCE_DIR}/common" ${OpenCV_INCLUDE_DIRS})
foreach(_e ${_notes_entries})
    if(IS_DIRECTORY "${_e}")
        list(APPEND _notes_incs "${_e}")
    endif()
endforeach()

# ---- Shared library for files without main() ----
add_library(demo_notes_common STATIC
    ${CMAKE_SOURCE_DIR}/notes/face_detect/image_process.cpp
    ${CMAKE_SOURCE_DIR}/notes/image_process/basic_drawing/draw.cpp)

target_include_directories(demo_notes_common PUBLIC ${_notes_incs})
target_link_libraries(demo_notes_common PUBLIC ${OpenCV_LIBS} opencv_demo_common)
target_compile_definitions(demo_notes_common PUBLIC
    LEARN_PROJECT_ROOT="${PROJ_ROOT}" LEARN_DATA_ROOT="${DATA_ROOT}" LEARN_MODELS_ROOT="${MODELS_ROOT}")
set_target_properties(demo_notes_common PROPERTIES
    CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

# ---- Helper: create a notes executable ----
function(add_notes_exe STEM SOURCE_PATH)
    # Disambiguate: if two files share the same stem (e.g. two different
    # subdirectories both have a "hough_lines.cpp"), append the parent dir.
    # This is checked at configure time.
    if(STEM IN_LIST _notes_exes)
        get_filename_component(_pardir "${SOURCE_PATH}" DIRECTORY)
        get_filename_component(_pardir "${_pardir}" NAME)
        set(_stem "${STEM}_${_pardir}")
        set(_k 2)
        while(_stem IN_LIST _notes_exes)
            set(_stem "${_stem}_${_k}")
            math(EXPR _k "${_k} + 1")
        endwhile()
        message(STATUS "[notes] duplicate stem disambiguated -> ${_stem}")
    else()
        set(_stem "${STEM}")
    endif()
    list(APPEND _notes_exes "${_stem}")
    set(_notes_exes "${_notes_exes}" PARENT_SCOPE)

    add_executable(notes_${_stem} "${SOURCE_PATH}")
    set_target_properties(notes_${_stem} PROPERTIES
        CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME "${_stem}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/notes"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/notes"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/notes"
        DEBUG_POSTFIX "")
    target_include_directories(notes_${_stem} PRIVATE ${_notes_incs})
    target_link_libraries(notes_${_stem} PRIVATE
        demo_notes_common opencv_demo_common ${OpenCV_LIBS})
    target_compile_definitions(notes_${_stem} PRIVATE
        LEARN_PROJECT_ROOT="${PROJ_ROOT}" LEARN_DATA_ROOT="${DATA_ROOT}" LEARN_MODELS_ROOT="${MODELS_ROOT}")
    if(NOT MSVC)
        target_compile_options(notes_${_stem} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    endif()
    message(STATUS "[notes] + exe ${_stem} <- ${SOURCE_PATH}")
endfunction()

# ---- camera_calibration ----
add_notes_exe(calibrate "camera_calibration/calibrate.cpp")

# ---- face_detect ----
add_notes_exe(face_detect "face_detect/face_detect.cpp")
add_notes_exe(single_frame_process "face_detect/single_frame_process.cpp")
add_notes_exe(test "face_detect/test.cpp")

# ---- features2d ----
add_notes_exe(surf_detection "features2d/feature_detection/surf_detection.cpp")

# ---- harris_detect ----
add_notes_exe(corner_harris "harris_detect/corner_harris.cpp")
add_notes_exe(harris "harris_detect/harris.cpp")

# ---- highgui ----
add_notes_exe(basic_linear_transforms_trackbar "highgui/basic_linear_transforms_trackbar.cpp")

# ---- histogram_match ----
add_notes_exe(calc_hist "histogram_match/calc_hist.cpp")
add_notes_exe(compare_hist "histogram_match/compare_hist.cpp")
add_notes_exe(equalize_hist "histogram_match/equalize_hist.cpp")
add_notes_exe(hs_hist "histogram_match/hs_hist.cpp")
add_notes_exe(match_template "histogram_match/match_template.cpp")

# ---- hough_transform ----
add_notes_exe(hough_circles "hough_transform/hough_circles.cpp")
add_notes_exe(hough_lines "hough_transform/hough_lines.cpp")

# ---- image_process ----
add_notes_exe(add_image "image_process/image/add_image.cpp")
add_notes_exe(color_reduce "image_process/image/color_reduce.cpp")
add_notes_exe(display_image "image_process/image/display_image.cpp")
add_notes_exe(image "image_process/image/image.cpp")
add_notes_exe(read_image "image_process/image/read_image.cpp")
add_notes_exe(region_of_interest "image_process/image/region_of_interest.cpp")
add_notes_exe(split_merge "image_process/image/split_merge.cpp")
add_notes_exe(mat "image_process/mat/mat.cpp")
add_notes_exe(mat1 "image_process/mat/mat1.cpp")
add_notes_exe(mat_operate "image_process/mat/mat_operate.cpp")
add_notes_exe(filter "image_process/morphology/filter.cpp")
add_notes_exe(flood_fill "image_process/morphology/flood_fill.cpp")
add_notes_exe(image_operation1 "image_process/morphology/image_operation1.cpp")
add_notes_exe(image_operation2 "image_process/morphology/image_operation2.cpp")
add_notes_exe(resize "image_process/morphology/resize.cpp")
add_notes_exe(filter_process "image_process/process/filter_process.cpp")
add_notes_exe(morph_op_process "image_process/process/morph_op_process.cpp")

# ---- image_segmentation ----
add_notes_exe(find_outline "image_segmentation/find_outline.cpp")
add_notes_exe(segmentation "image_segmentation/segmentation.cpp")

# ---- image_stitching ----
add_notes_exe(stitch "image_stitching/stitch.cpp")
add_notes_exe(stitcher_panorama "image_stitching/stitcher_panorama.cpp")

# ---- image_transformation ----
add_notes_exe(alpha "image_transformation/alpha.cpp")
add_notes_exe(canny "image_transformation/canny.cpp")
add_notes_exe(equalize_hist "image_transformation/equalize_hist.cpp")
add_notes_exe(laplacian "image_transformation/laplacian.cpp")
add_notes_exe(remap "image_transformation/remap.cpp")
add_notes_exe(scharr "image_transformation/scharr.cpp")
add_notes_exe(sobel "image_transformation/sobel.cpp")
add_notes_exe(trans "image_transformation/trans.cpp")

# ---- ml ----
add_notes_exe(svm "ml/svm.cpp")
add_notes_exe(svm_rbf "ml/svm_rbf.cpp")

# ---- photo_editing ----
add_notes_exe(photo_effects "photo_editing/photo_effects.cpp")
add_notes_exe(seamless_clone "photo_editing/seamless_clone.cpp")

# ---- video_tracking ----
add_notes_exe(bg_subtraction "video_tracking/bg_subtraction.cpp")
add_notes_exe(camshift "video_tracking/camshift.cpp")
add_notes_exe(meanshift "video_tracking/meanshift.cpp")
add_notes_exe(optical_flow "video_tracking/optical_flow.cpp")
add_notes_exe(optical_flow_dense "video_tracking/optical_flow_dense.cpp")