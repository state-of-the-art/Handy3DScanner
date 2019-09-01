# Flann PCL dependency

set(FLANN_VERSION "1.9.1")
set(FLANN_HASH "ca3aee5670297f1db2eff122679ed9e87a70d830")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/src/flann-${FLANN_VERSION}")
    message("Downloading flann ${FLANN_VERSION} sources")
    set(flann_url "https://github.com/mariusmuja/flann/archive/${FLANN_VERSION}.tar.gz")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(DOWNLOAD "${flann_url}" "${CMAKE_SOURCE_DIR}/lib/src/flann.tar.gz" EXPECTED_HASH "SHA1=${FLANN_HASH}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/lib/src/flann.tar.gz" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    execute_process(COMMAND patch -p1 -i "${CMAKE_SOURCE_DIR}/lib/pcl-flann.patch" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src/flann-${FLANN_VERSION}")
endif()

set(BUILD_C_BINDINGS OFF CACHE BOOL "Flann option" FORCE)
set(BUILD_PYTHON_BINDINGS OFF CACHE BOOL "Flann option" FORCE)
set(BUILD_MATLAB_BINDINGS OFF CACHE BOOL "Flann option" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Flann option" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Flann option" FORCE)
set(BUILD_DOC OFF CACHE BOOL "Flann option" FORCE)

add_subdirectory("lib/src/flann-${FLANN_VERSION}" "${CMAKE_BINARY_DIR}/lib/flann")
set_target_properties(flann_cpp PROPERTIES COMPILE_FLAGS "-w") # Ignoring warnings from the dependency

add_dependencies(flann_cpp lz4)

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:flann_cpp>)
endif()

set(FLANN_INCLUDE_DIR "${flann_SOURCE_DIR}/src/cpp")
set(FLANN_LIBRARY "${flann_BINARY_DIR}/lib/libflann_cpp.so")

if(ANDROID)
    list(APPEND CUSTOM_ANDROID_EXTRA_LIBS "${flann_BINARY_DIR}/lib/libflann_cpp.so")
endif()
