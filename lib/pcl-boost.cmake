# Boost PCL dependency

set(BOOST_VERSION "1.70.0")
set(BOOST_HASH "7804c782deb00f36ac80b1000b71a3707eadb620")
set(BOOST_COMPONENTS system thread filesystem date_time iostreams serialization)

string(REPLACE "." "_" boost_archive_dir "boost_${BOOST_VERSION}")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/src/${boost_archive_dir}")
    message("Downloading boost ${BOOST_VERSION} sources")
    set(boost_url "https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/${boost_archive_dir}.tar.gz")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    file(DOWNLOAD "${boost_url}" "${CMAKE_SOURCE_DIR}/lib/src/boost.tar.gz" EXPECTED_HASH "SHA1=${BOOST_HASH}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "boost.tar.gz" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src")
    execute_process(COMMAND patch -p1 -i "${CMAKE_SOURCE_DIR}/lib/pcl-boost.patch" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/src/${boost_archive_dir}")
endif()

# Copy sources as hardlinks to save some time & disk space
if(NOT EXISTS "${CMAKE_BINARY_DIR}/${boost_archive_dir}")
    message("Exec boost src copy")
    execute_process(COMMAND cp -al "${CMAKE_SOURCE_DIR}/lib/src/${boost_archive_dir}" "${CMAKE_BINARY_DIR}")
endif()

# Run bootstrap with the required components
if(ANDROID)
    string(REPLACE "-" "" boost_arch_compiler ${CMAKE_SYSTEM_PROCESSOR})
    if(ANDROID_ARCH_NAME STREQUAL "arm")
        set(boost_target_os_type "androideabi${ANDROID_PLATFORM_LEVEL}")
    elseif(ANDROID_ARCH_NAME STREQUAL "arm64")
        set(boost_target_os_type "android${ANDROID_PLATFORM_LEVEL}")
    else()
        message(FATAL_ERROR "Unknown android architecture")
    endif()
    file(WRITE "${CMAKE_BINARY_DIR}/${boost_archive_dir}/tools/build/src/user-config.jam"
         "using clang : androidos : ${ANDROID_TOOLCHAIN_ROOT}/bin/${boost_arch_compiler}-linux-${boost_target_os_type}-clang++\n:\n<cxxflags>\"-fPIC -std=c++11 -stdlib=libc++\"\n;")
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/${boost_archive_dir}/b2")
    message("Executing Boost bootstrap...")
    execute_process(COMMAND ./bootstrap.sh "--prefix=${CMAKE_BINARY_DIR}/lib/boost"
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${boost_archive_dir}" RESULT_VARIABLE bootstrap_result ERROR_VARIABLE bootstrap_error)
    if(bootstrap_result GREATER 0)
        message(FATAL_ERROR "Boot bootstrap error (${bootstrap_result}): ${bootstrap_error}")
    endif()
endif()

set(Boost_INCLUDE_DIR "${CMAKE_BINARY_DIR}/${boost_archive_dir}" CACHE STRING "Boost out" FORCE)
set(Boost_LIBRARY_DIR "${CMAKE_BINARY_DIR}/lib/boost/lib" CACHE STRING "Boost out" FORCE)

string(REPLACE ";" "," boost_libs "${BOOST_COMPONENTS}")
foreach(lib ${BOOST_COMPONENTS})
    string(TOUPPER "${lib}" lib_cap)
    set(BOOST_BUILD_FLAGS ${BOOST_BUILD_FLAGS} "--with-${lib}")
    set(boost_products ${boost_products} "${Boost_LIBRARY_DIR}/libboost_${lib}.so")
    set(Boost_${lib_cap}_LIBRARY_RELEASE "${Boost_LIBRARY_DIR}/libboost_${lib}.so" CACHE STRING "Boost out" FORCE)
    set(Boost_${lib_cap}_LIBRARY_DEBUG "${Boost_LIBRARY_DIR}/libboost_${lib}.so" CACHE STRING "Boost out" FORCE)
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} "${Boost_LIBRARY_DIR}/libboost_${lib}.so")
    endif()
    if(ANDROID)
        list(APPEND CUSTOM_ANDROID_EXTRA_LIBS "${Boost_LIBRARY_DIR}/libboost_${lib}.so")
    endif()
endforeach()

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(BOOST_BUILD_FLAGS ${BOOST_BUILD_FLAGS} -j${N})
endif()

if(ANDROID)
    set(BOOST_BUILD_FLAGS ${BOOST_BUILD_FLAGS} target-os=android architecture=arm abi=aapcs)
    if(ANDROID_ARCH_NAME STREQUAL "arm")
        set(BOOST_BUILD_FLAGS ${BOOST_BUILD_FLAGS} address-model=32)
    elseif(ANDROID_ARCH_NAME STREQUAL "arm64")
        set(BOOST_BUILD_FLAGS ${BOOST_BUILD_FLAGS} address-model=64)
    else()
        message(FATAL_ERROR "Unknown android architecture")
    endif()
endif()

add_custom_target(boost COMMAND ./b2 --build-dir=build --debug-configuration -d0 --abbreviate-paths ${BOOST_BUILD_FLAGS} link=shared variant=release threading=multi install
                  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${boost_archive_dir}" BYPRODUCTS ${boost_products})

include_directories(AFTER "${CMAKE_BINARY_DIR}/lib/boost/include")
