cmake_minimum_required(VERSION 3.13)

# List of the dirs and add as subdirs
set(_libs_dir "${CMAKE_CURRENT_SOURCE_DIR}/libs")
file(GLOB childs RELATIVE "${_libs_dir}" "${_libs_dir}/*")
unset(LIBS_LIST)
foreach(child ${childs})
  if(IS_DIRECTORY ${_libs_dir}/${child})
    list(APPEND LIBS_LIST "${child}")
  endif()
endforeach()

# Cleaning the variables
unset(LIBS_DEPENDS)
unset(LIBS_LIB_DIRS_${BUILD_ABI})
unset(LIBS_INCLUDE_DIRS_${BUILD_ABI})

include(ExternalProject)
foreach(lib ${LIBS_LIST})
  message("Configure lib: ${lib}")

  if(${PROJECT_NAME}-MultiAbiBuild)
    # For multi-abi libs - cutting MultiAbi...
    set(LIB_BASE_BINARY_DIR ${CMAKE_BINARY_DIR}/../../../libs/${lib})
  else()
    set(LIB_BASE_BINARY_DIR ${CMAKE_BINARY_DIR}/libs/${lib})
  endif()
  include(${_libs_dir}/${lib}/config.cmake)

  ExternalProject_Add(lib_${lib}_sources
    URL               ${lib_${lib}_url}
    URL_HASH          ${lib_${lib}_hash}
    DOWNLOAD_DIR      libs/_download
    SOURCE_DIR        ${LIB_BASE_BINARY_DIR}/src
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )

  if(ANDROID)
    set(android_lib_args
      -D ANDROID_ABI=${BUILD_ABI}
      -D ANDROID_PLATFORM=${ANDROID_PLATFORM}
      -D ANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
    )
  endif()

  set(LIB_RESULT_DIR ${LIB_BASE_BINARY_DIR}/${BUILD_ABI})

  ExternalProject_Add(lib_${lib}_${BUILD_ABI}
    DEPENDS lib_${lib}_sources
    PREFIX libs/_build
    SOURCE_DIR ${_libs_dir}/${lib}
    INSTALL_DIR ${LIB_RESULT_DIR}
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_ALWAYS YES
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_ARGS
      -D LIB_BASE_BINARY_DIR=${LIB_BASE_BINARY_DIR}
      -D LIB_RESULT_DIR=${LIB_RESULT_DIR}

      -D CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
      -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
      -D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
      -D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
      -D CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
      ${android_lib_args}
  )

  list(APPEND LIBS_LIB_DIRS_${BUILD_ABI} "${LIB_RESULT_DIR}/lib")
  list(APPEND LIBS_INCLUDE_DIRS_${BUILD_ABI} "${LIB_RESULT_DIR}/include")
  list(APPEND LIBS_DEPENDS lib_${lib}_${BUILD_ABI})
endforeach()
