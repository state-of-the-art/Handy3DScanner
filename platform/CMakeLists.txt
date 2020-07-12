cmake_minimum_required(VERSION 3.13)

project(handy3dscanner LANGUAGES CXX)

include(../common.cmake)

find_package(Qt5 COMPONENTS Core Quick Sensors 3DCore 3DRender 3DInput 3DQuick 3DQuickExtras REQUIRED)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h" "include/*.h")

if(ANDROID)
  find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
  add_library(${PROJECT_NAME} SHARED ${Execution_CPP} ${Execution_H} "${CMAKE_CURRENT_SOURCE_DIR}/src/app.qrc")
  set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_BINARY_DIR}/android" CACHE INTERNAL "")
else()
  # Excluding android-only sources
  list(REMOVE_ITEM Execution_H src/androidwrapper.h src/androidwrapperexception.h)
  list(REMOVE_ITEM Execution_CPP src/android_jni.cpp src/androidwrapper.cpp)
  add_executable(${PROJECT_NAME} ${Execution_CPP} ${Execution_H} src/app.qrc)
endif()

include(android.cmake)


target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME}
  PRIVATE Qt5::Core Qt5::Quick Qt5::Sensors Qt5::3DCore Qt5::3DRender Qt5::3DInput Qt5::3DQuick Qt5::3DLogic Qt5::3DQuickExtras
  $<$<BOOL:${ANDROID}>:Qt5::AndroidExtras> $<$<BOOL:${ANDROID}>:-llog> )

# Attach project modules
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${BUILD_H3DS_INCLUDE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
