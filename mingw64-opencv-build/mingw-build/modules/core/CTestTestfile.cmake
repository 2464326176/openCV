# CMake generated Testfile for 
# Source directory: E:/lyh/opencv/opencv/sources/modules/core
# Build directory: E:/lyh/opencv/opencv/mingw-build/modules/core
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_core "E:/lyh/opencv/opencv/mingw-build/bin/opencv_test_core.exe" "--gtest_output=xml:opencv_test_core.xml")
set_tests_properties(opencv_test_core PROPERTIES  LABELS "Main;opencv_core;Accuracy" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/accuracy" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1313;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/modules/core/CMakeLists.txt;160;ocv_add_accuracy_tests;E:/lyh/opencv/opencv/sources/modules/core/CMakeLists.txt;0;")
add_test(opencv_perf_core "E:/lyh/opencv/opencv/mingw-build/bin/opencv_perf_core.exe" "--gtest_output=xml:opencv_perf_core.xml")
set_tests_properties(opencv_perf_core PROPERTIES  LABELS "Main;opencv_core;Performance" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/performance" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1215;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/modules/core/CMakeLists.txt;161;ocv_add_perf_tests;E:/lyh/opencv/opencv/sources/modules/core/CMakeLists.txt;0;")
add_test(opencv_sanity_core "E:/lyh/opencv/opencv/mingw-build/bin/opencv_perf_core.exe" "--gtest_output=xml:opencv_perf_core.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
set_tests_properties(opencv_sanity_core PROPERTIES  LABELS "Main;opencv_core;Sanity" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/sanity" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1216;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/modules/core/CMakeLists.txt;161;ocv_add_perf_tests;E:/lyh/opencv/opencv/sources/modules/core/CMakeLists.txt;0;")
