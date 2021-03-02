#!/bin/sh

# Install the required build dependencies (imagemagick - images rasterization):
sudo apt update; sudo DEBIAN_FRONTEND=noninteractive apt install -y gnupg2 imagemagick
curl https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
echo 'deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main' | sudo tee /etc/apt/sources.list.d/llvm-10.list
sudo apt update ; sudo apt install -y clang-format-10

# Create build directory:
mkdir -p build out && cd build
# Generate the build scripts (available ABIs: `armeabi-v7a`, `arm64-v8a`) 
cmake ../project -G Ninja -DCMAKE_BUILD_TYPE:STRING=Release "-DANDROID_SDK_BUILD_TOOLS=${ANDROID_SDK_BUILD_TOOLS}" -DANDROID_ABI:STRING=armeabi-v7a -DANDROID_BUILD_ABI_arm64-v8a:BOOL=ON -DANDROID_NATIVE_API_LEVEL:STRING=${ANDROID_NATIVE_API_LEVEL} "-DANDROID_SDK:PATH=${ANDROID_SDK_ROOT}" "-DANDROID_NDK:PATH=${ANDROID_NDK_ROOT}" "-DCMAKE_PREFIX_PATH:PATH=${QT_ANDROID}" "-DCMAKE_FIND_ROOT_PATH:STRING=${QT_ANDROID}" "-DCMAKE_TOOLCHAIN_FILE:PATH=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake"
# Build the binaries:
cmake --build .

# Copy binary to the output dir
cp -a handy3dscanner-*.apk ../out/
