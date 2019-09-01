# LZ4 PCL dependency

set(LZ4_VERSION "1.9.1")
set(LZ4_HASH "70cf7b4ac726b6afd54c2e151aa4a7815279966a")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/src/lz4-${LZ4_VERSION}")
    message("Downloading lz4 ${LZ4_VERSION} sources")
    set(lz4_url "https://github.com/lz4/lz4/archive/v${LZ4_VERSION}.tar.gz")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(DOWNLOAD "${lz4_url}" "${CMAKE_SOURCE_DIR}/lib/src/lz4.tar.gz" EXPECTED_HASH "SHA1=${LZ4_HASH}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/lib/src/lz4.tar.gz" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    execute_process(COMMAND patch -p1 -i "${CMAKE_SOURCE_DIR}/lib/pcl-lz4.patch" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src/lz4-${LZ4_VERSION}")
endif()

add_subdirectory("lib/src/lz4-${LZ4_VERSION}" "${CMAKE_BINARY_DIR}/lib/lz4")
set_target_properties(lz4 PROPERTIES COMPILE_FLAGS "-w") # Ignoring warnings from the dependency

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:lz4>)
endif()

#if(ANDROID)
#    list(APPEND CUSTOM_ANDROID_EXTRA_LIBS "${lz4_BINARY_DIR}/liblz4.so")
#endif()

#target_link_libraries(${PROJECT_NAME} PRIVATE "${lz4_BINARY_DIR}/liblz4.so")
#include_directories(AFTER "${lz4_SOURCE_DIR}/src" "${lz4_BINARY_DIR}/lz4")
