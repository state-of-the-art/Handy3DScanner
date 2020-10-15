# Handy 3D Scanner

[![Join the chat at https://gitter.im/state-of-the-art/Handy-3D-Scanner](https://badges.gitter.im/state-of-the-art/Handy-3D-Scanner.svg)](https://gitter.im/state-of-the-art/Handy-3D-Scanner?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

General purpose 3d scaner based on Intel Realsense D400 cameras.

Application to help build a cheap 3d scanner based on Intel RealSense D415/D435 cameras and your android smartphone, tablet or laptop.

* [Site page about it](https://www.state-of-the-art.io/projects/handy-3d-scanner/)
* [GooglePlay store](https://play.google.com/store/apps/details?id=io.stateoftheart.handy3dscanner)

## Usage

Please check out the wiki page: [Wiki](https://github.com/state-of-the-art/Handy3DScanner/wiki)

## Requirements

* Host:
  * **Smartphone / Tablet** - anything with Android 5+ (Lollipop or better) and usb3 or usb2 host (camera need ~1000mA)
  * **Linux workstation** - Ubuntu LTS, Debian, CentOS...
  * **Linux SBC** - for small boards it also should work
* **Intel Realsense D400** - stereo camera with D4 board (right tested D415 and D435i)
* **USB-C/USB-C cable** - USB3.1 Gen 1 should work properly or you can use usb2 for low speed capture
* **Handy 3D Scanner** - application piece, binaries published in Android store (paid) and available for workstations (opensource).

## Application

### Overview

Basically have just 3 main functions:
* **Capture** - allow to get shots and records of the environment
  * General mode
    you can take pointclouds capturing your surrounding with a proper positioning of the
    shots (gyroscope or arcore positioning).
  * Record mode (*in dev*)
    Useful for experimenting with AI, but consumes alot of memory
* **Preview** - shows what `pc` you captured
  * List of the captured `pc`
  * Show/Hide pointclouds
  * Camera controls: rotation, zoom, focus to point
* **Load/Save** - when we would like to view or store `pc` or mesh
  * Individual pointclouds - store in PCD format as point of clouds for the further processing
  * Export visible - save the whole 3d scene as glTF 2.0 glb file to import it in 3d edit software or share

Also there will be settings, skeletal animation and other stuff.

### Applications

* VR/AR avatars,  objects - scan yourself, your cat or entire house and bring them to VR/AR.
* General purpose portable 3d scanner for 3d printer - if you want to build a copy of object you like.
* Modelling - sometimes it's much easier to scan and edit in 3d editor (Blender / 3DSMax).
* Measuring - it's pretty accurate, so you can use it to measure stuff or proportions.
* 3d Instagram - hopefully we will get to this state :)
* Who knows - when such techs are going wild, it's hard to predict how they will be used.

### Features

* Easy pointcloud capture interface
* Pointcloud preview and edit
* Support for RealSense library
* Support for ARCore library
* Save & load in PCD format
* Export to glTFv2 with Draco compression

### Example data

You can find the examples & PCD/glb files on the wiki page: [Example data](https://github.com/state-of-the-art/Handy3DScanner/wiki/Example-data)

## Price

* Professional 3d scanners: ~$10'000 - $20'000.
* Table laser scanners: ~$70 - $500.

With Intel Realsense D415 (~$140) and mobile app we can get a simple and cheap solution to provide a really
cheap mid-range HD solution with advanced specifications:
* Resolution: 1Mpix (1280x720)
* Frame-rate: 30-60 fps
* Angle: 63°x40°, 85°x58°
* Range: 16cm-10m, 11cm-10m

Means that finally for ~$200 users can get simple solution to build second instagram, now in 3D - and the
last piece is just a userspace software.

### Plans

You can see all the feature requests/bugs on the github page:

* [Milestones](https://github.com/state-of-the-art/Handy3DScanner/milestones)
* [Issues](https://github.com/state-of-the-art/Handy3DScanner/issues)

But overall we have a huge plans to make this application better!
* **Prepare version 1.0** - it should be useful for everyday use.
* **Kickstarter campagin** - customers want to see the complete product - so why not?
* **Publish source code** - necessary to make sure the project will live it's long life.
* **Reach the top** - the market could be bigger, any smartphone should get this feature
  because AR/VR is coming (check Samsung Galaxy Note 10). Handy 3D Scanner can help with
  building the new future where everyone will take not just photo, but capture part of
  the 3d world and place it in VR.

## OpenSource

This is an experimental project - main goal is to test State Of The Art philosophy on practice.

We would like to see a number of independent developers working on the same project issues
for the real money (attached to the ticket) or just for fun. So let's see how this will work.

### License

Repository and it's content is covered by `Apache v2.0` - so anyone can use it without any concerns.

If you will have some time - it will be great to see your changes merged to the original repository -
but it's your choise, no pressure.

### Build

Build process is quite hard, but requires a minimum dependencies (cmake will get all the requirements
automatically).

*NOTICE*: Qt 5.12.4, 5.12.5 and 5.13.0 have an issue with gyro (QTBUG-77423) - so please use 5.12.3
if you want to use gyroscope.

#### Build in docker

##### For desktop

1. Clone the repository:
    ```
    host$ git clone https://github.com/state-of-the-art/Handy3DScanner.git ~/Build/Handy3DScanner
    ```
2. Run the docker container:
    ```
    host$ cd ~/Build/Handy3DScanner
    host$ docker run -it --rm --name h3ds-build --volume="${PWD}:/home/user/project" rabits/qt:5.13-desktop
    ```
3. Install the required dependencies:
    ```
    docker$ sudo apt update
    docker$ sudo apt install -y libusb-1.0-0-dev
    ```
4. Create build directory:
    ```
    docker$ mkdir project/build
    docker$ cd project/build
    ```
5. Generate the build scripts
    ```
    docker$ cmake .. -G Ninja "-DCMAKE_PREFIX_PATH:PATH=${QT_DESKTOP}"
    ```
6. Build the binaries:
    ```
    docker$ cmake --build .
    ```
7. You can find the compiled binaries in the `build` directory

##### For android

1. Clone the repository:
    ```
    host$ git clone https://github.com/state-of-the-art/Handy3DScanner.git ~/Build/Handy3DScanner
    ```
2. Run the docker container (use `-armv7` if you need armv7 binaries):
    ```
    host$ cd ~/Build/Handy3DScanner
    host$ docker run -it --rm --name h3ds-build --volume="${PWD}:/home/user/project" rabits/qt:5.13-android-arm64
    ```
3. Install the required dependencies (build-essential for boost build system):
    ```
    docker$ sudo apt update
    docker$ sudo apt install -y imagemagick build-essential
    ```
4. Create build directory:
    ```
    docker$ mkdir project/build
    docker$ cd project/build
    ```
5. Generate the build scripts
    ```
    docker$ cmake .. -G Ninja "-DCMAKE_PREFIX_PATH:PATH=${QT_ANDROID}" "-DCMAKE_TOOLCHAIN_FILE:PATH=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake" "-DANDROID_ABI:STRING=${ANDROID_NDK_TOOLCHAIN_ABI}" -DANDROID_NATIVE_API_LEVEL:STRING=29
    ```
6. Build the binaries:
    ```
    docker$ cmake --build .
    ```
7. Debug APK will be created automatically with help of `tools/build-apk.sh` - and you will see where it's 

#### Build on host

You can use your host:

##### Dependencies

* Ubuntu 18.04 (build is tested only using ubuntu, but you can try something else)
* CMake 3.10
* Ninja (optional, but helpful)
* Qt SDK 5.12 or 5.13
* build-essential (is needed to build host binaries, even for android boost requires to compile the build system)
* android:
  * Android SDK android-29 (actually could be built on 21-29 API levels)
  * Android NDK r20
  * Imagemagick (using `convert` to generate png out of svg)
* desktop:
  * libusb-1.0-0-dev (on android we using jni interface, but on desktop the native one)

##### Variables

Already set in the docker images, but you need to set them to build on the host system (there is an examples, you need to choose yours):

* `QT_DESKTOP`: "~/local/Qt/5.13.0/gcc_64" - path to the Qt desktop binaries
* `QT_ANDROID`: "~/local/Qt/5.13.0/android_armv7" - path to the Qt android binaries
* `ANDROID_NDK_PLATFORM`: "android-29" - what the platform to use while android apk build
* `ANDROID_NDK_ROOT`: "~/local/android-sdk/ndk-bundle" - path to the Android NDK
* `ANDROID_NDK_TOOLCHAIN_ABI`: "armeabi-v7a", "arm64-v8a" - binary type

To build the APK for android you need to set the next env variables in addition:

* `ANDROID_SDK_ROOT`: "/opt/android-sdk" - path to the android sdk
* `ANDROID_NDK_HOST`: "linux-x86_64" - ndk host platform
* `ANDROID_SDK_BUILD_TOOLS`: "29.0.1" - version of the sdk build-tools will be used

##### Build

Just follow the docker instructions (but without docker) - and you will be good.

## Privacy policy

It's very important to save user private data and you can be sure: we working on security
of our applications and improving it every day. No data could be sent somewhere without
user notification and his direct approve. This application will work standalone without
any internet connection and will not collect any user personal data anyway.
