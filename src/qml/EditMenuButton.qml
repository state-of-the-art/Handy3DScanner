import QtQuick 2.0

Item {
    id: root
    height: width

    property string caption: ""
    property color fgColor: "#000"
    property color bgColor: "#88ffffff"
    property string description: ""

    property bool enabled: true
    property bool selected: false
    property var action: function() { this.actionDone() }
    property var actionDone: function() { root.state = mouseArea.containsMouse ? "showDescription" : "" }
    property var menu: null
    property bool inversed: false

    function stateDescription() {
         root.state = "showDescription"
    }

    function stateDefault() {
         root.state = ""
    }

    function stateActive() {
        if( menu !== null ) {
            if( root.state === "showMenu" )
                root.state = mouseArea.containsMouse ? "showDescription" : ""
            else if( menu !== null )
                root.state = "showMenu"
        } else {
            root.state = "action"
            root.action()
        }
    }

    function closeMenu() {
        if( root.state === "showMenu" )
            root.state = ""
    }

    Rectangle {
        id: captionBox
        border.color: Qt.rgba(fgColor.r, fgColor.g, fgColor.b, 0.5)
        border.width: 1
        anchors.fill: parent
        anchors.margins: parent.width
        radius: height
        color: bgColor
    }

    Text {
        id: captionText
        text: caption
        color: inversed ? "#00000000" : fgColor
        font.family: "icons"
        font.pointSize: 120
        anchors.fill: parent
        fontSizeMode: Text.Fit
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        style: Text.Outline
        styleColor: inversed ? fgColor : Qt.rgba(1.0 - fgColor.r, 1.0 - fgColor.g, 1.0 - fgColor.b, 1.0)
    }

    Rectangle {
        id: menuBox
        anchors.left: parent.right
        anchors.leftMargin: -parent.width
        anchors.top: parent.top
        radius: 10
        color: bgColor
        opacity: 0.0
        visible: false
        height: childrenRect.height
        width: childrenRect.width

        children: menu
    }

    Rectangle {
        id: descriptionBox
        anchors.left: parent.right
        anchors.leftMargin: -parent.width
        radius: height
        color: bgColor
        opacity: 0.0
        visible: false
        height: parent.height * 0.5
        width: descriptionText.width + height
        anchors.verticalCenter: parent.verticalCenter

        Text {
            anchors.centerIn: parent
            id: descriptionText
            text: description
            color: fgColor
            height: parent.height * 0.8
            fontSizeMode: Text.VerticalFit
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        onContainsMouseChanged: {
            if( containsMouse ) {
                if( root.state === "" )
                    stateDescription()
            } else {
                if( root.state === "showDescription" )
                    stateDefault()
            }
        }
        onReleased: {
            if( mouseArea.containsMouse )
                stateActive()
        }
        onPressAndHold: stateDescription()
    }

    states: [
        State {
            name: "showDescription"
            PropertyChanges { target: descriptionBox; visible: true }
            PropertyChanges { target: descriptionBox; opacity: 0.7 }
            PropertyChanges { target: descriptionBox.anchors; leftMargin: descriptionBox.height * 0.5 }
            PropertyChanges { target: captionBox.anchors; margins: -5 }
        },
        State {
            name: "action"
            PropertyChanges { target: captionBox.anchors; margins: -5 }
        },
        State {
            name: "showMenu"
            PropertyChanges { target: menuBox; visible: true }
            PropertyChanges { target: menuBox; opacity: 1.0 }
            PropertyChanges { target: menuBox.anchors; leftMargin: descriptionBox.height * 0.5 }
            PropertyChanges { target: captionBox.anchors; margins: -5 }
        }
    ]

    transitions: [
        Transition {
            to: "*"
            ParallelAnimation {
                NumberAnimation { property: "opacity"; duration: 200 }
                NumberAnimation { property: "leftMargin"; duration: 400 }
                NumberAnimation { property: "margins"; duration: 200; easing.type: Easing.InElastic; easing.amplitude: 2.0; easing.period: 1.5 }
                PropertyAnimation { property: "visible"; duration: 200 }
            }
        },
        Transition {
            to: "showDescription"
            ParallelAnimation {
                NumberAnimation { property: "opacity"; duration: 400 }
                NumberAnimation { property: "leftMargin"; duration: 200 }
                NumberAnimation { property: "margins"; duration: 200; easing.type: Easing.OutElastic; easing.amplitude: 2.0; easing.period: 1.5 }
            }
        },
        Transition {
            to: "action"
            ParallelAnimation {
                SequentialAnimation {
                    loops: Animation.Infinite
                    NumberAnimation { property: "margins"; to: 10; duration: 500; easing.type: Easing.InElastic; easing.amplitude: 2.0; easing.period: 1.5 }
                    NumberAnimation { property: "margins"; to: -5; duration: 500; easing.type: Easing.OutElastic; easing.amplitude: 2.0; easing.period: 1.5 }
                }
            }
        }
    ]
}
