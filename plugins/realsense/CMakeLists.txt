cmake_minimum_required(VERSION 3.13)

# Name and version
project(realsense LANGUAGES CXX VERSION 0.1.0)

# Common plugin parameters
include(${CMAKE_CURRENT_SOURCE_DIR}/../common.cmake)

# Add lib to include
target_link_libraries(${PROJECT_NAME} PRIVATE realsense2)
