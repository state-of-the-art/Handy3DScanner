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

            property var current_path: []
            property var plugin_streams_tree

            function trigger() {
                streams_list.visible = !streams_list.visible
                if( streams_list.visible )
                    streams_list.current_path = streams_list.current_path // to trigger the path changed
            }

            onCurrent_pathChanged: {
                var formed_list = []

                console.log("Update current path: " + JSON.stringify(current_path))

                // Finding the destination
                var dest = { "childrens": plugin_streams_tree } // To simplify the logic
                for( var key of current_path ) {
                    dest = dest["childrens"][key]

                    if( dest === undefined ) {
                        app.error("Unable to locate path destination: " + key)
                        current_path = []
                        return
                    }

                    // Dest has no childrens - so it's the path end and we can use it as stream path
                    if( dest["childrens"] === undefined ) {
                        monitor.setStream(current_path)
                        trigger()
                        current_path = []
                        return
                    }
                }

                if( current_path.length == 0 ) {
                    // Update the device tree
                    var plugins_list = plugins.getInterfacePlugins("io.stateoftheart.handy3dscanner.plugins.VideoSourceInterface")

                    plugin_streams_tree = {}
                    dest = { "childrens": plugin_streams_tree } // TODO: Remove duplication
                    for( var plug of plugins_list ) {
                        plugin_streams_tree[plug.name()] = { "childrens": plug.getAvailableStreams() }
                    }
                }

                // Walking through destintation and forming the display list
                for( var id in dest["childrens"] )
                    formed_list.push(Object.assign(dest["childrens"][id], {id: id}))

                if( formed_list.length === 0 )
                    formed_list.push({ name: qsTr("Nothing to display"), description: qsTr("There is no devices connected?") })

                formed_list.sort( function(a,b) {
                    // Sort by status
                    if( a.status !== b.status )
                        return b.status - a.status

                    // Natural sort by num 9999-1-A
                    var num_a = parseInt(a.name || a.id)
                    var num_b = parseInt(b.name || b.id)
                    if( num_a !== NaN && num_b !== NaN )
                        return num_a > num_b ? -1 : 1

                    // Usual sort A-Z
                    return (a.name || a.id) > (b.name || b.id) ? -1 : 1
                } )

                if( current_path.length > 0 )
                    formed_list.unshift({ id: "..", description: qsTr("Go up") })

                streams_list.model = formed_list
                console.log("Update current path done")
            }

            delegate: Item {
                // WARNING: Do not modify modelData.childrens
                id: root_item
                width: ListView.view.width
                height: streams_list.height / 10

                Rectangle {
                    id: item
                    anchors.fill: parent
                    border.width: 1
                    color: "#88ffffff"
                    radius: 10
                    clip: true

                    Rectangle {
                        id: stream_status
                        height: width
                        width: parent.height-(parent.height/4)
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 5
                        color: {
                            switch(modelData.status) {
                                case 0: return "#000"  // NOT_SUPPORTED
                                case 2: return "#0a0"  // ACTIVE
                                case 3: return "#a00"  // CAPTURE
                                default: return "#000"
                            }
                        }
                        radius: 100
                        clip: true
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: stream_status.right
                        anchors.leftMargin: 5
                        color: modelData.status !== 0 ? "#000" : "#333"
                        font.bold: modelData.status !== 0
                        text: {
                            var out_text = modelData.name || modelData.id
                            if( modelData.description )
                                out_text += " (" + modelData.description + ")"
                            if( modelData.status === 0 )
                                out_text += " (not implemented by the plugin)"
                            return out_text
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: modelData.status !== 0
                    onClicked: {
                        var path = streams_list.current_path
                        if( modelData.id === undefined )
                            return
                        if( modelData.id === '..' ) {
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
