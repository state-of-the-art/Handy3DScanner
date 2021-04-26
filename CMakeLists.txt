cmake_minimum_required(VERSION 3.5)

project(handy3dscanner LANGUAGES CXX VERSION 0.5.2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ANDROID)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER CACHE STRING "Force to prevent issues" FORCE)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING "Force to prevent issues" FORCE)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY CACHE STRING "Force to prevent issues" FORCE)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY CACHE STRING "Force to prevent issues" FORCE)
    set(ANDROID_STL "c++_shared" CACHE STRING "Force to prevent issues" FORCE)
endif()

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_SOURCE_DIR}" src/*.cpp)
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_SOURCE_DIR}" src/*.h)

find_package(Qt5 COMPONENTS Core Quick Sensors 3DCore 3DRender 3DInput 3DQuick 3DQuickExtras REQUIRED)

# TODO: use the original PCL instead of this one
include(lib/pcl.cmake)

if(ANDROID)
    find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
    add_library(${PROJECT_NAME} SHARED ${Execution_CPP} ${Execution_H} src/app.qrc)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_BINARY_DIR}/android" CACHE INTERNAL "")
else()
    # Excluding android-only sources
    list(REMOVE_ITEM Execution_H  src/androidwrapper.h src/androidwrapperexception.h src/provider/arcore/arcore.h src/provider/arcore/arcoreworker.h)
    list(REMOVE_ITEM Execution_CPP src/android_jni.cpp src/androidwrapper.cpp src/provider/arcore/arcore.cpp src/provider/arcore/arcoreworker.cpp)
    add_executable(${PROJECT_NAME} ${Execution_CPP} ${Execution_H} src/app.qrc)
endif()

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:handy3dscanner>)
endif()

# Dependencies build
include(lib/librealsense.cmake)
include(lib/draco.cmake)
include(lib/arcore.cmake)
# Disabled until will be used
#include(pcl.cmake)

include(android.cmake)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME}
  PRIVATE Qt5::Core Qt5::Quick Qt5::Sensors Qt5::3DCore Qt5::3DRender Qt5::3DInput Qt5::3DQuick Qt5::3DLogic Qt5::3DQuickExtras)

if(ANDROID)
    list(REMOVE_DUPLICATES CUSTOM_ANDROID_EXTRA_LIBS)
    string(REPLACE ";" "," CUSTOM_ANDROID_EXTRA_LIBS_STRING "${CUSTOM_ANDROID_EXTRA_LIBS}")
    set(ANDROID_EXTRA_LIBS "${CUSTOM_ANDROID_EXTRA_LIBS_STRING}" CACHE INTERNAL "")
    message("Additional android libs: ${ANDROID_EXTRA_LIBS}")

    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras GLESv3)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND sh "${CMAKE_SOURCE_DIR}/tools/build-apk.sh" $<TARGET_FILE:handy3dscanner> ${ANDROID_EXTRA_LIBS} ${ANDROID_PACKAGE_SOURCE_DIR})
endif()
