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

        property int prevCount: 0

        footer: Item {
            width: chatListView.width
            height: Math.max(0, chatListView.height - 180)
        }

        NumberAnimation {
            id: scrollAnimation
            target: chatListView
            property: "contentY"
            duration: 380
            easing.type: Easing.OutCubic
        }

        onDragStarted: scrollAnimation.stop()

        function scrollToTurnBeginning(index, animated) {
            if (index < 0 || index >= chatListView.count) return;

            if (!animated) {
                chatListView.positionViewAtIndex(index, ListView.Beginning);
                return;
            }

            var startY = chatListView.contentY;
            chatListView.positionViewAtIndex(index, ListView.Beginning);
            var targetY = chatListView.contentY;
            chatListView.contentY = startY;

            scrollAnimation.stop();
            scrollAnimation.from = startY;
            scrollAnimation.to = targetY;
            scrollAnimation.start();
        }

        onCountChanged: {
            Qt.callLater(function() {
                if (chatListView.count <= 0) {
                    chatListView.prevCount = 0;
                    return;
                }

                if (chatListView.count - chatListView.prevCount === 1 && chatListView.prevCount > 0) {
                    chatListView.scrollToTurnBeginning(chatListView.count - 1, true);
                }
                else if (chatListView.prevCount === 0) {
                    chatListView.positionViewAtEnd();
                }

                chatListView.prevCount = chatListView.count;
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