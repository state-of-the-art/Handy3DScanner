# ARCore dependency
# URL bin: https://mvnrepository.com/artifact/com.google.ar/core/${ARCORE_VERSION}
# URL headers: https://github.com/google-ar/arcore-android-sdk/blob/v${ARCORE_VERSION}/libraries/include/arcore_c_api.h

if(ANDROID)
    set(ARCORE_VERSION "1.15.0")
    set(ARCORE_AAR_HASH "87b62bd8628f7aa380b7d069dfba511ce0a7419f")
    set(ARCORE_HEADER_HASH "635e483cfb1816da4ec6595e275ac474c0464d19")

    set(arcore_aar_url "https://maven.google.com/com/google/ar/core/${ARCORE_VERSION}/core-${ARCORE_VERSION}.aar")
    set(arcore_header_url "https://raw.githubusercontent.com/google-ar/arcore-android-sdk/v${ARCORE_VERSION}/libraries/include/arcore_c_api.h")

    message("Downloading arcore ${ARCORE_VERSION} binary")
    file(DOWNLOAD "${arcore_aar_url}" "${CMAKE_BINARY_DIR}/arcore/core.aar" EXPECTED_HASH "SHA1=${ARCORE_AAR_HASH}")
    file(DOWNLOAD "${arcore_header_url}" "${CMAKE_BINARY_DIR}/arcore/include/arcore_c_api.h" EXPECTED_HASH "SHA1=${ARCORE_HEADER_HASH}")

    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_BINARY_DIR}/arcore/core.aar" WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/arcore)

    target_link_libraries(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/arcore/jni/${ANDROID_ABI}/libarcore_sdk_c.so")
    include_directories(AFTER "${CMAKE_BINARY_DIR}/arcore/include")

    list(APPEND CUSTOM_ANDROID_EXTRA_LIBS "${CMAKE_BINARY_DIR}/arcore/jni/${ANDROID_ABI}/libarcore_sdk_c.so")
    file(COPY "${CMAKE_BINARY_DIR}/arcore/classes.jar" DESTINATION "${CMAKE_BINARY_DIR}/android/libs")
endif()

