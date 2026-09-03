# ============================================================================
#  learn/CMakeLists.txt - Learn layered exercises
#
#  Layer filtering via LEARN_LAYER cache variable (set in root CMakeLists.txt):
#    ALL  - build all layers (default)
#    L0   - L0_intro only
#    L1   - L1_core only
#    ...etc
#
#  The file lists below are data-driven: adding a new demo is a single line.
#  Target names are learn_<stem>; output names are just the stem, all
#  boilerplate (includes/links/defines/warnings/output dir) is applied by
#  the root add_demo_exe() helper.
# ============================================================================

# ---- Demo file lists per layer ----
set(_learn_L0_intro
    01_hello_imread 02_named_window_trackbar 03_mouse_roi
    04_drawing_primitives 05_videocapture_camera 06_split_merge
    07_opencv_version 08_videowriter)

set(_learn_L1_core
    01_mat_create_type 02_pixel_scan 03_lut_color_reduce
    04_mask_convolution 05_add_weighted 06_file_storage
    07_dft_spectrum 08_create_mask 09_ecc_align
    10_jpeg_codec 11_parallel_for 12_simd_basic)

set(_learn_L2_imgproc
    01_smoothing 02_filter2d 03_linear_transform_colormap
    04_erode_dilate 05_morphology_ex 06_hit_miss
    07_morph_lines 08_threshold 09_inrange_hsv
    10_sobel 11_scharr 12_laplacian
    13_canny 14_hough_lines 15_hough_lines_p
    16_hough_circles 17_warp_affine 18_warp_perspective
    19_remap 20_copy_make_border 21_polar_transform
    22_distance_transform 23_connected_components 24_flood_fill
    25_watershed 26_calc_hist 27_equalize_clahe
    28_compare_hist 29_backproject 30_match_template
    31_pyramids 32_phase_correlate 33_contrast_brightness_trackbar
    34_anisotropic_segmentation 35_motion_deblur 36_generalized_hough)

set(_learn_L3_features_video
    01_corner_harris 02_good_features 03_corner_subpix
    04_corner_detector 05_orb_detect_match 06_akaze_match
    07_bf_match_lowe 08_flann_match 09_homography
    10_find_contours 11_convex_hull 12_bounding_shapes
    13_moments_hu 14_point_polygon_test 15_general_contours
    16_bg_subtract_mog2 17_lk_optical_flow 18_farneback_dense
    19_dis_opticalflow 20_camshift 21_meanshift
    22_kalman 23_mser 24_blob_lsd
    25_homography_decompose 26_lk_stepwise)

set(_learn_L4_detect_calib
    01_cascade_face 02_hog_pedestrian 03_qrcode
    04_aruco_create 05_aruco_detect 06_inpaint
    07_grabcut 08_seamless_clone 09_decolor
    10_npr 11_hdr 12_camera_calib
    13_epipolar 14_stereo_match 15_stitching
    16_dbt_face_detection 17_facial_features 18_smile_detect
    19_essential_mat 20_pnp_pose 21_charuco_detect)

set(_learn_L5_ml_gapi
    01_kmeans 02_svm_intro 03_svm_nonlinear
    04_pca 05_em 06_ann_mlp
    07_decision_tree 08_boosting 09_logistic_reg
    10_svmsgd 11_gapi_blur_canny 12_gpu_basic
    13_digits_dnn 14_hog_svm_train 15_gapi_pipeline)

# ---- Layer filtering ----
set(_all_layers L0_intro L1_core L2_imgproc L3_features_video L4_detect_calib L5_ml_gapi)

# Short-key -> layer-name mapping (L0 -> L0_intro, ...)
set(_L0 L0_intro)
set(_L1 L1_core)
set(_L2 L2_imgproc)
set(_L3 L3_features_video)
set(_L4 L4_detect_calib)
set(_L5 L5_ml_gapi)

if(LEARN_LAYER STREQUAL "ALL")
    set(_active ${_all_layers})
elseif(LEARN_LAYER IN_LIST _all_layers)
    # Full layer name given, e.g. "L2_imgproc"
    set(_active ${LEARN_LAYER})
else()
    # Short key given, e.g. "L2"
    if(NOT DEFINED _${LEARN_LAYER})
        message(FATAL_ERROR "[learn] Unknown LEARN_LAYER='${LEARN_LAYER}' "
            "(expected ALL, L0..L5, or a full layer name like L2_imgproc)")
    endif()
    set(_active ${_${LEARN_LAYER}})
endif()

list(LENGTH _active _n)
message(STATUS "[learn] LEARN_LAYER=${LEARN_LAYER} active=${_active} (${_n} layers)")

# ---- Create all targets ----
foreach(_layer ${_active})
    foreach(_f IN LISTS _learn_${_layer})
        add_demo_exe(learn_${_f} "${_layer}/${_f}.cpp" 11 learn)
    endforeach()
endforeach()
