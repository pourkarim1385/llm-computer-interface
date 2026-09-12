import QtQuick 2.15
import QtQuick.Window 2.15
import "components"
import "components/chat"

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

    Row {
        anchors.fill: parent
        spacing: 0

        LeftSidebar {
            id: leftSidebar
            height: parent.height
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
}