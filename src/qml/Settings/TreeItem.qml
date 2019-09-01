import QtQuick 2.0

Item {
    anchors.fill: parent

    property string item_name
    property string item_key
    property var item_data
    property string key_path

    Text {
        height: parent.height * 0.75
        anchors {
            left: parent.left
            right: arrow_right.left
            verticalCenter: parent.verticalCenter
            margins: parent.height * 0.25
        }
        verticalAlignment: Text.AlignVCenter
        text: item_name
        font.pointSize: 120
        fontSizeMode: Text.Fit
        wrapMode: Text.WordWrap
    }
    Text {
        id: arrow_right
        height: parent.height
        width: height
        anchors.right: parent.right
        text: "("+Object.keys(item_data).length+")>"
        color: "#000"
        font.pointSize: 120
        fontSizeMode: Text.Fit
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    MouseArea {
        anchors.fill: parent
        onReleased: root_settings.openKeyPath(key_path)
    }
}
