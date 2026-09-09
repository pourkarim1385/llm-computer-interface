import QtQuick 2.15

Item {
    id: root
    width: Math.min(parent.width * 0.72, 680)
    anchors.horizontalCenter: parent.horizontalCenter

    property string textData: ""

    // اطمینان از اینکه متن وجود دارد
    readonly property bool hasText: textData !== undefined && textData !== ""

    visible: hasText
    height: hasText ? markdownEditor.contentHeight : 0

    TextEdit {
        id: markdownEditor
        width: parent.width
        height: contentHeight

        text: root.textData || ""
        textFormat: TextEdit.MarkdownText
        readOnly: true
        selectByMouse: true
        selectionColor: "#7C3AED"
        selectedTextColor: "#FFFFFF"

        wrapMode: TextEdit.Wrap
        color: "#EDEDED"
        font.pixelSize: 14
        font.weight: Font.Normal
        horizontalAlignment: Text.AlignAuto

        cursorDelegate: Item {}
    }
}