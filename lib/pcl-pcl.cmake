# PCL dependency

set(PCL_VERSION "1.9.1")
set(PCL_HASH "51e9e05008ab972b85ed416bb8685c04c4ce5fd7")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/src/pcl-${PCL_VERSION}")
    message("Downloading PCL ${PCL_VERSION} sources")
    set(pcl_url "https://github.com/PointCloudLibrary/pcl/archive/pcl-${PCL_VERSION}.tar.gz")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(DOWNLOAD "${pcl_url}" "${CMAKE_SOURCE_DIR}/lib/src/pcl.tar.gz" EXPECTED_HASH "SHA1=${PCL_HASH}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/lib/src/pcl.tar.gz" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(GLOB pcl_unpack "lib/src/pcl-*")
    file(RENAME "${pcl_unpack}" "${CMAKE_SOURCE_DIR}/lib/src/pcl-${PCL_VERSION}")
    execute_process(COMMAND patch -p1 -i "${CMAKE_SOURCE_DIR}/lib/pcl-pcl.patch" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src/pcl-${PCL_VERSION}")
endif()

set(PCL_SHARED_LIBS ON CACHE BOOL "PCL option" FORCE)
set(PCL_BINARIES OFF CACHE BOOL "PCL option" FORCE)
set(WITH_CUDA OFF CACHE BOOL "PCL option" FORCE)
set(WITH_OPENGL OFF CACHE BOOL "PCL option" FORCE)
set(WITH_PCAP OFF CACHE BOOL "PCL option" FORCE)
set(WITH_PNG OFF CACHE BOOL "PCL option" FORCE)
set(WITH_QHULL OFF CACHE BOOL "PCL option" FORCE)
set(WITH_VTK OFF CACHE BOOL "PCL option" FORCE)

# Required
set(BUILD_registration ON CACHE BOOL "PCL option" FORCE)
set(BUILD_io ON CACHE BOOL "PCL option" FORCE)

# Deps of reqs
set(BUILD_common ON CACHE BOOL "PCL option" FORCE)
set(BUILD_kdtree ON CACHE BOOL "PCL option" FORCE)
set(BUILD_octree ON CACHE BOOL "PCL option" FORCE)
set(BUILD_search ON CACHE BOOL "PCL option" FORCE)
set(BUILD_sample_consensus ON CACHE BOOL "PCL option" FORCE)
set(BUILD_filters ON CACHE BOOL "PCL option" FORCE)
set(BUILD_features ON CACHE BOOL "PCL option" FORCE)
set(BUILD_2d ON CACHE BOOL "PCL option" FORCE)

# Disabled
set(BUILD_geometry OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_ml OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_segmentation OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_surface OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_keypoints OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_tracking OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_recognition OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_stereo OFF CACHE BOOL "PCL option" FORCE)
set(BUILD_tools OFF CACHE BOOL "PCL option" FORCE)

add_subdirectory("lib/src/pcl-${PCL_VERSION}" "${CMAKE_BINARY_DIR}/lib/pcl")

get_cmake_property(_variableNames VARIABLES)
foreach(_variableName ${_variableNames})
    # Locating vars with PCL components to check they are enabled or not
    string(FIND "${_variableName}" "PCL_SUBSYS_STATUS_" out)
    if(out EQUAL 0 AND "${${_variableName}}")
        string(REPLACE "PCL_SUBSYS_STATUS_" "" comp ${_variableName})
        if(NOT comp STREQUAL "2d") # 2d pcl component is just a header
            if(ANDROID)
                list(APPEND CUSTOM_ANDROID_EXTRA_LIBS "${PCL_BINARY_DIR}/lib/libpcl_${comp}.so")
            endif()
            if(CMAKE_BUILD_TYPE STREQUAL Release)
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} "${PCL_BINARY_DIR}/lib/libpcl_${comp}.so")
            endif()
        endif()
    endif()
endforeach()

target_link_libraries(${PROJECT_NAME} PRIVATE pcl_registration pcl_io)

file(GLOB pcl_include_dirs LIST_DIRECTORIES true "${PCL_SOURCE_DIR}/*/include")
include_directories(AFTER ${pcl_include_dirs} "${PCL_BINARY_DIR}/include")
