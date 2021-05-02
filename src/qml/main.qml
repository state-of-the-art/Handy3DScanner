import QtQuick 2.11
import QtQuick.Window 2.11

import QtQuick.Layouts 1.3
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.2

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("Handy 3D Scanner")

    Rectangle{
        id: rectangle
        anchors.fill: parent
        color: "white"

        TabBar {
            id: bar
            anchors.right: parent.right
            anchors.left: parent.left

            TabButton {
                text: "\uE800 "+qsTr("Capture")
                font.family: "icons"
            }
            TabButton {
                text: "\uE801 "+qsTr("Edit")
                font.family: "icons"
            }
            TabButton {
                text: "\uE802 "+qsTr("Settings")
                font.family: "icons"
            }
        }

        StackLayout {
            anchors.top: bar.bottom
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            currentIndex: bar.currentIndex

            Capture {
                id: capture
            }
            Edit {
                id: edit
            }
            Settings {
                id: settings
            }
        }
    }

    Rectangle {
        id: memory_stat
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 3
        width: memory_stat_text.width + 6
        height: memory_stat_text.height + 6
        color: "#88888888"
        radius: 3
        Text {
            id: memory_stat_text
            anchors.centerIn: parent
            color: "#fff"
            text: "available memory"
        }
        Timer {
            interval: 1000
            running: true
            repeat: true
            onTriggered: memory_stat_text.text = Math.floor(osWrapper.getMemAvail() / 1024 / 1024) + " MB"
        }
    }

    Notification {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 5
        width: parent.width * 0.3

        Component.onCompleted: {
            app.notification.connect(notification)
        }
    }
}
