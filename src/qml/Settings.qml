import QtQuick 2.0

import "js/common.js" as Common

Item {
    id: root_settings
    focus: visible

    property string key_path: ""

    function openKeyPath(path) {
        key_path = path
    }

    function openChildren(id) {
        key_path += key_path == "" ? id : '.'+id
    }

    function openParent() {
        key_path = key_path.split('.').slice(0, -1).join('.')
    }

    Rectangle {
        color: "#444"
        anchors.fill: parent
    }

    Rectangle {
        id: key_status
        width: parent.width
        height: root_settings.height / 12
        color: "#333"
        Row {
            anchors.fill: parent
            Text {
                height: parent.height
                width: height
                text: "<"
                color: key_path == "" ? "#22cccccc" : "#ffffff"
                font.pointSize: 120
                fontSizeMode: Text.Fit
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                height: parent.height * 0.75
                anchors {
                    verticalCenter: parent.verticalCenter
                    margins: parent.height / 4
                }
                verticalAlignment: Text.AlignVCenter
                text: key_path
                font.pointSize: 120
                fontSizeMode: Text.Fit
                wrapMode: Text.WordWrap
            }
        }
        MouseArea {
            anchors.fill: parent
            onReleased: openParent()
        }
    }

    ListView {
        id: settings_list
        anchors {
            top: key_status.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        width: parent.width
        model: Object.entries(cfg.opt(key_path))
        clip: true

        delegate: Item {
            id: root_item
            width: parent.width
            height: settings_list.height / 10

            property Component component

            function completeCreation() {
                component.createObject(item, {
                    "item_key": modelData[0],
                    "item_data": modelData[1],
                    "item_name": Common.beautifulName(modelData[0]),
                    "key_path": key_path == "" ? modelData[0] : key_path+'.'+modelData[0]
                })
            }

            Rectangle {
                id: item
                anchors.fill: parent
                border.width: 1
                color: "#22ffffff"
                radius: 10
                clip: true
            }

            Component.onCompleted: {
                component = Qt.createComponent("Settings/"+("type" in modelData[1] ? Common.beautifulName(modelData[1].type, '') : "Tree")+"Item.qml")

                if( component.status === Component.Ready )
                    completeCreation();
                else
                    component.statusChanged.connect(completeCreation);
            }
        }
    }
}
