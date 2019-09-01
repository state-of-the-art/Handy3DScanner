import QtQuick 2.0
import QtMultimedia 5.8

Rectangle {
    id: root_colorcamera
    color: "#0f0"

    property int aspectRatio: 16.0/9.0

    Camera {
        id: camera

        captureMode: Camera.CaptureStillImage

        //imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceFlash

        /*exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }*/

        //flash.mode: Camera.FlashRedEyeReduction

        imageCapture {
            onImageCaptured: {
                photoPreview.source = preview  // Show the preview in an Image
            }
        }

        Component.onCompleted: {
            camera.start()
            console.log(camera.metaData.pixelAspectRatio)
            console.log(camera.viewfinder.resolution)
            console.log(camera.viewfinder.maximumFrameRate)
            console.log(camera.viewfinder.minimumFrameRate)
            console.log(camera.supportedViewfinderResolutions())
            if( camera.metaData.pixelAspectRatio !== undefined )
                aspectRatio = camera.metaData.pixelAspectRatio
        }
    }

    VideoOutput {
        anchors.fill: parent
        source: camera
        focus : visible // to receive focus and capture key events when visible
    }
}
