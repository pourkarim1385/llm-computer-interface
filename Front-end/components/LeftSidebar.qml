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
    property string editingChatId: ""

    signal settingsRequested()

    onIsCollapsedChanged: {
        if (isCollapsed) {
            contextMenu.closeMenu()
            root.editingChatId = ""
        }
    }

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

            // --- Header Area ---
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
                    enabled: (typeof agentBridge !== "undefined") ? !agentBridge.isWorking : true
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
                            width: 14; height: 14
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
                        hoverEnabled: parent.enabled
                        cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: if (typeof navController !== "undefined") navController.createNewChat()
                    }
                }

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

            // --- Chat List View ---
            ListView {
                id: chatList
                anchors.top: headerArea.bottom
                anchors.bottom: footerArea.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottomMargin: 4
                anchors.topMargin: 6
                clip: true
                spacing: 2
                opacity: root.isCollapsed ? 0.0 : 1.0
                visible: opacity > 0.0
                Behavior on opacity { NumberAnimation { duration: 150 } }

                model: (typeof chatModel !== "undefined") ? chatModel : fallbackDummyModel

                delegate: Item {
                    id: delegateRoot
                    width: chatList.width
                    height: 38

                    readonly property bool isEditing: root.editingChatId === model.chatId
                    readonly property bool isRowHovered: itemMouse.containsMouse || moreMouse.containsMouse

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        z: 0
                        enabled: !delegateRoot.isEditing && ((typeof agentBridge !== "undefined") ? !agentBridge.isWorking : true)
                        hoverEnabled: enabled
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                        onClicked: (mouse) => {
                            if (mouse.button === Qt.RightButton) {
                                var pt = mapToItem(mainPanel, mouse.x, mouse.y);
                                contextMenu.openMenu(model.chatId, pt.x, pt.y);
                            } else if (mouse.button === Qt.LeftButton) {
                                if (typeof chatModel !== "undefined") {
                                    chatModel.selectChat(index);
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: itemBg
                        z: 1
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: model.isActive ? -8 : 8
                        height: 34
                        radius: 10
                        color: model.isActive ? palette.activeChatBg : (delegateRoot.isRowHovered ? palette.chatHoverBg : "transparent")
                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.right: moreBtn.visible ? moreBtn.left : parent.right
                            anchors.rightMargin: moreBtn.visible ? 4 : 12
                            text: model.chatTitle
                            elide: Text.ElideRight
                            font.pixelSize: 12
                            font.bold: model.isActive
                            color: model.isActive ? palette.textActive : (delegateRoot.isRowHovered ? palette.textHover : palette.textMuted)
                            visible: !delegateRoot.isEditing
                            Behavior on color { ColorAnimation { duration: 150 } }
                        }

                        TextInput {
                            id: renameInput
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            font.pixelSize: 12
                            font.bold: true
                            color: palette.textActive
                            selectByMouse: true
                            selectionColor: palette.textBoxGlowColorOne
                            selectedTextColor: "#FFFFFF"
                            clip: true
                            visible: delegateRoot.isEditing

                            onVisibleChanged: {
                                if (visible) {
                                    text = model.chatTitle;
                                    forceActiveFocus();
                                    selectAll();
                                }
                            }
                            onAccepted: commitRename()
                            Keys.onEscapePressed: { root.editingChatId = ""; }
                            onActiveFocusChanged: {
                                if (!activeFocus && delegateRoot.isEditing) commitRename();
                            }

                            function commitRename() {
                                var newT = text.trim();
                                if (newT !== "" && newT !== model.chatTitle) {
                                    if (typeof chatModel !== "undefined") {
                                        chatModel.renameChat(model.chatId, newT);
                                    } else {
                                        fallbackDummyModel.setProperty(index, "chatTitle", newT);
                                    }
                                }
                                root.editingChatId = "";
                            }
                        }

                        Rectangle {
                            id: moreBtn
                            z: 2
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 6
                            width: 22; height: 22
                            radius: 6
                            color: moreMouse.containsMouse ? palette.iconHoverCircle : "transparent"
                            visible: !delegateRoot.isEditing && (delegateRoot.isRowHovered || (contextMenu.isOpen && contextMenu.targetChatId === model.chatId))
                            opacity: visible ? 1.0 : 0.0

                            Behavior on opacity { NumberAnimation { duration: 120 } }
                            Behavior on color { ColorAnimation { duration: 120 } }

                            Row {
                                anchors.centerIn: parent
                                spacing: 2
                                Repeater {
                                    model: 3
                                    Rectangle {
                                        width: 3; height: 3; radius: 1.5
                                        color: moreMouse.containsMouse ? palette.iconHover : palette.iconNormal
                                    }
                                }
                            }

                            MouseArea {
                                id: moreMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton
                                cursorShape: Qt.PointingHandCursor
                                onClicked: (mouse) => {
                                    mouse.accepted = true;
                                    var pt = moreBtn.mapToItem(mainPanel, 0, moreBtn.height + 4);
                                    contextMenu.openMenu(model.chatId, pt.x, pt.y);
                                }
                            }
                        }
                    }
                }
            }

            // --- Footer Area (Settings Button) ---
            Item {
                id: footerArea
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 52
                z: 10

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    height: 1
                    color: palette.dividerColor
                    opacity: root.isCollapsed ? 0.0 : 1.0
                    visible: opacity > 0.0
                }

                Rectangle {
                    id: settingsBtn
                    width: 36; height: 36
                    radius: 18
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: root.isCollapsed ? undefined : parent.right
                    anchors.rightMargin: root.isCollapsed ? 0 : 14
                    anchors.horizontalCenter: root.isCollapsed ? parent.horizontalCenter : undefined

                    readonly property bool isAgentBusy: (typeof agentBridge !== "undefined") ? agentBridge.isWorking : false
                    enabled: !isAgentBusy
                    opacity: enabled ? 1.0 : 0.35

                    color: settingsMouse.containsMouse ? palette.iconHoverCircle : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on opacity { NumberAnimation { duration: 150 } }

                    Shape {
                        anchors.centerIn: parent
                        width: 18; height: 18

                        ShapePath {
                            strokeColor: settingsMouse.containsMouse ? palette.iconHover : palette.iconNormal
                            strokeWidth: 1.3
                            fillColor: "transparent"
                            capStyle: ShapePath.RoundCap
                            joinStyle: ShapePath.RoundJoin
                            scale: Qt.size(18 / 24, 18 / 24)
                            PathSvg {
                                path: "M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.09.63-.09.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"
                            }
                        }
                    }

                    MouseArea {
                        id: settingsMouse
                        anchors.fill: parent
                        hoverEnabled: parent.enabled
                        cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: root.settingsRequested()
                    }
                }
            }

            // --- Context Menu ---
            MouseArea {
                id: dismissArea
                anchors.fill: parent
                z: 90
                visible: contextMenu.isOpen
                enabled: contextMenu.isOpen
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: contextMenu.closeMenu()
            }

            Rectangle {
                id: contextMenu
                z: 100
                width: 128; height: 70
                radius: 10
                color: "#15161E"
                border.color: "#262837"
                border.width: 1

                property bool isOpen: false
                property string targetChatId: ""

                scale: isOpen ? 1.0 : 0.85
                opacity: isOpen ? 1.0 : 0.0
                visible: opacity > 0.0
                transformOrigin: Item.TopLeft

                Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                Behavior on scale { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }

                function openMenu(chatId, posX, posY) {
                    targetChatId = chatId;
                    var clX = Math.min(Math.max(8, posX), mainPanel.width - width - 10);
                    var clY = Math.min(Math.max(headerArea.height + 4, posY), mainPanel.height - height - 10);
                    x = clX; y = clY;
                    isOpen = true;
                }
                function closeMenu() { isOpen = false; }

                Column {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 2

                    Rectangle {
                        id: renameBtn
                        width: parent.width; height: 28; radius: 6
                        color: renameMouse.containsMouse ? "#232636" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }

                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left; anchors.leftMargin: 8
                            spacing: 8
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Rename"
                                font.pixelSize: 12; font.bold: true
                                color: renameMouse.containsMouse ? "#FFFFFF" : "#8E94A5"
                            }
                        }
                        MouseArea {
                            id: renameMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var cId = contextMenu.targetChatId;
                                contextMenu.closeMenu();
                                root.editingChatId = cId;
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
                        id: deleteBtn
                        width: parent.width; height: 28; radius: 6
                        color: deleteMouse.containsMouse ? "#232636" : "transparent"
                        Behavior on color { ColorAnimation { duration: 120 } }

                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left; anchors.leftMargin: 8
                            spacing: 8
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Delete"
                                font.pixelSize: 12; font.bold: true
                                color: deleteMouse.containsMouse ? "#FFFFFF" : "#8E94A5"
                            }
                        }
                        MouseArea {
                            id: deleteMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var targetId = contextMenu.targetChatId;
                                contextMenu.closeMenu();
                                if (typeof chatModel !== "undefined") {
                                    chatModel.deleteChat(targetId);
                                }
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
        ListElement { chatId: "1"; chatTitle: "Active Chat"; isActive: true }
    }
}