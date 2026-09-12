import QtQuick 2.15
import QtQuick.Controls.Basic 2.15

Item {
    id: root
    width: parent.width
    height: 60

    property string labelText: "API format"
    property int currentIndex: 0
    property bool readOnly: false
    readonly property var modelOptions: ["OpenAI Compatible", "Anthropic", "Ollama", "Gemini", "Custom"]

    Column {
        anchors.fill: parent
        spacing: 6

        Text {
            text: root.labelText
            color: "#E2E8F0"
            font.pixelSize: 13
            font.bold: true
        }

        ComboBox {
            id: combo
            width: parent.width
            height: 38
            model: root.modelOptions
            currentIndex: root.currentIndex
            enabled: !root.readOnly

            onCurrentIndexChanged: root.currentIndex = combo.currentIndex

            background: Rectangle {
                radius: 8
                color: root.readOnly ? "#10121A" : (combo.activeFocus ? "#131622" : (combo.hovered ? "#151824" : "#12141D"))
                border.color: combo.activeFocus ? "#7C3AED" : (combo.hovered ? "#2D3247" : "#1D202D")
                border.width: 1
            }

            contentItem: Text {
                leftPadding: 12
                text: combo.displayText
                font.pixelSize: 12
                color: root.readOnly ? "#9CA3AF" : "#FFFFFF"
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            indicator: Canvas {
                x: combo.width - width - 12
                y: (combo.height - height) / 2
                width: 10; height: 6
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();
                    ctx.moveTo(0, 0);
                    ctx.lineTo(width, 0);
                    ctx.lineTo(width / 2, height);
                    ctx.closePath();
                    ctx.fillStyle = combo.hovered ? "#FFFFFF" : "#8B90A0";
                    ctx.fill();
                }
            }

            popup: Popup {
                y: combo.height + 4
                width: combo.width
                implicitHeight: contentItem.implicitHeight
                padding: 4

                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: combo.popup.visible ? combo.delegateModel : null
                    currentIndex: combo.highlightedIndex
                }

                background: Rectangle {
                    color: "#161822"
                    border.color: "#262938"
                    border.width: 1
                    radius: 8
                }
            }

            delegate: ItemDelegate {
                width: combo.width
                height: 32
                highlighted: combo.highlightedIndex === index

                contentItem: Text {
                    text: modelData
                    color: highlighted ? "#FFFFFF" : "#8B90A0"
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: highlighted ? "#252838" : "transparent"
                    radius: 6
                }
            }
        }
    }
}