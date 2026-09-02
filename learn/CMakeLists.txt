# learn/CMakeLists.txt
# Layered OpenCV exercises. Each .cpp is an independent executable learn_<stem>.

get_filename_component(LEARN_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(LEARN_DATA_ROOT "${LEARN_PROJECT_ROOT}/data")
set(LEARN_MODELS_ROOT "${LEARN_PROJECT_ROOT}/models")

add_library(learn_common STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../common/opencv_utils.cpp)
target_include_directories(learn_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${OpenCV_INCLUDE_DIRS})
target_link_libraries(learn_common PUBLIC ${OpenCV_LIBS})
target_compile_definitions(learn_common PUBLIC
    LEARN_PROJECT_ROOT="${LEARN_PROJECT_ROOT}/"
    LEARN_DATA_ROOT="${LEARN_DATA_ROOT}"
    LEARN_MODELS_ROOT="${LEARN_MODELS_ROOT}")

set(LEARN_LAYER "ALL" CACHE STRING "Which layer to build: L0,L1,L2,L3,L4,L5 or ALL")
set_property(CACHE LEARN_LAYER PROPERTY STRINGS ALL L0 L1 L2 L3 L4 L5)

set(_all_layers L0_intro L1_core L2_imgproc L3_features_video L4_detect_calib L5_ml_gapi)
if(LEARN_LAYER STREQUAL "ALL")
    set(_active_layers ${_all_layers})
else()
    set(_map_L0 L0_intro)
    set(_map_L1 L1_core)
    set(_map_L2 L2_imgproc)
    set(_map_L3 L3_features_video)
    set(_map_L4 L4_detect_calib)
    set(_map_L5 L5_ml_gapi)
    set(_active_layers ${_map_${LEARN_LAYER}})
endif()

message(STATUS "learn: LEARN_LAYER=${LEARN_LAYER}, active layers=${_active_layers}")
message(STATUS "learn: project root=${LEARN_PROJECT_ROOT}")

set(_learn_runtime_dir "${CMAKE_BINARY_DIR}/learn")
file(MAKE_DIRECTORY "${_learn_runtime_dir}")

foreach(_layer ${_active_layers})
    file(GLOB _srcs "${CMAKE_CURRENT_SOURCE_DIR}/${_layer}/*.cpp")
    if(NOT _srcs)
        message(STATUS "learn: ${_layer} has no .cpp yet (skipped)")
    endif()
    foreach(_src ${_srcs})
        get_filename_component(_stem ${_src} NAME_WE)
        add_executable(learn_${_stem} ${_src})
        target_link_libraries(learn_${_stem} PRIVATE learn_common)
        target_include_directories(learn_${_stem} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common)
        set_target_properties(learn_${_stem} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${_learn_runtime_dir}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${_learn_runtime_dir}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_learn_runtime_dir}"
        )
    endforeach()
endforeach()
