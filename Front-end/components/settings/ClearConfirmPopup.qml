import QtQuick 2.15

Item {
    id: root
    anchors.fill: parent
    visible: opacity > 0.0
    opacity: 0.0

    signal confirmed()
    signal cancelled()

    function open() { opacity = 1.0; }
    function close() { opacity = 0.0; }

    Behavior on opacity { NumberAnimation { duration: 180 } }

    Rectangle {
        anchors.fill: parent
        color: "#B0000000"

        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    Rectangle {
        id: dialog
        width: 420
        height: 210
        radius: 14
        anchors.centerIn: parent
        color: "#11131B"
        border.color: "#381E23"
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 22
            spacing: 14

            Text {
                text: "Clear all conversations & messages?"
                color: "#FFFFFF"
                font.pixelSize: 16
                font.bold: true
            }

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: "This action will permanently delete all chat history and messages from the local database. You cannot undo this action."
                color: "#9CA3AF"
                font.pixelSize: 12
                lineHeight: 1.3
            }

            Item { width: 1; height: 6 }

            Row {
                anchors.right: parent.right
                spacing: 10

                Rectangle {
                    width: 90; height: 34; radius: 8
                    color: cancelMouse.containsMouse ? "#232635" : "#1A1D2A"
                    border.color: "#2C3042"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        color: cancelMouse.containsMouse ? "#FFFFFF" : "#C2CBD5"
                        font.pixelSize: 12; font.bold: true
                    }
                    MouseArea {
                        id: cancelMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.close();
                            root.cancelled();
                        }
                    }
                }

                Rectangle {
                    width: 120; height: 34; radius: 8
                    color: confirmMouse.containsMouse ? "#DC2626" : "#B91C1C"

                    Text {
                        anchors.centerIn: parent
                        text: "Delete Everything"
                        color: "#FFFFFF"
                        font.pixelSize: 12; font.bold: true
                    }
                    MouseArea {
                        id: confirmMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.close();
                            root.confirmed();
                        }
                    }
                }
            }
        }
    }
}