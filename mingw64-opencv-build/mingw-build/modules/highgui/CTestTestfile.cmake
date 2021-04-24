# CMake generated Testfile for 
# Source directory: E:/lyh/opencv/opencv/sources/modules/highgui
# Build directory: E:/lyh/opencv/opencv/mingw-build/modules/highgui
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_highgui "E:/lyh/opencv/opencv/mingw-build/bin/opencv_test_highgui.exe" "--gtest_output=xml:opencv_test_highgui.xml")
set_tests_properties(opencv_test_highgui PROPERTIES  LABELS "Main;opencv_highgui;Accuracy" WORKING_DIRECTORY "E:/lyh/opencv/opencv/mingw-build/test-reports/accuracy" _BACKTRACE_TRIPLES "E:/lyh/opencv/opencv/sources/cmake/OpenCVUtils.cmake;1707;add_test;E:/lyh/opencv/opencv/sources/cmake/OpenCVModule.cmake;1313;ocv_add_test_from_target;E:/lyh/opencv/opencv/sources/modules/highgui/CMakeLists.txt;165;ocv_add_accuracy_tests;E:/lyh/opencv/opencv/sources/modules/highgui/CMakeLists.txt;0;")
