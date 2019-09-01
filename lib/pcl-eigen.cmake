# Eigen PCL dependency

set(EIGEN_VERSION "3.3.7")
set(EIGEN_HASH "a06faa6f358d5d1ca0da7cddb95da39e436dc9e8")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/src/eigen-${EIGEN_VERSION}")
    message("Downloading eigen ${EIGEN_VERSION} sources")
    set(eigen_url "https://bitbucket.org/eigen/eigen/get/${EIGEN_VERSION}.tar.bz2")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(DOWNLOAD "${eigen_url}" "${CMAKE_SOURCE_DIR}/lib/src/eigen.tar.gz" EXPECTED_HASH "SHA1=${EIGEN_HASH}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/lib/src/eigen.tar.gz" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(GLOB eigen_unpack "lib/src/eigen-*")
    file(RENAME ${eigen_unpack} "${CMAKE_SOURCE_DIR}/lib/src/eigen-${EIGEN_VERSION}")
endif()

set(EIGEN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/src/eigen-${EIGEN_VERSION}")
include_directories(AFTER "${EIGEN_INCLUDE_DIR}")
