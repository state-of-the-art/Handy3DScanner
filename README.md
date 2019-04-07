# Handy 3D Scanner

General purpose 3d scaner based on Intel Realsense D400 cameras.

Right now just a repo to track the current plans - but later the source code will be published here.

* [Site page about it](https://www.state-of-the-art.io/projects/handy-3d-scanner/)
* [GooglePlay store](https://play.google.com/store/apps/details?id=io.stateoftheart.handy3dscanner)

## Requirements

* Host:
  * **Smartphone** - anything with Android 5+ (Lollipop or better) and usb3 host is ok (*later usb2 will be supported*)
  * **Linux workstation** - Ubuntu LTS, Debian, CentOS...
* **Intel Realsense D400** - stereo camera with D4 board (right now tested D415, but D435 also should be ok)
* **USB-C/USB-C cable** - USB3.1 Gen 1 should work properly (*seems could also work on usb2 + 5V1A cable*)
* **Handy 3D Scanner** - application piece, published in Android store and available for workstations.

## Application

### Overview

Basically have just 3 main functions:
* **Capture** - allow to get shots and records of the environment
  * Single shot mode -
    Just getting shot of the current view and converting it to pointcloud (`pc`)
  * Record mode (*in dev*) -
    Useful for experimenting with AI, but consumes alot of memory
  * Walk-around mode (*in dev*) -
    to capture relatively small objects and humans just walking around and taking pictures
  * Panorama mode (*uses phone `gyro` to get panorama*) -
    allow to capture panoramas & huge environments around the camera
* **View/Edit** - shows what `pc` you have and allow to edit them
  * List of the captured `pc`
  * Show/Hide `pc` from the list
  * Camera controls: rotation, zoom, focus to point
  * Selection of `pc` points: single, line, box
  * Hide points: selected or unselected
  * Delete points: selected or unselected
* **Load/Save** - when we would like to view or store `pc` or mesh

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
* Save captured pc in PCD format (to internal app dir)
* Simple pc edit tools
* Gyro-based panoramic shots (need improvement)

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
  because AR/VR is coming. So Handy 3D Scanner can help with building the new future where
  everyone will take not just photo, but capture part of the 3d world and place it in VR.

### OpenSource

We have plans to publish full source code on github to make sure application will live for a long time.

This is an exprerimental project - main goal is to test State Of The Art philosophy on practice.

So for now we starting with closed source & Android PlayStore, but when there will
be something to show - source code will be opened and we will start to actually search developers
in opensource community to help with implementation some non-standard logic with `pc` procesing.

## Privacy policy

It's very important to save user private data and you can be sure: we working on security
of our applications and improving it every day. No data could be sent somewhere without
user notification and his direct approve. This application will work standalone without
any internet connection and will not collect any user personal data anyway.
