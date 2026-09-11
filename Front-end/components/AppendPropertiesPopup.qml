import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id: root
    width: 380
    height: 390
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: parent

    property int targetIndex: -1
    property string currentPath: ""

    function openForIndex(idx) {
        if (typeof appendedFilesModel === "undefined" || idx < 0) return;
        targetIndex = idx;
        let data = appendedFilesModel.get(idx);
        currentPath = data.filePath || "";
        descInput.text = data.description || "";
        namesInput.text = data.excludeNames || "";
        extsInput.text = data.excludeExtensions || "";
        open();
    }

    background: Rectangle {
        radius: 14
        color: "#161822"
        border.color: "#282B3B"
        border.width: 1
    }

    contentItem: Item {
        Column {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 8

            // Header
            Row {
                width: parent.width
                height: 24

                Text {
                    width: parent.width - 24
                    text: "Append Properties"
                    font.bold: true
                    font.pixelSize: 16
                    color: "#FFFFFF"
                }

                Text {
                    text: "✕"
                    font.pixelSize: 14
                    color: closeMouse.containsMouse ? "#FFFFFF" : "#8B90A0"

                    MouseArea {
                        id: closeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.close()
                    }
                }
            }

            // Path
            Text {
                width: parent.width
                text: 'Path: "' + root.currentPath + '"'
                color: "#94A3B8"
                font.pixelSize: 12
                elide: Text.ElideMiddle
            }

            // Description
            Text {
                text: "Description"
                color: "#E2E8F0"
                font.pixelSize: 13
                font.weight: Font.Medium
            }

            Rectangle {
                width: parent.width
                height: 38
                radius: 8
                color: "#1F212D"
                border.color: descInput.activeFocus ? "#7C3AED" : "#2E3242"
                border.width: 1

                TextField {
                    id: descInput
                    anchors.fill: parent
                    anchors.margins: 6
                    placeholderText: "What would the ideal answer have been?"
                    placeholderTextColor: "#64748B"
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    background: null
                }
            }

            // Dont Analyze Section
            Text {
                text: "Dont Analyze:"
                color: "#E2E8F0"
                font.pixelSize: 13
                font.weight: Font.Medium
            }

            Text {
                text: "Names:"
                color: "#94A3B8"
                font.pixelSize: 12
                anchors.leftMargin: 4
            }

            Rectangle {
                width: parent.width
                height: 38
                radius: 8
                color: "#1F212D"
                border.color: namesInput.activeFocus ? "#7C3AED" : "#2E3242"
                border.width: 1

                TextField {
                    id: namesInput
                    anchors.fill: parent
                    anchors.margins: 6
                    placeholderText: "Write Names and seprate them with space"
                    placeholderTextColor: "#64748B"
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    background: null
                }
            }

            Text {
                text: "Extensions:"
                color: "#94A3B8"
                font.pixelSize: 12
                anchors.leftMargin: 4
            }

            Rectangle {
                width: parent.width
                height: 38
                radius: 8
                color: "#1F212D"
                border.color: extsInput.activeFocus ? "#7C3AED" : "#2E3242"
                border.width: 1

                TextField {
                    id: extsInput
                    anchors.fill: parent
                    anchors.margins: 6
                    placeholderText: "Write extentions and seprate them with space"
                    placeholderTextColor: "#64748B"
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    background: null
                }
            }

            //Apply button
            Item {
                width: parent.width
                height: 38

                Rectangle {
                    id: applyBtn
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: 82
                    height: 32
                    radius: 8
                    color: applyMouse.pressed ? "#1E202C" : (applyMouse.containsMouse ? "#2A2D3E" : "#222533")
                    border.color: applyMouse.containsMouse ? "#4B5563" : "#374151"
                    border.width: 1
                    scale: applyMouse.pressed ? 0.95 : 1.0

                    Behavior on scale { NumberAnimation { duration: 100 } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "Apply"
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                    }

                    MouseArea {
                        id: applyMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (root.targetIndex >= 0 && typeof appendedFilesModel !== "undefined") {
                                appendedFilesModel.updateProperties(
                                    root.targetIndex,
                                    descInput.text.trim(),
                                    namesInput.text.trim(),
                                    extsInput.text.trim()
                                );
                            }
                            root.close();
                        }
                    }
                }
            }
        }
    }
}