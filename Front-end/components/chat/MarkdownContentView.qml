import QtQuick 2.15

Item {
    id: root
    width: Math.min(parent.width * 0.72, 680)
    anchors.horizontalCenter: parent.horizontalCenter

    property string textData: ""

    readonly property bool hasText: textData !== undefined && textData !== ""

    visible: hasText
    height: hasText ? (markdownEditor.contentHeight + copyButton.height + 8) : 0

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

    //Copy Button
    Rectangle {
        id: copyButton
        width: 28
        height: 28
        radius: 14
        anchors.top: markdownEditor.bottom
        anchors.topMargin: 8
        anchors.left: parent.left

        color: copyArea.containsMouse ? "#27272A" : "transparent"

        Behavior on color {
            ColorAnimation { duration: 150 }
        }

        Canvas {
            id: copyIcon
            width: 14
            height: 14
            anchors.centerIn: parent

            property color iconColor: copyArea.containsMouse ? "#FFFFFF" : "#9CA3AF"
            onIconColorChanged: requestPaint()

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                ctx.strokeStyle = iconColor;
                ctx.lineWidth = 1.3;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";

                var fx = 4.5, fy = 1.5, fw = 8, fh = 9.5, fr = 1.8;
                ctx.beginPath();
                ctx.moveTo(fx + fr, fy);
                ctx.lineTo(fx + fw - fr, fy);
                ctx.arcTo(fx + fw, fy, fx + fw, fy + fr, fr);
                ctx.lineTo(fx + fw, fy + fh - fr);
                ctx.arcTo(fx + fw, fy + fh, fx + fw - fr, fy + fh, fr);
                ctx.lineTo(fx + fr, fy + fh);
                ctx.arcTo(fx, fy + fh, fx, fy + fh - fr, fr);
                ctx.lineTo(fx, fy + fr);
                ctx.arcTo(fx, fy, fx + fr, fy, fr);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(3.5, 4.0);
                ctx.lineTo(1.5 + fr, 4.0);
                ctx.arcTo(1.5, 4.0, 1.5, 4.0 + fr, fr);
                ctx.lineTo(1.5, 12.0);
                ctx.arcTo(1.5, 13.5, 3.0, 13.5, fr);
                ctx.lineTo(9.5, 13.5);
                ctx.stroke();
            }
        }

        MouseArea {
            id: copyArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                markdownEditor.selectAll();
                markdownEditor.copy();
                markdownEditor.deselect();
            }
        }
    }
}