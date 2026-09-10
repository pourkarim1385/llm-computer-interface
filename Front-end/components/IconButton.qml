import QtQuick 2.15
import QtQuick.Shapes 1.15

Item {
    id: root
    width: 32
    height: 32

    property string iconType: "sidebar"
    property color iconColor: mouseArea.containsMouse ? "#ffffff" : "#8a8f9d"
    property color hoverCircleColor: "#1d202d"

    signal clicked()

    Rectangle {
        id: hoverCircle
        anchors.fill: parent
        radius: width / 2
        color: root.hoverCircleColor
        opacity: mouseArea.containsMouse ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 180; easing.type: Easing.OutQuad }
        }
    }

    Item {
        anchors.centerIn: parent
        width: 18
        height: 18

        //Sidebar Panel Icon
        Rectangle {
            visible: root.iconType === "sidebar"
            anchors.fill: parent
            radius: 3
            color: "transparent"
            border.color: root.iconColor
            border.width: 1.5

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 1.5
                width: 4
                color: root.iconColor
                radius: 1
            }

            Behavior on border.color { ColorAnimation { duration: 150 } }
        }

        Shape {
            visible: root.iconType === "arrow_left"
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4

            ShapePath {
                strokeColor: root.iconColor
                strokeWidth: 1.6
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin

                startX: 11; startY: 4
                PathLine { x: 5; y: 9 }
                PathLine { x: 11; y: 14 }
                PathMove { x: 5; y: 9 }
                PathLine { x: 14; y: 9 }
            }
        }

        Shape {
            visible: root.iconType === "arrow_right"
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4

            ShapePath {
                strokeColor: root.iconColor
                strokeWidth: 1.6
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin

                startX: 7; startY: 4
                PathLine { x: 13; y: 9 }
                PathLine { x: 7; y: 14 }
                PathMove { x: 13; y: 9 }
                PathLine { x: 4; y: 9 }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}