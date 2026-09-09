import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    ListView {
        id: chatListView
        anchors.fill: parent
        anchors.bottomMargin: 100
        clip: true
        spacing: 24
        model: agentBridge.feedModel

        cacheBuffer: 1500

        reuseItems: true

        pixelAligned: true

        flickDeceleration: 2000
        maximumFlickVelocity: 4000
        boundsBehavior: Flickable.StopAtBounds

        onCountChanged: {
            Qt.callLater(function() {
                chatListView.positionViewAtEnd()
            })
        }

        delegate: Item {
            id: turnDelegate
            width: chatListView.width
            height: turnColumn.implicitHeight

            Column {
                id: turnColumn
                width: parent.width
                spacing: 16

                UserBubble {
                    text: model.userPrompt
                }

                AgentStatusSection {
                    visible: model.isPending && agentBridge.isWorking
                    height: visible ? implicitHeight : 0
                }

                MarkdownContentView {
                    textData: model.assistantMarkdown || ""
                }

                PlanCardView {
                    planObj: model.planData
                    visible: model.hasPlan === true
                    height: visible ? implicitHeight : 0
                }
            }
        }
    }
}