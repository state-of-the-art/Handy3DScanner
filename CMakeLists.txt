cmake_minimum_required(VERSION 3.13)

project(handy3dscanner LANGUAGES CXX VERSION 0.6.0)

if(ANDROID)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY CACHE STRING "Force to prevent issues" FORCE)
  set(ANDROID_STL "c++_shared" CACHE STRING "Force to prevent issues" FORCE)
endif()

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
set(BUILD_H3DS_SRC_DIR "${CMAKE_SOURCE_DIR}" CACHE STRING "Handy 3D Scanner source directory for plugins")
set(BUILD_H3DS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" CACHE STRING "Handy 3D Scanner include directory for plugins")

# Common parameters
include(common.cmake)

# Build libs
include(libs.cmake)

## Build plugins
include(plugins.cmake)


# Listing the available sources to build
file(GLOB_RECURSE Execution_CPP RELATIVE "${CMAKE_SOURCE_DIR}" "src/*.cpp")
file(GLOB_RECURSE Execution_H RELATIVE "${CMAKE_SOURCE_DIR}" "src/*.h" "include/*.h")

find_package(Qt5 COMPONENTS Core Quick Sensors 3DCore 3DRender 3DInput 3DQuick 3DQuickExtras REQUIRED)

if(ANDROID)
  find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
  add_library(${PROJECT_NAME} SHARED ${Execution_CPP} ${Execution_H} src/app.qrc)
  set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_BINARY_DIR}/android" CACHE INTERNAL "")
else()
  # Excluding android-only sources
  list(REMOVE_ITEM Execution_H  src/androidwrapper.h src/androidwrapperexception.h src/provider/arcore/arcore.h src/provider/arcore/arcoreworker.h)
  list(REMOVE_ITEM Execution_CPP src/android_jni.cpp src/androidwrapper.cpp src/provider/arcore/arcore.cpp src/provider/arcore/arcoreworker.cpp)
  add_executable(${PROJECT_NAME} ${Execution_CPP} ${Execution_H} src/app.qrc)
endif()

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>)
endif()

include(android.cmake)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME}
  PRIVATE Qt5::Core Qt5::Quick Qt5::Sensors Qt5::3DCore Qt5::3DRender Qt5::3DInput Qt5::3DQuick Qt5::3DLogic Qt5::3DQuickExtras)
target_include_directories(${PROJECT_NAME} PRIVATE "${BUILD_H3DS_INCLUDE_DIR}" "${CMAKE_SOURCE_DIR}/src")

if(ANDROID)
  list(REMOVE_DUPLICATES CUSTOM_ANDROID_EXTRA_LIBS)
  string(REPLACE ";" "," CUSTOM_ANDROID_EXTRA_LIBS_STRING "${CUSTOM_ANDROID_EXTRA_LIBS}")
  set(ANDROID_EXTRA_LIBS "${CUSTOM_ANDROID_EXTRA_LIBS_STRING}" CACHE INTERNAL "")
  message("Additional android libs: ${ANDROID_EXTRA_LIBS}")

  target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras GLESv3)

  # qtcreator cmake android deployment don't want to set sdkBuildToolsRevision
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    BYPRODUCTS notexisting.file # To build the package anyway
    COMMAND ${CMAKE_COMMAND} -E env
      ANDROID_KEYSTORE_FILE="${CMAKE_SOURCE_DIR}/android.keystore"
      QT_ANDROID="${CMAKE_PREFIX_PATH}"
      ANDROID_SDK_BUILD_TOOLS=29.0.1
      ANDROID_NDK_TOOLCHAIN_ABI=${ANDROID_ABI}
      QML_ROOT_PATH="${CMAKE_SOURCE_DIR}"
      sh "${CMAKE_SOURCE_DIR}/tool/build-apk.sh" $<TARGET_FILE:${PROJECT_NAME}> ${ANDROID_EXTRA_LIBS} ${ANDROID_PACKAGE_SOURCE_DIR}
  )
endif()


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
  COMMAND ${CMAKE_COMMAND} -E echo "Ok, package is done"
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

foreach(abi ${BUILD_ABIS})
  set(main_target_name "${PROJECT_NAME}")
  if(NOT abi STREQUAL BUILD_ABI)
    set(main_target_name "${main_target_name}-${abi}-builder")
  endif()
  unset(libraries_to_pack_${abi})
  foreach(dir ${LIBS_LIB_DIRS_${abi}} ${PLUGINS_DIRS_${abi}})
    list(APPEND libraries_to_pack_${abi} ${dir}/*.so)
  endforeach()

  if(ANDROID)
    add_custom_target(prepare_package_${abi} ALL
      COMMAND ${CMAKE_COMMAND} -E copy ${libraries_to_pack_${abi}} android-build/libs/${abi}
      DEPENDS ${main_target_name}
    )

    # Strip binaries for release build
    #if(CMAKE_BUILD_TYPE STREQUAL Release)
    #  add_custom_command(TARGET prepare_package_${abi} PRE_BUILD
    #    COMMAND ${CMAKE_BINARY_DIR}/strip_app_${abi}.exe -s android-build/libs/${abi}/*.so
    #  )
    #endif()

    add_dependencies(apk prepare_package_${abi})
  else()
    # TODO: Check how that will work with a number of ABIS
    add_custom_target(prepare_package_${abi} ALL
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_NAME}-${PROJECT_VERSION}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_NAME}-${PROJECT_VERSION}/bin ${PROJECT_NAME}-${PROJECT_VERSION}/lib
      COMMAND ${CMAKE_COMMAND} -E copy ${libraries_to_pack_${abi}} ${PROJECT_NAME}-${PROJECT_VERSION}/lib
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${main_target_name}> ${PROJECT_NAME}-${PROJECT_VERSION}/bin/${PROJECT_NAME}
      DEPENDS ${main_target_name}
    )

    # Strip binaries for release build
    #if(CMAKE_BUILD_TYPE STREQUAL Release)
    #  add_custom_command(TARGET prepare_package_${abi} PRE_BUILD
    #    COMMAND ${CMAKE_BINARY_DIR}/strip_app_${abi}.exe -s ${PROJECT_NAME}-${PROJECT_VERSION}/bin/* ${PROJECT_NAME}-${PROJECT_VERSION}/lib/*
    #  )
    #endif()
    add_custom_target(package_${abi} ALL
      COMMAND ${CMAKE_COMMAND} -E tar czf ${PROJECT_NAME}-${PROJECT_VERSION}_${abi}.tar.gz ${PROJECT_NAME}-${PROJECT_VERSION}
      DEPENDS prepare_package_${abi}
      BYPRODUCTS ${PROJECT_NAME}-${PROJECT_VERSION}_${abi}.tar.gz
    )
    add_dependencies(package package_${abi})
  endif()

  add_dependencies(prepare_package_${abi} ${PLUGINS_DEPENDS})
endforeach()
