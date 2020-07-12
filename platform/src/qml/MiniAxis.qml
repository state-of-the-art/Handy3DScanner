import QtQuick 2.12

Item {
    Text {
        anchors.fill: parent
        visible: !(cameraPos.isSupportRotation || cameraPos.isSupportPosition)
        text: "N/A"
        color: "#444"
        font.pointSize: 120
        fontSizeMode: Text.Fit
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    MiniAxisScene {
        id: miniaxis
        anchors.fill: parent
        visible: cameraPos.isSupportRotation
    }

    Text {
        id: value_text
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height / 3
        visible: cameraPos.isSupportPosition
        text: cameraPos.translation.x.toFixed(1) + " " + cameraPos.translation.y.toFixed(1) + " " + cameraPos.translation.z.toFixed(1)
        color: "#fff"
        font.pointSize: 120
        fontSizeMode: Text.Fit
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        anchors.fill: parent
        onPressed: miniaxis.resetPosition()
    }
}
