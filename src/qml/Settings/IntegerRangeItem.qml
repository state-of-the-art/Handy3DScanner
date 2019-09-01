import QtQuick 2.0
import QtQuick.Controls 2.5

Item {
    anchors.fill: parent

    property string item_name
    property string item_key
    property var item_data
    property string key_path

    property int max_symbols: (''+item_data.max).length

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

    Slider {
        id: slider
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: value_box.left
            margins: parent.height * 0.10
        }
        opacity: 0.3
        handle.width: height * 0.80
        handle.height: handle.width
        snapMode: Slider.SnapOnRelease

        value: cfg[key_path]
        from: item_data.min
        to: item_data.max
        stepSize: item_data.step

        onPressedChanged: {
            if( !pressed ) {
                value_text.color = "#000"
                cfg.setVal(key_path, Number.parseInt(value))
            }
        }
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
            text: (''+slider.value).padStart(max_symbols, '0');
            color: item_data.value !== undefined ? "#000" : "#444"
            font.pointSize: 120
            fontSizeMode: Text.Fit
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea {
            anchors.fill: parent
            preventStealing: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            function unsetValue() {
                value_text.color = "#444"
                cfg.unsetVal(key_path)
                slider.value = cfg[key_path]
            }

            onReleased: {
                if( mouse.button == Qt.RightButton )
                    unsetValue()
            }

            onPressAndHold: unsetValue()
        }
    }
}
