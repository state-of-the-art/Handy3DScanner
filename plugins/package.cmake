cmake_minimum_required(VERSION 3.13)

if(NOT ${PROJECT_NAME}-MultiAbiBuild)

  # Add android additions
  include(${CMAKE_CURRENT_LIST_DIR}/android.cmake)

  # Using custom target to build aar since androiddeployqt can't do that
  add_custom_target(aar
    COMMAND ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME} ${QT_DIR}/src/3rdparty/gradle/gradlew
      --no-daemon assembleRelease
    COMMAND ${CMAKE_COMMAND} -E copy 
      "build/outputs/aar/android-build-release.aar"
      "${PROJECT_NAME}.aar"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/android-build
    VERBATIM
  )

  # Packaging
  add_custom_target(package ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Ok, package of plugin ${PROJECT_NAME} is done"
  )

  if(ANDROID)
    # TODO Making an addition to the deployment JSON file
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/android_deployment_settings.json" deploy_data_in)
    string(REPLACE "\"ndk\":"
      "\"sdkBuildToolsRevision\": \"${ANDROID_SDK_BUILD_TOOLS}\",\n  \"ndk\":" deploy_data_in "${deploy_data_in}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/android_deployment_settings.json" "${deploy_data_in}")
    add_custom_command(TARGET package PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy
      "${CMAKE_CURRENT_BINARY_DIR}/android-build/${PROJECT_NAME}.aar"
      "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}_$<JOIN:${BUILD_ABIS},->.aar"
    )
    add_dependencies(package aar)
  endif()

  foreach(abi ${BUILD_ABIS})
    set(main_target_name "${PROJECT_NAME}")
    if(NOT abi STREQUAL BUILD_ABI)
      set(main_target_name "${main_target_name}-${abi}-builder")
    endif()
    unset(libraries_to_pack_${abi})
    foreach(lib ${LIBS_LIST})
      list(APPEND libraries_to_pack_${abi} ${CMAKE_BINARY_DIR}/libs/${lib}/${abi}/lib/*.so)
    endforeach()

    if(ANDROID)
      add_custom_target(prepare_package_${abi} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory android-build/libs/${abi}
        COMMAND ${CMAKE_COMMAND} -E copy ${libraries_to_pack_${abi}} android-build/libs/${abi}
        DEPENDS ${main_target_name}
      )

      # Strip binaries for release build
      #if(CMAKE_BUILD_TYPE STREQUAL Release)
      #  add_custom_command(TARGET prepare_package_${abi} PRE_BUILD
      #    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/strip_app_${abi}.exe -s android-build/libs/${abi}/*.so
      #  )
      #endif()

      add_dependencies(aar prepare_package_${abi})
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
      #    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/strip_app_${abi}.exe -s ${PROJECT_NAME}-${PROJECT_VERSION}/bin/* ${PROJECT_NAME}-${PROJECT_VERSION}/lib/*
      #  )
      #endif()
      add_custom_target(package_${abi} ALL
        COMMAND ${CMAKE_COMMAND} -E tar czf ${PROJECT_NAME}-${PROJECT_VERSION}_${abi}.tar.gz ${PROJECT_NAME}-${PROJECT_VERSION}
        DEPENDS prepare_package_${abi}
        BYPRODUCTS ${PROJECT_NAME}-${PROJECT_VERSION}_${abi}.tar.gz
      )
      add_dependencies(package package_${abi})
    endif()
  endforeach()

endif()
