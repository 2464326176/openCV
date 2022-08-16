cmake_minimum_required(VERSION 3.24)
project(image)

set(CMAKE_CXX_STANDARD 11)

set(OpenCV_DIR mingw-build/install)
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/units)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/units SRC_LIST)
add_subdirectory(imageProcess)
add_executable(opencv ${SRC_LIST}
        imageProcess/image/image.cpp)

target_link_libraries(opencv PRIVATE ${OpenCV_LIBS})