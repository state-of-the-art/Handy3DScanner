cmake_minimum_required(VERSION 3.13)

project(handy3dscanner)

if(ANDROID)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(ANDROID_STL "c++_shared" CACHE STRING "Force to prevent issues" FORCE)
endif()

# Common parameters
include(common.cmake)

## Build plugins
include(plugins.cmake)

message("Configure platform")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/platform)


#if(ANDROID)
#  list(REMOVE_DUPLICATES CUSTOM_ANDROID_EXTRA_LIBS)
#  string(REPLACE ";" "," CUSTOM_ANDROID_EXTRA_LIBS_STRING "${CUSTOM_ANDROID_EXTRA_LIBS}")
#  set(ANDROID_EXTRA_LIBS "${CUSTOM_ANDROID_EXTRA_LIBS_STRING}" CACHE INTERNAL "")
#  message("Additional android libs: ${ANDROID_EXTRA_LIBS}")
#
#  target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras GLESv3)
#
#  # qtcreator cmake android deployment don't want to set sdkBuildToolsRevision
#  add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#    BYPRODUCTS notexisting.file # To build the package anyway
#    COMMAND ${CMAKE_COMMAND} -E env
#      ANDROID_KEYSTORE_FILE="${CMAKE_SOURCE_DIR}/android.keystore"
#      QT_ANDROID="${CMAKE_PREFIX_PATH}"
#      ANDROID_SDK_BUILD_TOOLS=29.0.1
#      ANDROID_NDK_TOOLCHAIN_ABI=${ANDROID_ABI}
#      QML_ROOT_PATH="${CMAKE_SOURCE_DIR}"
#      sh "${CMAKE_SOURCE_DIR}/tool/build-apk.sh" $<TARGET_FILE:${PROJECT_NAME}> ${ANDROID_EXTRA_LIBS} ${ANDROID_PACKAGE_SOURCE_DIR}
#  )
#endif()


#set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
#
## Add targets to check the source code format
#set(CLANG_FORMAT_BIN_NAME clang-format-10)
#set(CLANG_FORMAT_EXCLUDE_PATTERNS "build/" "/libs/" ${CMAKE_BINARY_DIR})
#find_package(ClangFormat)
#
## Make sure the source code is well formatted
#add_dependencies(${PROJECT_NAME} format-check)


# Packaging
add_custom_target(package ALL
  COMMAND ${CMAKE_COMMAND} -E echo "Ok, package of ${PROJECT_NAME} is done"
)

if(ANDROID)
  # TODO Making an addition to the deployment JSON file
  file(READ "${CMAKE_BINARY_DIR}/android_deployment_settings.json" deploy_data_in)
  string(REPLACE "\"ndk\":"
    "\"sdkBuildToolsRevision\": \"${ANDROID_SDK_BUILD_TOOLS}\",\n  \"ndk\":" deploy_data_in "${deploy_data_in}")
  file(WRITE "${CMAKE_BINARY_DIR}/android_deployment_settings.json" "${deploy_data_in}")
  add_custom_command(TARGET package PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_BINARY_DIR}/android-build/${PROJECT_NAME}.apk"
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}_$<JOIN:${BUILD_ABIS},->.apk"
  )
  add_dependencies(package apk)
endif()

# Copy plugins
if(ANDROID)
  add_custom_target(prepare_package ALL
    COMMAND ${CMAKE_COMMAND} -E copy plugins/*.aar android-build/libs
    DEPENDS ${main_target_name}
  )
else()
  # TODO: copy plugins for non-android
endif()

add_dependencies(apk prepare_package)
add_dependencies(prepare_package ${PLUGINS_DEPENDS})
