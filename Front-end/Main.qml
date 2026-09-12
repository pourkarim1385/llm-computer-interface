import QtQuick 2.15
import QtQuick.Window 2.15
import "components"
import "components/chat"
import "components/settings"

Window {
    id: window
    width: 960
    height: 700
    visible: true
    title: "AI Chat Interface"
    color: "#050608"

    Palette {
        id: appPalette
    }

    Item {
        id: rootContainer
        anchors.fill: parent

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

        // --- Main Assistant Layout ---
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

        // --- Settings View ---
        SettingsPage {
            id: settingsPage
            anchors.fill: parent
            onBackRequested: rootContainer.state = "assistant"
        }
    }
}