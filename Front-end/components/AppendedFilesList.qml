import QtQuick 2.15
import QtQuick.Controls 2.15
import ".."

Item {
    id: root
    width: 650

    readonly property int itemsCount: (typeof appendedFilesModel !== "undefined" && appendedFilesModel) ? appendedFilesModel.count : 0
    property int maxHeight: 160

    height: itemsCount > 0 ? Math.min(maxHeight, (itemsCount * 44) + 24) : 0
    visible: opacity > 0.01

    signal editPropertiesRequested(int index)

    opacity: itemsCount > 0 ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
    }
    Behavior on height {
        NumberAnimation { duration: 220; easing.type: Easing.OutQuad }
    }

    transform: Translate {
        y: root.itemsCount > 0 ? 0 : 35
        Behavior on y {
            NumberAnimation { duration: 280; easing.type: Easing.OutCubic }
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: 16
        color: "#11121A"
        border.color: "#222533"
        border.width: 1

        ListView {
            id: filesListView
            anchors.fill: parent
            anchors.margins: 12
            anchors.rightMargin: 16
            spacing: 6
            clip: true
            model: (typeof appendedFilesModel !== "undefined") ? appendedFilesModel : null

            delegate: AppendedFileRow {
                width: filesListView.width
                itemIndex: index
                filePath: model.filePath
                fileName: model.fileName
                isFolder: model.isFolder

                onEditRequested: (idx) => root.editPropertiesRequested(idx)
                onRemoveRequested: (idx) => {
                    if (typeof appendedFilesModel !== "undefined") {
                        appendedFilesModel.removeAt(idx)
                    }
                }
            }
        }

        Rectangle {
            id: customScrollBar
            anchors.right: parent.right
            anchors.rightMargin: 6
            anchors.top: parent.top
            anchors.topMargin: 12
            anchors.bottomMargin: 12
            width: 4
            radius: 2
            color: "#4A4D62"
            visible: filesListView.visibleArea.heightRatio < 1.0
            y: parent.height * filesListView.visibleArea.yPosition
            height: Math.max(20, parent.height * filesListView.visibleArea.heightRatio)
        }
    }
}