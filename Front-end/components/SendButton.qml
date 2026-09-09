import QtQuick
import QtQuick.Effects

Item {
    id: root
    width: 38
    height: 38

    property bool isActive: false
    property color activeColor: "#7C3AED"
    signal clicked()

    scale: clickArea.pressed ? 0.90 : (clickArea.containsMouse && root.isActive ? 1.06 : 1.0)
    Behavior on scale {
        NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
    }

    Rectangle {
        id: glowBg
        anchors.fill: parent
        radius: width / 2
        color: root.activeColor
        opacity: root.isActive ? (clickArea.containsMouse ? 0.95 : 0.65) : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    MultiEffect {
        source: glowBg
        anchors.fill: glowBg
        blurEnabled: true
        blur: 0.75
        blurMax: 22
        opacity: glowBg.opacity
    }

    Rectangle {
        id: buttonBody
        anchors.fill: parent
        radius: width / 2
        color: root.isActive ? root.activeColor : "#22222E"

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -1
            text: "↑"
            font.pixelSize: 18
            font.bold: true
            color: root.isActive ? "#FFFFFF" : "#525266"

            Behavior on color {
                ColorAnimation { duration: 150 }
            }
        }
    }

    MouseArea {
        id: clickArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.isActive ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: {
            if (root.isActive) {
                root.clicked()
            }
        }
    }
}
