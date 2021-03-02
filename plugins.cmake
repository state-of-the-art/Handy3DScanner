cmake_minimum_required(VERSION 3.13)

# List of the dirs and add as subdirs
set(_plugins_dir "${CMAKE_CURRENT_LIST_DIR}/plugins")
file(GLOB childs RELATIVE ${_plugins_dir} "${_plugins_dir}/*")
set(PLUGINS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins)
set(PLUGINS_LIST "")

foreach(child ${childs})
  if(EXISTS "${_plugins_dir}/${child}/CMakeLists.txt")
    list(APPEND PLUGINS_LIST "${child}")
  endif()
endforeach()

unset(PLUGINS_DIRS)
unset(PLUGINS_INCLUDE_DIRS)

include(ExternalProject)
foreach(plugin ${PLUGINS_LIST})
  message("Configure plugin: ${plugin}")

  if(ANDROID)
    set(android_plugin_args
      -D ANDROID_ABI=${BUILD_ABI}
      -D ANDROID_PLATFORM=${ANDROID_PLATFORM}
      -D ANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
      -D ANDROID_NDK=${ANDROID_NDK}
      -D ANDROID_SDK=${ANDROID_SDK}
    )
    foreach(abi ${BUILD_ABIS})
      list(APPEND -D ANDROID_BUILD_ABI_${abi}=${ANDROID_BUILD_ABI_${abi}})
    endforeach()
    set(_install_command ${CMAKE_COMMAND} -E copy
      <BINARY_DIR>/android-build/${plugin}.aar
      <INSTALL_DIR>/${plugin}.aar)
  else()
    set(_install_command ${CMAKE_COMMAND} -E copy
      <BINARY_DIR>/lib${BUILD_H3DS_PLUGIN_PREFIX}${plugin}.so
      <INSTALL_DIR>/lib${BUILD_H3DS_PLUGIN_PREFIX}${plugin}_${BUILD_ABI}.so
    )
  endif()

  set(PLUGIN_RESULT_DIR ${PLUGINS_BIN_DIR})

  ExternalProject_Add(plugin_${plugin}
    PREFIX plugins/_build
    SOURCE_DIR ${_plugins_dir}/${plugin}
    INSTALL_DIR ${PLUGIN_RESULT_DIR}
    UPDATE_COMMAND "${CMAKE_COMMAND}"
    PATCH_COMMAND ""
    BUILD_ALWAYS YES
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ${_install_command}
    LIST_SEPARATOR | # Use the alternate list separator
    CMAKE_ARGS
      -D BUILD_H3DS_SRC_DIR=${CMAKE_SOURCE_DIR}
      -D BUILD_H3DS_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/include

      -D CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
      -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
      -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
      -D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
      -D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
      -D CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
      ${android_plugin_args}
  )

  list(APPEND PLUGINS_DIRS "${PLUGIN_RESULT_DIR}")
  list(APPEND PLUGINS_DEPENDS plugin_${plugin})
endforeach()
