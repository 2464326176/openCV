# CMake generated Testfile for 
# Source directory: E:/lyh/opencv/opencv/sources/modules/ml
# Build directory: E:/lyh/opencv/opencv/mingw-build/modules/ml
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_ml "E:/lyh/opencv/opencv/mingw-build/bin/opencv_test_ml.exe" "--gtest_output=xml:opencv_test_ml.xml")
set_tests_properties(opencv_test_ml PROPERTIES  LABELS "Main;opencv_ml;Accuracy" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/accuracy" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1313;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1077;ocv_add_accuracy_tests;E:/lyh/opencv/opencv/sources/modules/ml/CMakeLists.txt;2;ocv_define_module;E:/lyh/opencv/opencv/sources/modules/ml/CMakeLists.txt;0;")
