import QtQuick 2.15
import QtQuick.Window 2.15
import "components"
import "components/chat"
import "components/settings"

Window {
    id: window
    width: 960
    height: 700
    minimumWidth: 600
    minimumHeight: 450
    visible: true
    title: "llm-computer-interface"
    color: "#050608"

    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowMinMaxButtonsHint

    Palette {
        id: appPalette
    }

    Rectangle {
        id: customTitleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 36
        color: "#050608"
        z: 100

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton

            onPressed: {
                if (mouse.button === Qt.LeftButton) {
                    window.startSystemMove()
                }
            }

            onDoubleClicked: {
                if (window.visibility === Window.Maximized) {
                    window.showNormal()
                } else {
                    window.showMaximized()
                }
            }
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: "llm-computer-interface"
            color: "#8e939d"
            font.pixelSize: 12
            font.family: "Segoe UI, Inter, sans-serif"
            font.weight: Font.Normal
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            Rectangle {
                id: btnMinimize
                width: 32
                height: 26
                radius: 5
                color: minMouse.pressed ? "#383a42" : (minMouse.containsMouse ? "#282a30" : "transparent")

                Behavior on color { ColorAnimation { duration: 120 } }

                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 1.6
                    radius: 0.8
                    color: minMouse.containsMouse ? "#ffffff" : "#8e939d"
                }

                MouseArea {
                    id: minMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.ArrowCursor
                    onClicked: window.showMinimized()
                }
            }

            Rectangle {
                id: btnMaximize
                width: 32
                height: 26
                radius: 5
                color: maxMouse.pressed ? "#383a42" : (maxMouse.containsMouse ? "#282a30" : "transparent")

                Behavior on color { ColorAnimation { duration: 120 } }

                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 10
                    radius: 2.5
                    color: "transparent"
                    border.width: 1.4
                    border.color: maxMouse.containsMouse ? "#ffffff" : "#8e939d"
                }

                MouseArea {
                    id: maxMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.ArrowCursor
                    onClicked: {
                        if (window.visibility === Window.Maximized) {
                            window.showNormal()
                        } else {
                            window.showMaximized()
                        }
                    }
                }
            }

            Rectangle {
                id: btnClose
                width: 32
                height: 26
                radius: 5
                color: closeMouse.pressed ? "#7e1212" : (closeMouse.containsMouse ? "#a11818" : "transparent")

                Behavior on color { ColorAnimation { duration: 120 } }

                // آیکون ضربدر بُرداری
                Item {
                    anchors.centerIn: parent
                    width: 10
                    height: 10

                    Rectangle {
                        anchors.centerIn: parent
                        width: 12
                        height: 1.5
                        radius: 0.75
                        rotation: 45
                        antialiasing: true
                        color: closeMouse.containsMouse ? "#ffffff" : "#8e939d"
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: 12
                        height: 1.5
                        radius: 0.75
                        rotation: -45
                        antialiasing: true
                        color: closeMouse.containsMouse ? "#ffffff" : "#8e939d"
                    }
                }

                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.ArrowCursor
                    onClicked: window.close()
                }
            }
        }
    }

    Item {
        id: resizeBorders
        anchors.fill: parent
        z: 999
        visible: window.visibility !== Window.Maximized && window.visibility !== Window.FullScreen

        readonly property int borderWidth: 6

        MouseArea {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: resizeBorders.borderWidth
            anchors.rightMargin: resizeBorders.borderWidth
            height: resizeBorders.borderWidth
            cursorShape: Qt.SizeVerCursor
            onPressed: window.startSystemResize(Qt.TopEdge)
        }
        MouseArea {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: resizeBorders.borderWidth
            anchors.rightMargin: resizeBorders.borderWidth
            height: resizeBorders.borderWidth
            cursorShape: Qt.SizeVerCursor
            onPressed: window.startSystemResize(Qt.BottomEdge)
        }
        MouseArea {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: resizeBorders.borderWidth
            anchors.bottomMargin: resizeBorders.borderWidth
            width: resizeBorders.borderWidth
            cursorShape: Qt.SizeHorCursor
            onPressed: window.startSystemResize(Qt.LeftEdge)
        }
        MouseArea {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: resizeBorders.borderWidth
            anchors.bottomMargin: resizeBorders.borderWidth
            width: resizeBorders.borderWidth
            cursorShape: Qt.SizeHorCursor
            onPressed: window.startSystemResize(Qt.RightEdge)
        }
        MouseArea {
            anchors.top: parent.top
            anchors.left: parent.left
            width: resizeBorders.borderWidth
            height: resizeBorders.borderWidth
            cursorShape: Qt.SizeFDiagCursor
            onPressed: window.startSystemResize(Qt.TopEdge | Qt.LeftEdge)
        }
        MouseArea {
            anchors.top: parent.top
            anchors.right: parent.right
            width: resizeBorders.borderWidth
            height: resizeBorders.borderWidth
            cursorShape: Qt.SizeBDiagCursor
            onPressed: window.startSystemResize(Qt.TopEdge | Qt.RightEdge)
        }
        MouseArea {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: resizeBorders.borderWidth
            height: resizeBorders.borderWidth
            cursorShape: Qt.SizeBDiagCursor
            onPressed: window.startSystemResize(Qt.BottomEdge | Qt.LeftEdge)
        }
        MouseArea {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: resizeBorders.borderWidth
            height: resizeBorders.borderWidth
            cursorShape: Qt.SizeFDiagCursor
            onPressed: window.startSystemResize(Qt.BottomEdge | Qt.RightEdge)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "#181a20"
        border.width: 1
        z: 998
        visible: window.visibility !== Window.Maximized && window.visibility !== Window.FullScreen
    }

    Item {
        id: rootContainer
        anchors.top: customTitleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        state: "assistant"

        states: [
            State {
                name: "assistant"
                PropertyChanges { target: mainChatLayout; opacity: 1.0; visible: true; x: 0 }
                PropertyChanges { target: settingsPage; opacity: 0.0; visible: false; x: 40 }
            },
            State {
                name: "settings"
                PropertyChanges { target: mainChatLayout; opacity: 0.0; visible: false; x: -40 }
                PropertyChanges { target: settingsPage; opacity: 1.0; visible: true; x: 0 }
            }
        ]

        transitions: [
            Transition {
                from: "assistant"; to: "settings"
                ParallelAnimation {
                    NumberAnimation { target: mainChatLayout; properties: "opacity,x"; duration: 250; easing.type: Easing.OutCubic }
                    NumberAnimation { target: settingsPage; properties: "opacity,x"; duration: 280; easing.type: Easing.OutCubic }
                }
            },
            Transition {
                from: "settings"; to: "assistant"
                ParallelAnimation {
                    NumberAnimation { target: settingsPage; properties: "opacity,x"; duration: 240; easing.type: Easing.OutCubic }
                    NumberAnimation { target: mainChatLayout; properties: "opacity,x"; duration: 260; easing.type: Easing.OutCubic }
                }
            }
        ]

        //Main Assistant Layout
        Row {
            id: mainChatLayout
            anchors.fill: parent
            spacing: 0

            LeftSidebar {
                id: leftSidebar
                height: parent.height
                onSettingsRequested: {
                    if (typeof agentBridge === "undefined" || !agentBridge.isWorking) {
                        rootContainer.state = "settings";
                    }
                }
            }

            Item {
                id: rightContentContainer
                width: parent.width - leftSidebar.width
                height: parent.height

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 14
                    radius: 20
                    color: "#050608"
                    border.color: "#050608"
                    border.width: 1

                    ChatFeedView {
                        anchors.fill: parent
                    }

                    AppendedFilesList {
                        id: appendedFilesList
                        anchors.horizontalCenter: chatInput.horizontalCenter
                        anchors.bottom: chatInput.top
                        anchors.bottomMargin: 8
                        z: 2

                        onEditPropertiesRequested: (idx) => {
                            propertiesPopup.openForIndex(idx)
                        }
                    }

                    GlowingTextBox {
                        id: chatInput
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 24
                        z: 5

                        onSubmitted: (prompt, flags) => {
                            if (typeof inputBoxController !== "undefined") {
                                if (inputBoxController.sendMessage(prompt, flags)) {
                                    chatInput.clear()
                                }
                            }
                        }
                    }

                    AppendPropertiesPopup {
                        id: propertiesPopup
                        z: 99
                    }
                }
            }
        }

        //Settings View
        SettingsPage {
            id: settingsPage
            anchors.fill: parent
            onBackRequested: rootContainer.state = "assistant"
        }
    }
}