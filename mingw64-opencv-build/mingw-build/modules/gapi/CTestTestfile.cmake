# CMake generated Testfile for 
# Source directory: E:/lyh/opencv/opencv/sources/modules/gapi
# Build directory: E:/lyh/opencv/opencv/mingw-build/modules/gapi
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_gapi "E:/lyh/opencv/opencv/mingw-build/bin/opencv_test_gapi.exe" "--gtest_output=xml:opencv_test_gapi.xml")
set_tests_properties(opencv_test_gapi PROPERTIES  LABELS "Main;opencv_gapi;Accuracy" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/accuracy" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1313;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/modules/gapi/CMakeLists.txt;192;ocv_add_accuracy_tests;E:/lyh/opencv/opencv/sources/modules/gapi/CMakeLists.txt;0;")
add_test(opencv_perf_gapi "E:/lyh/opencv/opencv/mingw-build/bin/opencv_perf_gapi.exe" "--gtest_output=xml:opencv_perf_gapi.xml")
set_tests_properties(opencv_perf_gapi PROPERTIES  LABELS "Main;opencv_gapi;Performance" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/performance" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1215;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/modules/gapi/CMakeLists.txt;240;ocv_add_perf_tests;E:/lyh/opencv/opencv/sources/modules/gapi/CMakeLists.txt;0;")
add_test(opencv_sanity_gapi "E:/lyh/opencv/opencv/mingw-build/bin/opencv_perf_gapi.exe" "--gtest_output=xml:opencv_perf_gapi.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
set_tests_properties(opencv_sanity_gapi PROPERTIES  LABELS "Main;opencv_gapi;Sanity" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/sanity" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1216;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/modules/gapi/CMakeLists.txt;240;ocv_add_perf_tests;E:/lyh/opencv/opencv/sources/modules/gapi/CMakeLists.txt;0;")
