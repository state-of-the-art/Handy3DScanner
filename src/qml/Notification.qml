import QtQuick 2.0

import "js/common.js" as Common

Item {
    id: root_notification

    function notification(type, message) {
        for( var i = 0; i < stack.count; i++ ) {
            var obj = stack.get(i)
            if( obj.message === message ) {
                stack.setProperty(i, "count_similar", obj.count_similar+1)
                return
            }
        }

        stack.append({
            title: Common.beautifulName(type),
            type: type,
            message: message,
            count_similar: 0,
        })
        notifications_list.positionViewAtEnd()
    }

    ListModel { id: stack }

    ListView {
        id: notifications_list
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        model: stack
        width: parent.width
        height: childrenRect.height < parent.height ? childrenRect.height : parent.height
        clip: true
        interactive: childrenRect.height > parent.height
        spacing: 5

        delegate: Rectangle {
            id: bubble
            width: root_notification.width
            height: root_notification.height * 0.1
            radius: height * 0.1
            opacity: 0.0
            clip: true

            property bool freeze: false

            function removeItem() {
                if( freeze )
                    return
                console.log("Remove " + type + " " + index)
                stack.remove(index)
            }

            onFreezeChanged: removeItem()

            Behavior on opacity { NumberAnimation { duration: 200 } }

            border.width: freeze ? 2 : 0
            color: "#77ffffff"
            state: type

            Timer {
                id: timer
                interval: type == "warning" ? 20000 : 5000
                onTriggered: removeItem()
            }

            Text {
                id: title_text
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: parent.height * 0.05
                }
                text: title + (count_similar > 0 ? " ("+count_similar+")" : "")
                font.pointSize: 12
                font.underline: true
                font.bold: true
                clip: true

                onTextChanged: {
                    timer.restart()
                }
            }
            Text {
                id: message_text
                anchors {
                    top: title_text.bottom
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                    margins: parent.height * 0.05
                }
                text: message
                font.pixelSize: 40
                minimumPixelSize: 6
                fontSizeMode: Text.Fit
                wrapMode: Text.WordWrap
                clip: true
            }

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    freeze = !freeze
                }
            }

            states: [
                State {
                    name: "warning"
                    PropertyChanges { target: bubble; color: "#77ff8800" }
                },
                State {
                    name: "error"
                    PropertyChanges { target: bubble; color: "#77ff0000" }
                }
            ]

            ListView.onRemove: SequentialAnimation {
                PropertyAction { target: bubble; property: "ListView.delayRemove"; value: true }
                NumberAnimation { target: bubble; property: "scale"; to: 0; duration: 250; easing.type: Easing.InOutQuad }
                PropertyAction { target: bubble; property: "ListView.delayRemove"; value: false }
            }

            Component.onCompleted: {
                console.log("Created " + type + " " + index)
                opacity = 1.0
                if( type !== "error" || freeze )
                    timer.start()
            }
        }
    }
}
