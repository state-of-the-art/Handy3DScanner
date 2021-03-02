#!/bin/sh

# Install the required build dependencies (libusb - librealsense):
sudo apt update; sudo DEBIAN_FRONTEND=noninteractive apt install -y gnupg2 libusb-1.0-0-dev
curl https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
echo 'deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main' | sudo tee /etc/apt/sources.list.d/llvm-10.list
sudo apt update ; sudo apt install -y clang-format-10

# Create build directory:
mkdir -p build out && cd build
# Generate the build scripts (ABI:`x86_64`) 
cmake ../project -G Ninja -DCMAKE_BUILD_TYPE:STRING=Release "-DCMAKE_PREFIX_PATH:PATH=${QT_DESKTOP}"
# Build the binaries:
cmake --build .

# Copy binary to the output dir
cp -a handy3dscanner-*.tar.gz ../out/
