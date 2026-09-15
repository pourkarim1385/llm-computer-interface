import QtQuick 2.15

Item {
    id: root
    width: parent.width

    property string text: ""
    property int maxCollapsedHeight: 120
    property bool expanded: false

    readonly property real naturalHeight: contentText.contentHeight + 36
    readonly property bool isOverflowing: naturalHeight > root.maxCollapsedHeight

    height: isOverflowing ? (expanded ? naturalHeight + 14 : root.maxCollapsedHeight) : naturalHeight

    Behavior on height {
        NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
    }

    Rectangle {
        id: bubbleRect
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width * 0.72, 680)
        height: parent.height
        radius: 16
        color: "#161822"
        border.color: "#262938"
        border.width: 1
        clip: true

        Text {
            id: contentText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 16
            text: root.text
            color: "#FFFFFF"
            font.pixelSize: 14
            font.weight: Font.Normal
            wrapMode: Text.Wrap
            lineHeight: 1.3
            horizontalAlignment: Text.AlignAuto
        }

        Rectangle {
            id: expandBtn
            visible: root.isOverflowing
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 8
            width: 26
            height: 26
            radius: 6
            color: expandMouse.containsMouse ? "#2A2D3E" : "#1D202D"

            Text {
                anchors.centerIn: parent
                text: root.expanded ? "▲" : "▼"
                color: "#8B90A0"
                font.pixelSize: 11
            }

            MouseArea {
                id: expandMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.expanded = !root.expanded
            }
        }
    }
}