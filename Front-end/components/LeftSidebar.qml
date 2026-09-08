import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15
import ".."

Item {
    id: root
    width: isCollapsed ? 56 : (palette.baseSidebarWidth + jellyOffset)
    anchors.top: parent.top
    anchors.bottom: parent.bottom

    Palette { id: palette }

    property bool isCollapsed: false
    property real jellyOffset: 0.0
    readonly property real maxJellyPull: 22.0

    Behavior on width {
        NumberAnimation {
            duration: root.isCollapsed ? 320 : 340
            easing.type: root.isCollapsed ? Easing.InCubic : Easing.OutCubic
        }
    }

    Behavior on jellyOffset {
        NumberAnimation {
            duration: 350
            easing.type: Easing.OutBack
            easing.overshoot: 1.6
        }
    }

    Item {
        anchors.fill: parent
        clip: true

        Rectangle {
            id: mainPanel
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 12
            anchors.bottomMargin: 12
            anchors.left: parent.left
            anchors.leftMargin: 8

            width: palette.baseSidebarWidth + root.jellyOffset - 12
            radius: palette.sidebarRadius
            color: palette.sidebarBg
            border.color: palette.sidebarBorder
            border.width: 1

            Item {
                id: headerArea
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 94
                z: 10

                IconButton {
                    id: toggleBtn
                    anchors.top: parent.top
                    anchors.topMargin: 12
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    iconType: "sidebar"
                    onClicked: root.isCollapsed = !root.isCollapsed
                }

                Row {
                    anchors.top: parent.top
                    anchors.topMargin: 12
                    anchors.right: parent.right
                    anchors.rightMargin: 14
                    spacing: 4
                    opacity: root.isCollapsed ? 0.0 : 1.0
                    visible: opacity > 0.0

                    Behavior on opacity { NumberAnimation { duration: 150 } }

                    IconButton {
                        iconType: "arrow_left"
                        onClicked: if (typeof navController !== "undefined") navController.navigateBack()
                    }
                    IconButton {
                        iconType: "arrow_right"
                        onClicked: if (typeof navController !== "undefined") navController.navigateForward()
                    }
                }

                //New Chat
                Rectangle {
                    id: newChatBtn
                    anchors.top: toggleBtn.bottom
                    anchors.topMargin: 10
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    height: 34
                    radius: 8
                    color: newChatMouse.containsMouse ? palette.chatHoverBg : "transparent"
                    opacity: root.isCollapsed ? 0.0 : 1.0
                    visible: opacity > 0.0

                    Behavior on color { ColorAnimation { duration: 160 } }
                    Behavior on opacity { NumberAnimation { duration: 150 } }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        spacing: 8

                        Item {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 14
                            height: 14
                            Shape {
                                anchors.fill: parent
                                ShapePath {
                                    strokeColor: newChatMouse.containsMouse ? palette.textActive : palette.textMuted
                                    strokeWidth: 1.4
                                    fillColor: "transparent"
                                    startX: 2; startY: 2
                                    PathLine { x: 12; y: 2 }
                                    PathLine { x: 8; y: 7 }
                                    PathLine { x: 8; y: 12 }
                                    PathLine { x: 6; y: 12 }
                                    PathLine { x: 6; y: 7 }
                                    PathLine { x: 2; y: 2 }
                                }
                            }
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "New Chat"
                            font.pixelSize: 13
                            font.bold: true
                            color: newChatMouse.containsMouse ? palette.textActive : palette.textMuted
                            Behavior on color { ColorAnimation { duration: 140 } }
                        }
                    }

                    MouseArea {
                        id: newChatMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (typeof navController !== "undefined") navController.createNewChat()
                    }
                }

                //Seperator Line
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    height: 1
                    color: palette.dividerColor
                    opacity: root.isCollapsed ? 0.0 : 1.0
                    visible: opacity > 0.0
                }
            }

            ListView {
                id: chatList
                anchors.top: headerArea.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottomMargin: 8
                anchors.topMargin: 6
                clip: true
                spacing: 2
                opacity: root.isCollapsed ? 0.0 : 1.0
                visible: opacity > 0.0

                Behavior on opacity { NumberAnimation { duration: 150 } }

                model: (typeof chatModel !== "undefined") ? chatModel : fallbackDummyModel

                delegate: Item {
                    width: chatList.width
                    height: 38

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        // برآمدگی کپسول چت اکتیو به سمت بیرون (Bulge)
                        anchors.rightMargin: model.isActive ? -8 : 8
                        height: 34
                        radius: 10
                        color: model.isActive ? palette.activeChatBg : (itemMouse.containsMouse ? palette.chatHoverBg : "transparent")

                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            text: model.chatTitle
                            elide: Text.ElideRight
                            font.pixelSize: 12
                            font.bold: model.isActive
                            color: model.isActive ? palette.textActive : (itemMouse.containsMouse ? palette.textHover : palette.textMuted)

                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof chatModel !== "undefined") {
                                chatModel.selectChat(index)
                            }
                        }
                    }
                }
            }

            MouseArea {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.rightMargin: -16
                width: 32
                hoverEnabled: true
                enabled: !root.isCollapsed
                cursorShape: Qt.SizeHorCursor

                onPositionChanged: (mouse) => {
                    var delta = mouse.x - (width / 2);
                    root.jellyOffset = (delta > 0) ? Math.min(delta * 0.75, root.maxJellyPull) : 0;
                }
                onExited: root.jellyOffset = 0
            }
        }
    }

    ListModel {
        id: fallbackDummyModel
        ListElement { chatId: "1"; chatTitle: "Chat Conversation #2"; isActive: false }
        ListElement { chatId: "2"; chatTitle: "Chat Conversation #2"; isActive: false }
        ListElement { chatId: "3"; chatTitle: "Chat Conversation #2"; isActive: false }
        ListElement { chatId: "4"; chatTitle: "Active Chat"; isActive: true }
    }
}