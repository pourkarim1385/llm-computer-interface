import QtQuick 2.15

Item {
    id: root
    width: 38
    height: 20

    property bool checked: false
    signal toggled(bool isChecked)

    Rectangle {
        id: track
        anchors.fill: parent
        radius: height / 2
        color: root.checked ? "#393B52" : "#222433"
        border.color: root.checked ? "#4E5170" : "#2F3245"
        border.width: 1

        Behavior on color { ColorAnimation { duration: 160 } }
        Behavior on border.color { ColorAnimation { duration: 160 } }

        Rectangle {
            id: thumb
            width: parent.height - 4
            height: width
            radius: width / 2
            color: "#FFFFFF"
            anchors.verticalCenter: parent.verticalCenter
            x: root.checked ? (parent.width - width - 2) : 2

            Behavior on x {
                NumberAnimation { duration: 160; easing.type: Easing.OutQuad }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.checked = !root.checked
            root.toggled(root.checked)
        }
    }
}