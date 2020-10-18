import QtQuick 2.0
import QtQuick.Layouts 1.3

import VideoPlayer 1.0

Item {
    id: root_capture
    focus: visible

    Rectangle {
        id: monitorArea
        color: "black"
        anchors.fill: parent

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
            id: current_stream
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 3
            width: current_stream_text.width + 6
            height: current_stream_text.height + 6
            color: monitor.isStreaming ? "#88008800" : "#88880000"
            radius: 3
            Text {
                id: current_stream_text
                anchors.centerIn: parent
                color: "#fff"
                text: monitor.currentStream || qsTr("Click here to choose the stream")
            }

            MouseArea {
                anchors.fill: parent
                onPressed: streams_list.trigger()
            }
        }

        ListView {
            id: streams_list
            visible: false
            anchors {
                top: current_stream.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            width: parent.width
            clip: true

            function trigger() {
                streams_list.visible = !streams_list.visible
                if( streams_list.visible )
                    streams_list.current_path = streams_list.current_path
            }

            property var current_path: []
            onCurrent_pathChanged: {
                var plugins_list = plugins.getInterfacePlugins("io.stateoftheart.handy3dscanner.plugins.VideoSourceInterface")
                var formed_list = []

                if( current_path.length > 0 )
                    formed_list.push({id: qsTr("Go back"), description: ".."})

                console.log("Update current path: " + JSON.stringify(current_path))

                for( var plugin of plugins_list ) {
                    var ret = plugin.getAvailableStreams(current_path)
                    for( var k in ret )
                        formed_list.push({id: k, description: ret[k]})
                    // TODO: Not great if we have a number of plugins
                    if( current_path.length > 0 && Object.keys(ret).length === 0 )
                        monitor.setStream(plugin, current_path)
                }

                if( formed_list.length < 1 )
                    formed_list.push({id: qsTr("There is no devices connected?"), description: null})

                streams_list.model = formed_list
            }

            delegate: Item {
                id: root_item
                width: ListView.view.width
                height: streams_list.height / 10

                Rectangle {
                    id: item
                    anchors.fill: parent
                    border.width: 1
                    color: "#22ffffff"
                    radius: 10
                    clip: true

                    Text {
                        anchors.centerIn: parent
                        color: "#fff"
                        text: {
                            var out_text = ""
                            if( modelData.description && modelData.id !== modelData.description )
                                out_text += modelData.description + ' - '
                            return out_text + modelData.id
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var path = streams_list.current_path
                        if( modelData.description === null )
                            return
                        if( modelData.description === '..' ) {
                            path.pop()
                            streams_list.current_path = path
                            return
                        }
                        path.push(modelData.id)
                        streams_list.current_path = path
                    }
                }
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
