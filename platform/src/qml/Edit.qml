import QtQuick 2.12

Item {
    id: root_edit
    focus: visible

    Rectangle {
        id: pointCloudViewArea
        color: "black"
        anchors.fill: parent

        PointCloudView {
            id: pointCloudView
            anchors.fill: parent
            visible: root_edit.visible
        }

        Rectangle { // Camera center point
            SequentialAnimation on color {
                loops: Animation.Infinite
                ColorAnimation { from: "#aaff0000"; to: "#ff000000"; duration: 250 }
                ColorAnimation { from: "#ff000000"; to: "#aaffff00"; duration: 250 }
                ColorAnimation { from: "#aaffff00"; to: "#00000000"; duration: 250 }
                ColorAnimation { from: "#00000000"; to: "#aaff0000"; duration: 250 }
            }
            width: parent.width/200
            height: width
            radius: width
            x: parent.width/2 - width/2
            y: parent.height/2 - width/2
        }

        Text {
            id: notify_text
            anchors.centerIn: parent
            color: "#aaa"
            text: qsTr("Select captured/opened pointcloud from the list")
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons

        property real prevMouseX: 0
        property real prevMouseY: 0
        property bool preclick: false
        property vector2d preclick_point

        onPressed: {
            prevMouseX = mouse.x
            prevMouseY = mouse.y
            console.log("Mouse pressed")
            if( mouse.button == Qt.LeftButton ) {
                preclick = true
                preclick_point = Qt.vector2d(mouse.x, mouse.y)
            }
        }

        onReleased: {
            prevMouseX = 0
            prevMouseY = 0
            console.log("Mouse released: " + Qt.vector2d(mouse.x, mouse.y).minus(preclick_point).length())
            if( preclick ) {
                console.log("Casting screen ray")
                pointCloudView.caster.trigger(Qt.point(mouse.x, mouse.y))
            }
            preclick = false
        }

        onPositionChanged: {
            if( preclick ) {
                if( Qt.vector2d(mouse.x, mouse.y).minus(preclick_point).length() < 1.0 )
                    return
                preclick = false
            }

            if( mouse.buttons === Qt.RightButton ) { // Camera up-down left-right
                // up-down
                var addition = pointCloudView.cameraView.upVector.times((mouse.y-prevMouseY)/50)

                // left-right
                var v1 = pointCloudView.cameraView.upVector
                var v2 = pointCloudView.cameraView.viewVector.normalized()
                var leftVector = Qt.vector3d(v1.y * v2.z - v1.z * v2.y,
                                             v1.z * v2.x - v1.x * v2.z,
                                             v1.x * v2.y - v1.y * v2.x)
                addition = addition.plus(leftVector.times((mouse.x-prevMouseX)/50))

                pointCloudView.cameraView.position = pointCloudView.cameraView.position.plus(addition)
                pointCloudView.cameraView.viewCenter = pointCloudView.cameraView.viewCenter.plus(addition)
            } else if( mouse.buttons === Qt.MiddleButton ) {
                // Camera rotation around view center
                pointCloudView.cameraView.panAboutViewCenter(-mouse.x+prevMouseX, Qt.vector3d( 0.0, -1.0, 0.0 ))
                pointCloudView.cameraView.tiltAboutViewCenter(mouse.y-prevMouseY)
            }

            prevMouseX = mouse.x
            prevMouseY = mouse.y
        }

        onWheel: {
            var delta = pointCloudView.cameraView.viewVector.times(Math.sign(wheel.angleDelta.y)).times(0.5)
            console.log(pointCloudView.cameraView.viewVector.length() + " " + wheel.angleDelta.y)
            if( wheel.angleDelta.y > 0 && pointCloudView.cameraView.viewVector.length() < pointCloudView.cameraView.nearPlane * 10 )
                return
            else if( wheel.angleDelta.y < 0 && pointCloudView.cameraView.viewVector.length() > pointCloudView.cameraView.farPlane )
                return
            // Modify camera view center if mouse button is pressed
            if( wheel.buttons === 0 )
                pointCloudView.cameraView.position = pointCloudView.cameraView.position.plus(delta)
            else
                pointCloudView.cameraView.viewCenter = pointCloudView.cameraView.viewCenter.plus(delta)
        }
    }

    MultiPointTouchArea {
        anchors.fill: parent
        mouseEnabled: false
        touchPoints: [
            TouchPoint { id: point1 },
            TouchPoint { id: point2 }
        ]

        property bool preclick: false
        property vector2d preclick_point
        property real init_between: 0
        property vector3d init_cam_pos
        property vector2d move_start

        onPressed: {
            console.log("Multi-touch pressed: " + touchPoints.length)
            if( point1.pressed && ! point2.pressed ) {
                preclick = true
                preclick_point = Qt.vector2d(point1.x, point1.y)
            } else
                preclick = false
            if( point1.pressed && point2.pressed ) {
                init_between = Qt.vector2d(point2.x-point1.x, point2.y - point1.y).length()
                init_cam_pos = pointCloudView.cameraView.position
                move_start = Qt.vector2d(0, 0)
            }
        }

        onReleased: {
            console.log("Multi-touch released: " + touchPoints.length)
            if( preclick )
                pointCloudView.caster.trigger(Qt.point(point1.x, point1.y))
            preclick = false
            init_between = 0
        }

        onUpdated: {
            if( preclick ) {
                if( Qt.vector2d(point1.x, point1.y).minus(preclick_point).length() < 10.0 )
                    return
                preclick = false
            }

            if( point2.pressed ) {
                // Camera movement
                if( point1.pressed ) { // Camera front-backward
                    var between = -0.1*(init_between-Qt.vector2d(point2.x-point1.x, point2.y - point1.y).length())
                    //var add_pos = init_cam_vector.times(-0.1*(init_between - between)).times(0.5)
                    var vector = pointCloudView.cameraView.viewVector
                    if( between > 0 && vector.length() < pointCloudView.cameraView.nearPlane * 2 )
                        return
                    else if( between < 0 && vector.length() > pointCloudView.cameraView.farPlane )
                        return

                    var func_val, add_pos
                    if( between < 0 ) { // Zoom out
                        func_val = Math.min(1.0, between*between*0.01*0.0625)*0.25 // 0 to inf
                        // From far point to start point
                        var far_point = pointCloudView.cameraView.viewCenter.minus(vector.normalized().times(pointCloudView.cameraView.farPlane))
                        add_pos = init_cam_pos.times(1.0-func_val).plus(far_point.times(func_val))
                    } else { // Zoom in
                        func_val = 1.0 - 1.0/(1.0+between) // 0 to 1
                        // From start point to center point
                        add_pos = init_cam_pos.times(1.0-func_val).plus(pointCloudView.cameraView.viewCenter.times(func_val))
                    }
                    pointCloudView.cameraView.position = add_pos

                } else { // Camera up-down left-right
                    // TODO: Delay before move
                    if( move_start.x < 0.001 && move_start.y < 0.001 )
                        move_start = Qt.vector2d(point2.x, point2.y)
                    if( move_start.minus(Qt.vector2d(point2.x, point2.y)).length() < 30.0 )
                        return
                    // up-down
                    var addition = pointCloudView.cameraView.upVector.times((point2.y-point2.previousY)/100)

                    // left-right
                    var v1 = pointCloudView.cameraView.upVector
                    var v2 = pointCloudView.cameraView.viewVector.normalized()
                    var leftVector = Qt.vector3d(v1.y * v2.z - v1.z * v2.y,
                                                 v1.z * v2.x - v1.x * v2.z,
                                                 v1.x * v2.y - v1.y * v2.x)
                    addition = addition.plus(leftVector.times((point2.x-point2.previousX)/100))

                    pointCloudView.cameraView.position = pointCloudView.cameraView.position.plus(addition)
                    pointCloudView.cameraView.viewCenter = pointCloudView.cameraView.viewCenter.plus(addition)
                }
            } else if( point1.pressed ) { // Camera rotation
                pointCloudView.cameraView.panAboutViewCenter(-point1.x+point1.previousX, Qt.vector3d( 0.0, -1.0, 0.0 ))
                pointCloudView.cameraView.tiltAboutViewCenter(point1.y-point1.previousY)
            }
        }
    }

    Keys.onPressed: {
        event.accepted = true
        console.log(event.key)
        switch( event.key ) {
            case Qt.Key_Left:
                console.log("Key: left")
                break;
            case Qt.Key_Right:
                console.log("Key: right")
                break;
            case Qt.Key_Up:
                console.log("Key: up")
                break;
            case Qt.Key_Down:
                console.log("Key: down")
                break;
            default:
                event.accepted = false
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 3
        width: primitive_view_text.width + 6
        height: primitive_view_text.height + 6
        color: "#88008800"
        radius: 3
        Text {
            id: primitive_view_text
            anchors.centerIn: parent
            color: "#fff"
            text: cfg['UI.Edit.display_mode']
        }
        MouseArea {
            anchors.fill: parent
            onReleased: {
                // TODO: Replace to popup settings menu: https://github.com/state-of-the-art/Handy3DScanner/issues/57
                var key_path = 'UI.Edit.display_mode'
                var list = cfg.opt(key_path).list
                cfg.setVal(key_path, list[(list.indexOf(cfg[key_path])+1) % list.length])
            }
        }
    }

    EditMenu {
        id: editMenu
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        width: parent.width / 20
        onCurrentToolChanged: {
            pointCloudView.setTool(currentTool.objectName)
        }
        onShowPointCloud: notify_text.visible = false
    }

    Component.onCompleted: {
        // Linking menu & view
        editMenu.showPointCloud.connect(pointCloudView.showPointCloud)
        editMenu.hidePointCloud.connect(pointCloudView.hidePointCloud)
    }
}
