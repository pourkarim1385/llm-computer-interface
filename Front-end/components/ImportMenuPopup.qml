import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15

Popup {
    id: root
    width: 145
    height: 68
    padding: 4
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal addFilesClicked()
    signal addFolderClicked()

    background: Rectangle {
        radius: 10
        color: "#15161E"
        border.color: "#262837"
        border.width: 1
    }

    Column {
        anchors.fill: parent
        spacing: 2

        Rectangle {
            id: filesBtn
            width: parent.width
            height: 28
            radius: 6
            color: filesArea.containsMouse ? "#232636" : "transparent"

            Behavior on color { ColorAnimation { duration: 120 } }

            Row {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 8

                Item {
                    width: 14
                    height: 14
                    anchors.verticalCenter: parent.verticalCenter
                    Shape {
                        anchors.fill: parent
                        ShapePath {
                            strokeColor: filesArea.containsMouse ? "#FFFFFF" : "#8E94A5"
                            strokeWidth: 1.2
                            fillColor: "transparent"
                            capStyle: ShapePath.RoundCap
                            joinStyle: ShapePath.RoundJoin
                            PathSvg {
                                path: "M3 1 H9 L12 4 V13 A1.5 1.5 0 0 1 10.5 14.5 H3.5 A1.5 1.5 0 0 1 2 13 V2.5 A1.5 1.5 0 0 1 3.5 1 Z"
                            }
                        }
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Add Files..."
                    font.pixelSize: 12
                    font.bold: true
                    color: filesArea.containsMouse ? "#FFFFFF" : "#8E94A5"
                    Behavior on color { ColorAnimation { duration: 120 } }
                }
            }

            MouseArea {
                id: filesArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.close()
                    root.addFilesClicked()
                }
            }
        }

        Rectangle {
            width: parent.width - 6
            anchors.horizontalCenter: parent.horizontalCenter
            height: 1
            color: "#262837"
        }

        Rectangle {
            id: folderBtn
            width: parent.width
            height: 28
            radius: 6
            color: folderArea.containsMouse ? "#232636" : "transparent"

            Behavior on color { ColorAnimation { duration: 120 } }

            Row {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 8

                Item {
                    width: 14
                    height: 14
                    anchors.verticalCenter: parent.verticalCenter
                    Shape {
                        anchors.fill: parent
                        ShapePath {
                            strokeColor: folderArea.containsMouse ? "#FFFFFF" : "#8E94A5"
                            strokeWidth: 1.2
                            fillColor: "transparent"
                            capStyle: ShapePath.RoundCap
                            joinStyle: ShapePath.RoundJoin
                            PathSvg {
                                path: "M1.5 3.5 V11.5 A1 1 0 0 0 2.5 12.5 H11.5 A1 1 0 0 0 12.5 11.5 V5 A1 1 0 0 0 11.5 4 H7 L5.5 2.5 H2.5 A1 1 0 0 0 1.5 3.5 Z"
                            }
                        }
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Add Folder..."
                    font.pixelSize: 12
                    font.bold: true
                    color: folderArea.containsMouse ? "#FFFFFF" : "#8E94A5"
                    Behavior on color { ColorAnimation { duration: 120 } }
                }
            }

            MouseArea {
                id: folderArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.close()
                    root.addFolderClicked()
                }
            }
        }
    }
}