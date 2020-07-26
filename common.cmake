set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Set universal BUILD_ABI & BUILD_ABIS
if(ANDROID)
  set(BUILD_ABI ${ANDROID_ABI})
  foreach(abi armeabi-v7a arm64-v8a x86 x86_64)
    if(abi STREQUAL ${BUILD_ABI})
      list(APPEND BUILD_ABIS ${abi})
    elseif(ANDROID_BUILD_ABI_${abi})
      list(APPEND BUILD_ABIS ${abi})
    endif()
  endforeach()
else()
  set(BUILD_ABI ${CMAKE_SYSTEM_PROCESSOR})
  set(BUILD_ABIS "${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(BUILD_H3DS_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/platform" CACHE STRING "Handy 3D Scanner source directory for plugins" FORCE)
set(BUILD_H3DS_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include" CACHE STRING "Handy 3D Scanner include directory for plugins" FORCE)
set(BUILD_H3DS_PLUGIN_PREFIX "h3ds-plugin-")
