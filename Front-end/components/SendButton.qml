import QtQuick
import QtQuick.Effects
import ".."

Item {
    id: root
    width: 38
    height: 38

    property bool isActive: false
    property bool isStop: false
    signal clicked()

    Palette { id: palette }

    scale: clickArea.pressed ? 0.90 : (clickArea.containsMouse && root.isActive ? 1.06 : 1.0)
    Behavior on scale {
        NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
    }

    Rectangle {
        id: glowBg
        anchors.fill: parent
        radius: width / 2
        color: palette.sendButtonHover
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
        color: root.isActive ? (clickArea.containsMouse ? palette.sendButtonHover : palette.sendButtonBg) : palette.sendButtonDisabled
        
        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -1
            text: "↑"
            font.pixelSize: 18
            font.bold: true
            color: root.isActive ? palette.sendButtonIcon : "#525266"
            visible: !root.isStop

            Behavior on color {
                ColorAnimation { duration: 150 }
            }
        }

        //Stop State
        Rectangle {
            anchors.centerIn: parent
            width: Math.round(parent.width * 0.36)
            height: width
            radius: 2
            color: root.isActive ? palette.sendButtonIcon : "#525266"
            visible: root.isStop

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