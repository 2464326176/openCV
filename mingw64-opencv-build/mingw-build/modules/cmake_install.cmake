# Install script for directory: E:/lyh/opencv/opencv/sources/modules

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "E:/lyh/opencv/opencv/mingw-build/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/mingw64/bin/objdump.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlicensesx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/etc/licenses" TYPE FILE RENAME "ade-LICENSE" FILES "E:/lyh/opencv/opencv/mingw-build/3rdparty/ade/ade-0.1.1f/LICENSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlicensesx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/etc/licenses" TYPE FILE RENAME "ffmpeg-license.txt" FILES "E:/lyh/opencv/opencv/sources/3rdparty/ffmpeg/license.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlicensesx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/etc/licenses" TYPE FILE RENAME "ffmpeg-readme.txt" FILES "E:/lyh/opencv/opencv/sources/3rdparty/ffmpeg/readme.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/calib3d/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/core/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/dnn/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/features2d/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/flann/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/gapi/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/highgui/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/imgcodecs/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/imgproc/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/java/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/js/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/ml/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/objc/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/objdetect/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/photo/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/python/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/stitching/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/ts/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/video/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/videoio/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/.firstpass/world/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/core/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/flann/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/imgproc/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/ml/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/photo/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/python_tests/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/dnn/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/features2d/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/imgcodecs/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/videoio/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/calib3d/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/highgui/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/objdetect/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/stitching/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/ts/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/video/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/gapi/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/java_bindings_generator/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/js_bindings_generator/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/objc_bindings_generator/cmake_install.cmake")
  include("E:/lyh/opencv/opencv/mingw-build/modules/python_bindings_generator/cmake_install.cmake")

endif()

