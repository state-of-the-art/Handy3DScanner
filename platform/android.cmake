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
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/android" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
  file(READ "${CMAKE_CURRENT_BINARY_DIR}/android/AndroidManifest.xml" manifest_data)

  # Replace PROJECT_VERSION
  string(REPLACE "{{ PROJECT_VERSION }}" "${PROJECT_VERSION}" manifest_data "${manifest_data}")

  # Replace PROJECT_VERSION_CODE
  string(REPLACE "." ";" version_list "${PROJECT_VERSION}")
  foreach(version_part ${version_list})
    PAD_STRING(version_part_pad 2 "0" "${version_part}")
    set(version_code "${version_code}${version_part_pad}")
  endforeach(version_part)
  string(REPLACE "{{ PROJECT_VERSION_CODE }}" "${version_code}" manifest_data "${manifest_data}")

  # Replace PROJECT_ARCH_CODE
  if(ANDROID_ABI STREQUAL "x86")
    set(arch_code 0)
  elseif(ANDROID_ABI STREQUAL "x86_64")
    set(arch_code 1)
  elseif(ANDROID_ABI STREQUAL "mips")
    set(arch_code 2)
  elseif(ANDROID_ABI STREQUAL "mips64")
    set(arch_code 3)
  elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(arch_code 4)
  elseif(ANDROID_ABI STREQUAL "arm64-v8a")
    set(arch_code 5)
  else()
    message(SEND_ERROR "Unable to set the PROJECT_ARCH_CODE ${ANDROID_ABI}")
  endif()
  string(REPLACE "{{ PROJECT_ARCH_CODE }}" "${arch_code}" manifest_data "${manifest_data}")

  # Write AndroidManifest.xml back
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/android/AndroidManifest.xml" "${manifest_data}")

  # Generating icons
  set(icon_svg "${CMAKE_CURRENT_SOURCE_DIR}/icon.svg")

  set(icon_res  ldpi mdpi hdpi xhdpi xxhdpi xxxhdpi)
  set(icon_size 36   48   72   96    144    192)
  list(LENGTH icon_res icon_number)
  math(EXPR icon_number_iter "${icon_number} - 1")

  foreach(i RANGE ${icon_number_iter})
    list(GET icon_res ${i} res)
    list(GET icon_size ${i} size)
    set(icon_dir "${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-${res}")
    file(MAKE_DIRECTORY "${icon_dir}")
    add_custom_target("${res}_out" DEPENDS "${icon_dir}/icon.png")
    add_custom_command(OUTPUT "${icon_dir}/icon.png"
                       COMMAND convert -background none -density 553 -resize "${size}x${size}" "${icon_svg}" "${icon_dir}/icon.png"
                       MAIN_DEPENDENCY "${icon_svg}")
    add_dependencies(${PROJECT_NAME} "${res}_out")
  endforeach(i)
endif()
