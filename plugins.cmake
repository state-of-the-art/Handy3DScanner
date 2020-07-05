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

foreach(abi ${BUILD_ABIS})
  unset(PLUGINS_DIRS_${abi})
  unset(PLUGINS_INCLUDE_DIRS_${abi})
endforeach()

include(ExternalProject)
foreach(plugin ${PLUGINS_LIST})
  message("Configure plugin: ${plugin}")

  foreach(abi ${BUILD_ABIS})
    if(ANDROID)
      set(android_plugin_args
        -D ANDROID_ABI=${abi}
        -D ANDROID_PLATFORM=${ANDROID_PLATFORM}
        -D ANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
      )
    endif()

    if(ANDROID)
      set(_install_command ${CMAKE_COMMAND} -E copy_directory <BINARY_DIR>/android-build/libs <INSTALL_DIR>/lib)
    else()
      set(_install_command ${CMAKE_COMMAND} -E copy
        <BINARY_DIR>/${BUILD_H3DS_PLUGIN_PREFIX}${plugin}.so
        <INSTALL_DIR>/${BUILD_H3DS_PLUGIN_PREFIX}${plugin}_${abi}.so
      )
    endif()

    set(PLUGIN_RESULT_DIR ${PLUGINS_BIN_DIR}/${abi})

    string(REPLACE ";" "|" libs_lib_dirs "${LIBS_LIB_DIRS_${abi}}")
    string(REPLACE ";" "|" libs_include_dirs "${LIBS_INCLUDE_DIRS_${abi}}")

    ExternalProject_Add(plugin_${plugin}_${abi}
      DEPENDS ${LIBS_DEPENDS} # Make sure plugins will be built after libs
      PREFIX plugins/_build
      SOURCE_DIR ${_plugins_dir}/${plugin}
      INSTALL_DIR ${PLUGIN_RESULT_DIR}
      UPDATE_COMMAND ""
      PATCH_COMMAND ""
      BUILD_ALWAYS YES
      DOWNLOAD_COMMAND ""
      INSTALL_COMMAND ${_install_command}
      LIST_SEPARATOR | # Use the alternate list separator
      CMAKE_ARGS
        -D BUILD_H3DS_SRC_DIR=${CMAKE_SOURCE_DIR}
        -D BUILD_H3DS_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
        -D BUILD_H3DS_LIBS_LIB_DIRS=${libs_lib_dirs}
        -D BUILD_H3DS_LIBS_INCLUDE_DIRS=${libs_include_dirs}

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

    list(APPEND PLUGINS_DIRS_${abi} "${PLUGIN_RESULT_DIR}")
    list(APPEND PLUGINS_DEPENDS plugin_${plugin}_${abi})
  endforeach()
endforeach()
