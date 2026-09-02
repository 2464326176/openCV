# ============================================================================
#  learn/CMakeLists.txt - Learn layered exercises
#
#  Layer filtering via LEARN_LAYER cache variable (set in root CMakeLists.txt):
#    ALL  - build all layers (default)
#    L0   - L0_intro only
#    L1   - L1_core only
#    ...etc
# ============================================================================

# ---- Layer filtering ----
set(_all_layers L0_intro L1_core L2_imgproc L3_features_video L4_detect_calib L5_ml_gapi)

if(LEARN_LAYER STREQUAL "ALL")
    set(_active ${_all_layers})
else()
    set(_L0 L0_intro)
    set(_L1 L1_core)
    set(_L2 L2_imgproc)
    set(_L3 L3_features_video)
    set(_L4 L4_detect_calib)
    set(_L5 L5_ml_gapi)
    set(_active ${_${LEARN_LAYER}})
endif()

message(STATUS "[learn] LEARN_LAYER=${LEARN_LAYER} active=${_active}")

# ---- L0_intro ----
if("L0_intro" IN_LIST _active)
    foreach(_f
        01_hello_imread 02_named_window_trackbar 03_mouse_roi
        04_drawing_primitives 05_videocapture_camera 06_split_merge
        07_opencv_version 08_videowriter)
        add_demo_exe(learn_${_f} "L0_intro/${_f}.cpp" 11 learn)
        set_target_properties(learn_${_f} PROPERTIES OUTPUT_NAME "${_f}")
    endforeach()
endif()

# ---- L1_core ----
if("L1_core" IN_LIST _active)
    foreach(_f
        01_mat_create_type 02_pixel_scan 03_lut_color_reduce
        04_mask_convolution 05_add_weighted 06_file_storage
        07_dft_spectrum 08_create_mask 09_ecc_align
        10_jpeg_codec 11_parallel_for 12_simd_basic)
        add_demo_exe(learn_${_f} "L1_core/${_f}.cpp" 11 learn)
        set_target_properties(learn_${_f} PROPERTIES OUTPUT_NAME "${_f}")
    endforeach()
endif()

# ---- L2_imgproc ----
if("L2_imgproc" IN_LIST _active)
    foreach(_f
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
        add_demo_exe(learn_${_f} "L2_imgproc/${_f}.cpp" 11 learn)
        set_target_properties(learn_${_f} PROPERTIES OUTPUT_NAME "${_f}")
    endforeach()
endif()

# ---- L3_features_video ----
if("L3_features_video" IN_LIST _active)
    foreach(_f
        01_corner_harris 02_good_features 03_corner_subpix
        04_corner_detector 05_orb_detect_match 06_akaze_match
        07_bf_match_lowe 08_flann_match 09_homography
        10_find_contours 11_convex_hull 12_bounding_shapes
        13_moments_hu 14_point_polygon_test 15_general_contours
        16_bg_subtract_mog2 17_lk_optical_flow 18_farneback_dense
        19_dis_opticalflow 20_camshift 21_meanshift
        22_kalman 23_mser 24_blob_lsd
        25_homography_decompose 26_lk_stepwise)
        add_demo_exe(learn_${_f} "L3_features_video/${_f}.cpp" 11 learn)
        set_target_properties(learn_${_f} PROPERTIES OUTPUT_NAME "${_f}")
    endforeach()
endif()

# ---- L4_detect_calib ----
if("L4_detect_calib" IN_LIST _active)
    foreach(_f
        01_cascade_face 02_hog_pedestrian 03_qrcode
        04_aruco_create 05_aruco_detect 06_inpaint
        07_grabcut 08_seamless_clone 09_decolor
        10_npr 11_hdr 12_camera_calib
        13_epipolar 14_stereo_match 15_stitching
        16_dbt_face_detection 17_facial_features 18_smile_detect
        19_essential_mat 20_pnp_pose 21_charuco_detect)
        add_demo_exe(learn_${_f} "L4_detect_calib/${_f}.cpp" 11 learn)
        set_target_properties(learn_${_f} PROPERTIES OUTPUT_NAME "${_f}")
    endforeach()
endif()

# ---- L5_ml_gapi ----
if("L5_ml_gapi" IN_LIST _active)
    foreach(_f
        01_kmeans 02_svm_intro 03_svm_nonlinear
        04_pca 05_em 06_ann_mlp
        07_decision_tree 08_boosting 09_logistic_reg
        10_svmsgd 11_gapi_blur_canny 12_gpu_basic
        13_digits_dnn 14_hog_svm_train 15_gapi_pipeline)
        add_demo_exe(learn_${_f} "L5_ml_gapi/${_f}.cpp" 11 learn)
        set_target_properties(learn_${_f} PROPERTIES OUTPUT_NAME "${_f}")
    endforeach()
endif()