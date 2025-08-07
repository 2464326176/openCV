cmake_minimum_required(VERSION 3.10.0)
project(openCv VERSION 0.1.0 LANGUAGES C CXX)

cmake_minimum_required(VERSION 3.24)
project(image)

set(CMAKE_CXX_STANDARD 11)

set(OpenCV_DIR mingw-build)
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/units)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/units SRC_LIST)
add_subdirectory(imageProcess)
add_executable(openCv ${SRC_LIST}
        facedetect/facedetect.cpp)

target_link_libraries(openCv PRIVATE ${OpenCV_LIBS})

