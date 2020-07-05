# Common plugin parameters

# Actually includes h3ds core common cmake
include(../../common.cmake)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")

set(CMAKE_SHARED_LIBRARY_PREFIX "${BUILD_H3DS_PLUGIN_PREFIX}")

add_library(${PROJECT_NAME} SHARED ${Execution_CPP})
target_link_directories(${PROJECT_NAME} PRIVATE ${BUILD_H3DS_LIBS_LIB_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE "${BUILD_H3DS_INCLUDE_DIR}" ${BUILD_H3DS_LIBS_INCLUDE_DIRS})
