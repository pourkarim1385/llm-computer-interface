import QtQuick 2.15

Item {
    id: root
    width: Math.min(parent.width * 0.72, 680)
    anchors.horizontalCenter: parent.horizontalCenter
    property var planObj: null

    readonly property bool hasValidSteps: !!(planObj && planObj.steps && planObj.steps.length > 0)

    height: hasValidSteps ? cardBg.height : 0
    visible: hasValidSteps

    Rectangle {
        id: cardBg
        width: parent.width
        height: colLayout.height + 36
        radius: 18
        color: "#11131C"
        border.color: "#222536"
        border.width: 1

        Column {
            id: colLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 18
            spacing: 12

            Text {
                // استفاده از fallback امن برای جلوگیری از ارور
                text: "Plan: " + ((root.planObj && root.planObj.name) ? root.planObj.name : "")
                font.pixelSize: 14
                font.bold: true
                color: "#FFFFFF"
                width: parent.width
                wrapMode: Text.Wrap
            }

            Text {
                text: (root.planObj && root.planObj.description) ? root.planObj.description : ""
                font.pixelSize: 13
                color: "#9A9EAE"
                width: parent.width
                wrapMode: Text.Wrap
                visible: text.length > 0
            }

            Item { width: 1; height: 4 }

            Repeater {
                model: root.hasValidSteps ? root.planObj.steps : []

                delegate: Row {
                    spacing: 12
                    width: colLayout.width

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 18
                        height: 18
                        radius: 9
                        color: "transparent"
                        border.color: (modelData && modelData.isDone) ? "#7C3AED" : "#45485E"
                        border.width: 1.8

                        Rectangle {
                            anchors.centerIn: parent
                            width: 8
                            height: 8
                            radius: 4
                            color: "#7C3AED"
                            visible: (modelData && modelData.isDone) ? true : false
                        }
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: (modelData && modelData.title) ? modelData.title : ""
                        font.pixelSize: 13
                        color: (modelData && modelData.isDone) ? "#E1E4EE" : "#8A8E9E"
                        width: parent.width - 32
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
}