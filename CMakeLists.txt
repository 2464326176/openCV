cmake_minimum_required(VERSION 3.13)
project(opencv) # Define project name

INCLUDE_DIRECTORIES("E:/lyh/opencv/opencv/mingw-build")

link_directories("E:/lyh/opencv/opencv/mingw-build/lib")


ADD_DEFINITIONS(
        -O3 -g -W -Wall
        -Wunused-variable -Wunused-parameter -Wunused-function -Wunused
        -Wno-deprecated -Woverloaded-virtual -Wwrite-strings
        -D__WUR= -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DTIXML_USE_STL
)

#寻找OpenCV库
find_package(OpenCV REQUIRED)


message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


set(CMAKE_CXX_STANDARD 11)

add_executable(opencv main.cpp)

#添加动态链接库
target_link_libraries(opencv PRIVATE ${OpenCV_LIBS})


