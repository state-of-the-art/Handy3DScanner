# Common plugin parameters

# Actually includes h3ds core common cmake
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

set(CMAKE_SHARED_LIBRARY_PREFIX "lib${BUILD_H3DS_PLUGIN_PREFIX}")

if(ANDROID)
  set(QT_ANDROID_APPLICATION_BINARY "${BUILD_H3DS_PLUGIN_PREFIX}${PROJECT_NAME}")
  set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android")
endif()

# Gui needed for qt android lib
find_package(Qt5 COMPONENTS Core Gui REQUIRED)
if(ANDROID)
  find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
endif()

# Build libs
include(${CMAKE_CURRENT_LIST_DIR}/libs.cmake)

# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${Execution_CPP})

add_dependencies(${PROJECT_NAME} ${LIBS_DEPENDS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Gui $<$<BOOL:${ANDROID}>:Qt5::AndroidExtras>)
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBS_LIB_DIRS_${BUILD_ABI}})
target_include_directories(${PROJECT_NAME} PRIVATE "${BUILD_H3DS_INCLUDE_DIR}" ${LIBS_INCLUDE_DIRS_${BUILD_ABI}})

include(${CMAKE_CURRENT_LIST_DIR}/package.cmake)
