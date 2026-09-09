import QtQuick 2.15
import ".."

Item {
    id: root
    width: Math.min(parent.width * 0.72, 680)
    anchors.horizontalCenter: parent.horizontalCenter

    implicitHeight: 30
    height: visible ? implicitHeight : 0

    visible: agentBridge.isWorking && (agentBridge.statusText !== "")
    opacity: visible ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation { duration: 250 }
    }

    Row {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        LoadingAnim {
            width: 20
            height: 20
            anchors.verticalCenter: parent.verticalCenter
            speed: 100
            pixelSpacing: 0.8
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: agentBridge.statusText
            font.pixelSize: 13
            font.weight: Font.Medium
            color: "#8B90A0"
        }
    }
}