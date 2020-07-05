# Handy 3D Scanner

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

* Easy capture from Intel Realsense D415/D435
* Full available camera resolution (D415 1280x720 points)
* Preview of the captured point clouds (pc)
* Save captured pc in PCD format and export scene to glTF 2.0
* ARCore-based camera positioning

### Example data

You can find the examples & PCD/glb files on the wiki page: [Example data](https://github.com/state-of-the-art/Handy3DScanner/wiki/Example-data)

## Price

* Professional 3d scanners: ~$10'000 - $20'000.
* Table laser scanners: ~$70 - $500.

With Intel Realsense D415 (~$140) and mobile app we can get a really cheap mid-range HD scanner solution:
* Resolution: 1Mpix (1280x720)
* Frame-rate: 30-60 fps
* Angle: 63°x40°, 85°x58°
* Range: 16cm-10m, 11cm-10m

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

1. Clone the repository:
    ```
    $ git clone https://github.com/state-of-the-art/Handy3DScanner.git && mkdir -p out
    ```
2. Run the docker container and build the project:
    * Linux desktop `x86_64`:
        ```
        $ docker run -it --rm -v "${PWD}/Handy3DScanner:/home/user/project:ro" -v "${PWD}/out:/home/user/out:rw" rabits/qt:5.14-desktop ./project/tool/build-docker-linux.sh
        ```
    * Android `multiarch`:
        ```
        $ docker run -it --rm -v "${PWD}/Handy3DScanner:/home/user/project:ro" -v "${PWD}/out:/home/user/out:rw" rabits/qt:5.14-android ./project/tool/build-docker-android.sh
        ```
3. Results will be placed in the `out` directory

## Privacy policy

It's very important to save user private data and you can be sure: we working on security
of our applications and improving it every day. No data could be sent somewhere without
user notification and his direct approve. This application will work standalone without
any internet connection and will not collect any user personal data anyway.
