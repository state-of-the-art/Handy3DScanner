# Android dependency files

FUNCTION(PAD_STRING OUT_VARIABLE DESIRED_LENGTH FILL_CHAR VALUE)
  STRING(LENGTH "${VALUE}" VALUE_LENGTH)
  MATH(EXPR REQUIRED_PADS "${DESIRED_LENGTH} - ${VALUE_LENGTH}")
  SET(PAD ${VALUE})
  IF(REQUIRED_PADS GREATER 0)
    MATH(EXPR REQUIRED_MINUS_ONE "${REQUIRED_PADS} - 1")
    FOREACH(FOO RANGE ${REQUIRED_MINUS_ONE})
      SET(PAD "${FILL_CHAR}${PAD}")
    ENDFOREACH()
  ENDIF()
  SET(${OUT_VARIABLE} "${PAD}" PARENT_SCOPE)
ENDFUNCTION()

if(ANDROID)
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/android-build" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
  file(READ "${CMAKE_CURRENT_BINARY_DIR}/android-build/AndroidManifest.xml" manifest_data)

  # Replace PROJECT_VERSION
  string(REPLACE "{{ PROJECT_VERSION }}" "${PROJECT_VERSION}" manifest_data "${manifest_data}")

  # Replace PROJECT_VERSION_CODE
  string(REPLACE "." ";" version_list "${PROJECT_VERSION}")
  foreach(version_part ${version_list})
    PAD_STRING(version_part_pad 2 "0" "${version_part}")
    set(version_code "${version_code}${version_part_pad}")
  endforeach(version_part)
  string(REPLACE "{{ PROJECT_VERSION_CODE }}" "${version_code}" manifest_data "${manifest_data}")

  # Write AndroidManifest.xml back
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/android-build/AndroidManifest.xml" "${manifest_data}")

  # Write gradle config files
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/android-build/gradle.properties" "
android.bundle.enableUncompressedNativeLibs=false
androidBuildToolsVersion=
androidCompileSdkVersion=29
buildDir=build
")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/android-build/local.properties" "
ndk.dir=${ANDROID_NDK}
sdk.dir=${ANDROID_SDK}
")
endif()
