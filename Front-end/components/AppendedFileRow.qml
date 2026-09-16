import QtQuick 2.15
import QtQuick.Controls 2.15
import ".."

Item {
    id: root
    width: parent ? parent.width : 400
    height: 38

    property int itemIndex: -1
    property string filePath: ""
    property string fileName: ""
    property bool isFolder: false

    signal editRequested(int itemIndex)
    signal removeRequested(int itemIndex)

    Rectangle {
        id: removeBtn
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 34
        height: 34
        radius: 8
        color: removeMouse.pressed ? "#3D171C" : (removeMouse.containsMouse ? "#30151B" : "#221419")
        border.color: removeMouse.containsMouse ? "#EF4444" : "#3B1B22"
        border.width: 1
        scale: removeMouse.pressed ? 0.92 : (removeMouse.containsMouse ? 1.05 : 1.0)

        Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }
        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        Canvas {
            anchors.centerIn: parent
            width: 14
            height: 15
            onPaint: {
                let ctx = getContext("2d");
                ctx.reset();
                ctx.strokeStyle = removeMouse.containsMouse ? "#FCA5A5" : "#EF4444";
                ctx.lineWidth = 1.4;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";
                ctx.beginPath();
                ctx.moveTo(2, 4);
                ctx.lineTo(12, 4);
                ctx.moveTo(5, 4);
                ctx.lineTo(5, 2);
                ctx.lineTo(9, 2);
                ctx.lineTo(9, 4);
                ctx.moveTo(3, 4);
                ctx.lineTo(4, 13);
                ctx.lineTo(10, 13);
                ctx.lineTo(11, 4);
                ctx.moveTo(6, 6.5);
                ctx.lineTo(6, 10.5);
                ctx.moveTo(8, 6.5);
                ctx.lineTo(8, 10.5);
                ctx.stroke();
            }
        }

        MouseArea {
            id: removeMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered: removeBtn.children[0].requestPaint()
            onExited: removeBtn.children[0].requestPaint()
            onClicked: root.removeRequested(root.itemIndex)
        }
    }

    Rectangle {
        id: editBtn
        anchors.right: removeBtn.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        width: 34
        height: 34
        radius: 8
        color: editMouse.pressed ? "#2B2245" : (editMouse.containsMouse ? "#231B3A" : "#1A1726")
        border.color: editMouse.containsMouse ? "#8B5CF6" : "#2E2447"
        border.width: 1
        scale: editMouse.pressed ? 0.92 : (editMouse.containsMouse ? 1.05 : 1.0)

        Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }
        Behavior on color { ColorAnimation { duration: 150 } }
        Behavior on border.color { ColorAnimation { duration: 150 } }

        Canvas {
            anchors.centerIn: parent
            width: 16
            height: 14
            onPaint: {
                let ctx = getContext("2d");
                ctx.reset();
                ctx.strokeStyle = editMouse.containsMouse ? "#C4B5FD" : "#8B5CF6";
                ctx.lineWidth = 1.5;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";
                ctx.beginPath();
                ctx.moveTo(1, 3);
                ctx.lineTo(6, 3);
                ctx.lineTo(8, 5);
                ctx.lineTo(15, 5);
                ctx.lineTo(15, 13);
                ctx.lineTo(1, 13);
                ctx.closePath();
                ctx.stroke();
            }
        }

        MouseArea {
            id: editMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered: editBtn.children[0].requestPaint()
            onExited: editBtn.children[0].requestPaint()
            onClicked: root.editRequested(root.itemIndex)
        }
    }

    Rectangle {
        id: pathContainer
        anchors.left: parent.left
        anchors.right: editBtn.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        height: 34
        radius: 8
        color: "#161722"
        border.color: pathMouseArea.containsMouse ? "#2A2D3D" : "#1F222E"
        border.width: 1

        Behavior on border.color { ColorAnimation { duration: 150 } }

        MouseArea {
            id: pathMouseArea
            anchors.fill: parent
            hoverEnabled: true
        }

        Text {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            verticalAlignment: Text.AlignVCenter
            text: root.filePath
            color: "#CBD5E1"
            font.pixelSize: 12
            font.family: "Consolas, monospace"
            elide: Text.ElideMiddle
        }
    }
}