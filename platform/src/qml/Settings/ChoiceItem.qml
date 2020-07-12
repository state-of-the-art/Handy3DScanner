import QtQuick 2.0

Item {
    anchors.fill: parent

    property string item_name
    property string item_key
    property var item_data
    property string key_path

    Text {
        height: parent.height * 0.5
        anchors {
            top: parent.top
            left: parent.left
            right: value_box.left
            margins: parent.height * 0.10
        }
        verticalAlignment: Text.AlignVCenter
        text: item_name
        font.pointSize: 120
        fontSizeMode: Text.Fit
        wrapMode: Text.WordWrap
    }
    Text {
        height: parent.height * 0.25
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: value_box.left
            margins: parent.height * 0.10
        }
        verticalAlignment: Text.AlignVCenter
        text: item_data.description
        font.pointSize: 120
        fontSizeMode: Text.Fit
        wrapMode: Text.WordWrap
    }

    Rectangle {
        id: value_box
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: parent.height * 0.125
        }
        height: parent.height * 0.75
        width: value_text.contentWidth + 20

        color: "#22ffffff"
        border.width: 2
        radius: 5

        Text {
            id: value_text
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: parent.height
            text: cfg[key_path]
            color: item_data.value !== undefined ? "#000" : "#444"
            font.pointSize: 120
            fontSizeMode: Text.Fit
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea {
            anchors.fill: parent
            preventStealing: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            property bool unset: false

            function setValue(value) {
                value_text.color = "#000"
                cfg.setVal(key_path, value)
            }

            function unsetValue() {
                value_text.color = "#444"
                cfg.unsetVal(key_path)
            }

            onReleased: {
                if( unset )
                    return unset = false
                if( mouse.button == Qt.LeftButton )
                    // Getting next value or start from the beginning
                    setValue(item_data.list[(item_data.list.indexOf(cfg[key_path])+1) % item_data.list.length])
                else
                    unsetValue()
            }
            onPressAndHold: {
                unset = true
                unsetValue()
            }
        }
    }
}
