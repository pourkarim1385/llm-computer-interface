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

        onCountChanged: {
            Qt.callLater(function() {
                chatListView.positionViewAtEnd()
            })
        }

        delegate: Item {
            id: turnDelegate
            width: chatListView.width
            height: turnColumn.height

            Column {
                id: turnColumn
                width: parent.width
                spacing: 16

                UserBubble {
                    text: model.userPrompt
                }

                AgentStatusSection {
                    visible: model.isPending && agentBridge.isWorking
                }

                MarkdownContentView {
                    textData: model.assistantMarkdown || ""
                }

                PlanCardView {
                    planObj: model.planData
                    visible: model.hasPlan === true
                }
            }
        }
    }
}