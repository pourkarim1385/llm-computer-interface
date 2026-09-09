import QtQuick 2.15
import QtQuick.Window 2.15
import "components"

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
            id: contentContainer
            width: parent.width - leftSidebar.width
            height: parent.height

            Rectangle {
                anchors.fill: parent
                anchors.margins: 14
                radius: 20
                color: "#12141c"
                border.color: "#1a1d28"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "Main Content Area (Ready for Backend Integration)"
                    color: "#8b90a0"
                    font.pixelSize: 14
                }

                GlowingTextBox {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 24

                    onSubmitted: (prompt) => {
                        inputBoxController.sendMessage(prompt)
                    }
                }
            }
        }
    }
}