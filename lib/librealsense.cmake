# Librealsense dependency

set(LIBREALSENSE_VERSION "2.44.0")
set(LIBREALSENSE_HASH "ad37fe8e8364f2c0533f8c46ccc4a0a3de538656")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/src/librealsense-${LIBREALSENSE_VERSION}")
    message("Downloading librealsense ${LIBREALSENSE_VERSION} sources")
    set(librealsense_url "https://github.com/IntelRealSense/librealsense/archive/v${LIBREALSENSE_VERSION}.tar.gz")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(DOWNLOAD "${librealsense_url}" "${CMAKE_SOURCE_DIR}/lib/src/librealsense.tar.gz" EXPECTED_HASH "SHA1=${LIBREALSENSE_HASH}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/lib/src/librealsense.tar.gz" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
endif()

set(BUILD_GRAPHICAL_EXAMPLES OFF CACHE BOOL "Librealsense option" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Librealsense option" FORCE)
set(BUILD_WITH_OPENMP ON CACHE BOOL "Librealsense option" FORCE)
set(BUILD_WITH_TM2 OFF CACHE BOOL "Librealsense option" FORCE)
set(BUILD_EXTERNAL_LIBUSB ON CACHE BOOL "Librealsense option" FORCE)

if(ANDROID)
    set(ANDROID_USB_HOST_UVC ON CACHE BOOL "Librealsense option" FORCE)
endif()

add_subdirectory("lib/src/librealsense-${LIBREALSENSE_VERSION}" "${CMAKE_BINARY_DIR}/lib/librealsense")
set_target_properties(realsense2 PROPERTIES COMPILE_FLAGS "-w") # Ignoring warnings from the dependency

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:realsense2>)
endif()

if(ANDROID)
    list(APPEND CUSTOM_ANDROID_EXTRA_LIBS "${librealsense2_BINARY_DIR}/librealsense2.so")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE realsense2)
include_directories(AFTER "${librealsense2_SOURCE_DIR}/include" "${librealsense2_SOURCE_DIR}/third-party")
