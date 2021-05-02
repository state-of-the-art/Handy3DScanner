import QtQuick 2.12
import QtQml.Models 2.12
import QtGraphicalEffects 1.12

Item {
    id: root
    height: buttonsList.implicitHeight

    property var currentTool: defaultTool
    property var menuList: [ toolsMenu, selectionMenu ]
    property bool toolsInversed: false
    property var selectedPointClouds: []

    signal showPointCloud(var pc)
    signal hidePointCloud(var pc)

    function closeMenus() {
        for( var i in menuList )
            menuList[i].closeMenu()
    }

    Column {
        id: buttonsList
        anchors.fill: parent

        EditMenuButton {
            width: parent.width
            caption: "\uE806"
            description: "Captured point clouds list"

            menu: Item {
                width: 200
                height: pointCloudsList.contentHeight < root.parent.height*0.5 ? pointCloudsList.contentHeight : root.parent.height*0.5
                ListView {
                    id: pointCloudsList
                    anchors.fill: parent
                    model: camera.pointclouds
                    clip: true

                    delegate: Rectangle {
                        id: item
                        width: 200
                        height: 30
                        border.width: 1
                        color: "#22ffffff"
                        radius: 10
                        Text {
                            anchors.fill: parent
                            anchors.margins: 5
                            verticalAlignment: Text.AlignVCenter
                            text: name
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                var index = selectedPointClouds.indexOf(modelData)
                                if( index > -1 ) {
                                    hidePointCloud(modelData)
                                    selectedPointClouds.splice(index, 1)
                                    color = "#22ffffff"
                                } else {
                                    selectedPointClouds.push(modelData)
                                    showPointCloud(modelData)
                                    color = "#4400ff00"
                                }
                            }
                            onPressAndHold: {
                                console.log("PointCloud " + name + " metadata:")
                                for( var i in metadata ) {
                                    console.log("  " + i + ": " + metadata[i])
                                }
                            }
                            onContainsMouseChanged: {
                                metadataBox.visible = containsMouse
                                var out = "No metadata defined"
                                var list = []
                                for( var key in metadata )
                                    list.push(key + ": " + metadata[key])
                                metadataText.text = "Metadata for " + name + ":\n" + (list.length === 0 ? "No metadata defined" : list.join("\n"))
                            }
                        }
                    }
                }
                Rectangle {
                    id: metadataBox
                    anchors.left: pointCloudsList.right
                    width: metadataText.width
                    height: root.parent.height*0.5
                    color: "#22ffffff"
                    visible: false
                    Text {
                        id: metadataText
                        text: ""
                        anchors.margins: 5
                    }
                }
            }
        }
        EditMenuButton {
            id: toolsMenu
            width: parent.width
            caption: "\uE804"
            description: "Available tools"
            menu: Column {
                width: buttonsList.width
                EditMenuButton {
                    id: defaultTool
                    objectName: "cameraViewPoint"
                    width: parent.width
                    caption: "\uE808"
                    description: "Camera view point"
                    action: function() {
                        root.currentTool = this
                        closeMenus()
                        this.actionDone()
                    }
                }
                EditMenuButton {
                    id: selectionMenu
                    width: parent.width
                    caption: "\uE847"
                    description: inversed ? "Deselect points" : "Select points"
                    inversed: toolsInversed
                    menu: Column {
                        width: buttonsList.width
                        EditMenuButton {
                            objectName: "selectPoint"
                            width: parent.width
                            caption: "\uE809"
                            description: inversed ? "Deselect point with radius" : "Select point with radius"
                            inversed: toolsInversed
                            action: function() {
                                root.currentTool = this
                                closeMenus()
                                this.actionDone()
                            }
                        }
                        /*EditMenuButton {
                            objectName: "selectLine"
                            width: parent.width
                            caption: "\uE80B"
                            description: inversed ? "Deselect line with radius" : "Select line with radius"
                            inversed: toolsInversed
                            action: function() {
                                root.currentTool = this
                                closeMenus()
                                this.actionDone()
                            }
                        }
                        EditMenuButton {
                            objectName: "selectRectangle"
                            width: parent.width
                            caption: "\uE80A"
                            description: inversed ? "Deselect rectangle" : "Select rectangle"
                            inversed: toolsInversed
                            action: function() {
                                root.currentTool = this
                                closeMenus()
                                this.actionDone()
                            }
                        }*/
                    }
                }
                /*EditMenuButton {
                    objectName: "deletePoints"
                    width: parent.width
                    caption: "\uF1F8"
                    description: "Delete points"
                    action: function() {
                        root.currentTool = this
                        closeMenus()
                        this.actionDone()
                    }
                }
                EditMenuButton {
                    width: parent.width
                    caption: "\uE80C"
                    description: inversed ? "Show hidden points" : "Hide selected points"
                    inversed: toolsInversed
                    action: function() {
                        root.currentTool = this
                        closeMenus()
                        this.actionDone()
                    }
                }
                EditMenuButton {
                    width: parent.width
                    caption: "!"
                    description: "Inverse tools"
                    inversed: ! toolsInversed
                    action: function() {
                        root.toolsInversed = ! root.toolsInversed
                        this.actionDone()
                    }
                }*/
            }

            Rectangle {
                width: parent.width * 0.5
                height: width
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                radius: height
                color: parent.bgColor

                Text {
                    anchors.fill: parent
                    text: root.currentTool.caption
                    color: root.currentTool.inversed ? "#00000000" : parent.parent.fgColor
                    font.family: "icons"
                    font.pointSize: 120
                    fontSizeMode: Text.Fit
                    horizontalAlignment: Text.AlignHCenter
                    style: root.currentTool.inversed ? Text.Outline : Text.Normal
                    styleColor: Qt.rgba(1.0 - parent.parent.fgColor.r, 1.0 - parent.parent.fgColor.g, 1.0 - parent.parent.fgColor.b, 1.0)
                }
            }
        }
        EditMenuButton {
            width: parent.width
            caption: "\uE80D"
            description: "Open pointcloud from file"
            action: function() {
                osWrapper.filesPicked.connect(filesSelected)
                osWrapper.choose("openFile")
            }

            function filesSelected(paths) {
                osWrapper.filesPicked.disconnect(filesSelected)
                for( var path of paths )
                    camera.loadPointCloud(path)

                closeMenus()
                actionDone()
            }
        }
        EditMenuButton {
            width: parent.width
            caption: "\uE805"
            description: "Save visible pointclouds to files"
            action: function() {
                osWrapper.dirPicked.connect(dirSelected)
                osWrapper.choose("selectDirectory")
            }

            function dirSelected(dirpath) {
                osWrapper.dirPicked.disconnect(dirSelected)
                console.log("Save pointclouds to dir " + dirpath)

                for( var i in selectedPointClouds ) {
                    var pc = selectedPointClouds[i]
                    pc.savePCD(dirpath)
                }

                closeMenus()
                actionDone()
            }
        }
        EditMenuButton {
            width: parent.width
            caption: "\uF14D"
            description: "Export visible scene to glTF 2.0"
            action: function() {
                osWrapper.filePicked.connect(fileSelected)
                osWrapper.choose("saveFile", "scene.glb")
            }

            function fileSelected(filepath) {
                osWrapper.filePicked.disconnect(fileSelected)
                console.log("Save pointclouds to dir " + filepath)

                var prevViewType = pointCloudView.primitiveViewType
                pointCloudView.primitiveViewType = 4 // Switching to triangles mode to export mesh indexes
                sceneFile.saveScene(pointCloudView.getRoot(), filepath)
                pointCloudView.primitiveViewType = prevViewType

                closeMenus()
                actionDone()
            }
        }
    }
}
