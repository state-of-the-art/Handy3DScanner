cmake_minimum_required(VERSION 3.13)

project(realsense LANGUAGES CXX VERSION 0.1.0)

# Common plugin parameters
include(../common.cmake)

find_package(Qt5 COMPONENTS Core REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core realsense2)
