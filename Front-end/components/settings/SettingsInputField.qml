import QtQuick 2.15
import ".."

Item {
    id: root
    width: parent.width
    height: 60

    property string labelText: ""
    property alias text: input.text
    property alias placeholderText: placeholder.text
    property alias echoMode: input.echoMode
    property bool readOnly: false

    Column {
        anchors.fill: parent
        spacing: 6

        Text {
            text: root.labelText
            color: "#E2E8F0"
            font.pixelSize: 13
            font.bold: true
        }

        Rectangle {
            width: parent.width
            height: 38
            radius: 8
            color: root.readOnly ? "#10121A" : (input.activeFocus ? "#131622" : (boxMouse.containsMouse ? "#151824" : "#12141D"))
            border.color: input.activeFocus ? "#7C3AED" : (boxMouse.containsMouse ? "#2D3247" : "#1D202D")
            border.width: 1

            Behavior on color { ColorAnimation { duration: 140 } }
            Behavior on border.color { ColorAnimation { duration: 140 } }

            Text {
                id: placeholder
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 12
                color: "#4B5563"
                font.pixelSize: 12
                visible: input.text.length === 0 && !input.activeFocus
            }

            TextInput {
                id: input
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                verticalAlignment: TextInput.AlignVCenter
                color: root.readOnly ? "#9CA3AF" : "#FFFFFF"
                font.pixelSize: 12
                selectByMouse: true
                selectionColor: "#7C3AED"
                selectedTextColor: "#FFFFFF"
                readOnly: root.readOnly
                clip: true
            }

            MouseArea {
                id: boxMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: root.readOnly ? Qt.ArrowCursor : Qt.IBeamCursor
                acceptedButtons: Qt.NoButton
            }
        }
    }
}