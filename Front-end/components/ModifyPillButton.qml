import QtQuick 2.15

Rectangle {
    id: root
    width: contentRow.width + 20
    height: 28
    radius: 14

    property bool active: false
    signal clicked()

    color: mouseArea.pressed ? "#222536" : (mouseArea.containsMouse || active ? "#1D202F" : "#171824")
    border.color: mouseArea.containsMouse || active ? "#3F4360" : "#2B2E42"
    border.width: 1

    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on border.color { ColorAnimation { duration: 120 } }

    Row {
        id: contentRow
        anchors.centerIn: parent
        spacing: 7

        Text {
            text: "Modify Observation/Thinking"
            color: mouseArea.containsMouse || root.active ? "#E2E8F0" : "#8B90A0"
            font.pixelSize: 11
            font.weight: Font.Medium
            anchors.verticalCenter: parent.verticalCenter
            Behavior on color { ColorAnimation { duration: 120 } }
        }

        Canvas {
            id: chevronDown
            width: 9
            height: 6
            anchors.verticalCenter: parent.verticalCenter
            onPaint: {
                let ctx = getContext("2d")
                ctx.reset()
                ctx.strokeStyle = mouseArea.containsMouse || root.active ? "#E2E8F0" : "#8B90A0"
                ctx.lineWidth = 1.5
                ctx.lineCap = "round"
                ctx.lineJoin = "round"
                ctx.beginPath()
                ctx.moveTo(1, 1)
                ctx.lineTo(4.5, 4.5)
                ctx.lineTo(8, 1)
                ctx.stroke()
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onEntered: chevronDown.requestPaint()
        onExited: chevronDown.requestPaint()
        onClicked: root.clicked()
    }
}