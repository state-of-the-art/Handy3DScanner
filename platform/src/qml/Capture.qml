import QtQuick 2.0
import QtQuick.Layouts 1.3

import VideoPlayer 1.0

Item {
    id: root_capture
    focus: visible

    property string selected_strem: ""

    Rectangle {
        id: monitorArea
        color: "black"
        anchors.fill: parent

        Text {
            anchors.centerIn: parent
            color: "#aaa"
            text: qsTr("Click here to choose the stream")
        }

        VideoPlayer {
            id: monitor
            anchors.fill: parent
            visible: true
        }

        Rectangle {
            anchors.fill: parent
            color: "#fff"
            opacity: 0.0

            SequentialAnimation on opacity {
                id: shot_flash
                running: false
                NumberAnimation {
                    from: 0.0; to: 1.0
                    duration: 25
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: 1.0; to: 0.0
                    duration: 25
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Rectangle {
            visible: cfg['UI.Capture.show_connection_type']
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 3
            width: connection_text.width + 6
            height: connection_text.height + 6
            // TODO: Reenable functionality
            //color: camera.isStreaming ? "#88008800" : "#88880000"
            radius: 3
            Text {
                id: connection_text
                anchors.centerIn: parent
                color: "#fff"
                // TODO: Reenable functionality
                //text: camera.connectionType || "No camera connected"
            }
        }

        Column {
            anchors.top: parent.top
            anchors.right: parent.right
            Rectangle {
                id: stream_fps
                visible: cfg['UI.Capture.show_stream_fps']
                anchors.right: parent.right
                anchors.margins: 3
                width: stream_fps_text.width + 6
                height: stream_fps_text.height + 6
                // TODO: Reenable functionality
                //color: camera.isStreaming ? "#88008800" : "#88880000"
                radius: 3
                Text {
                    id: stream_fps_text
                    anchors.centerIn: parent
                    color: "#fff"
                    // TODO: Reenable functionality
                    //text: (camera.isStreaming ? camera.streamFPS.toFixed(1) : "0.0") + " fps"
                }
            }

            Rectangle {
                id: stream_fwt
                visible: cfg['UI.Capture.show_stream_fwt']
                anchors.right: parent.right
                anchors.margins: 3
                width: stream_fwt_text.width + 6
                height: stream_fwt_text.height + 6
                // TODO: Reenable functionality
                //color: camera.isStreaming ? "#88008800" : "#88880000"
                radius: 3
                Text {
                    id: stream_fwt_text
                    anchors.centerIn: parent
                    color: "#fff"
                    // TODO: Reenable functionality
                    //text: (camera.isStreaming ? camera.streamFWT.toFixed(1) : "0.0") + " fwt"
                }
            }

            Rectangle {
                id: stream_fpt
                visible: cfg['UI.Capture.show_stream_fpt']
                anchors.right: parent.right
                anchors.margins: 3
                width: stream_fpt_text.width + 6
                height: stream_fpt_text.height + 6
                // TODO: Reenable functionality
                //color: camera.isStreaming ? "#88008800" : "#88880000"
                radius: 3
                Text {
                    id: stream_fpt_text
                    anchors.centerIn: parent
                    color: "#fff"
                    // TODO: Reenable functionality
                    //text: (camera.isStreaming ? camera.streamFPT.toFixed(1) : "0.0") + " fpt"
                }
            }
        }

        MiniAxis {
            id: miniaxis
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: parent.width/10
            height: width
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                var res = plugins.listPluginsQML("io.stateoftheart.handy3dscanner.plugins.VideoSourceInterface")
                console.log("DEBUG: Found plugins:", res)
            }
        }
    }

    Rectangle {
        id: shotButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 20

        width: 100
        height: 100
        opacity: 0.5
        // TODO: Reenable functionality
        //color: camera.isStreaming ? "#f00" : "#aaa"
        radius: 50

        // TODO: Reenable functionality
        MouseArea {
            anchors.fill: parent
            // TODO: Reenable functionality
            //enabled: camera.isStreaming
            onPressed: {
                // TODO: Reenable functionality
                //camera.makeShot()
                shot_flash.start()
            }
        }
    }

    // TODO: Reenable functionality
    /*Connections {
        id: cameraConnections
        target: camera
        onNewDepthImage: monitor.setImage(image)
        onIsConnectedChanged: {
            if( camera.isConnected && root_capture.visible )
                camera.start()
        }
        //onErrorOccurred: errorText.text = error
    }*/
}
