import QtQuick

Item {
    id: root
    width: 24
    height: 24

    property int speed: 260
    property real pixelSpacing: 0.8
    property int fadeDuration: 180

    readonly property var patterns: [
        [1.0, 1.0, 0.25,
            1.0, 0.25, 0.0,
            0.25, 0.1, 0.0],

        [1.0, 0.5, 0.1,
            1.0, 0.1, 0.0,
            0.5, 0.25, 0.0],

        [0.5, 0.25, 0.0,
            1.0, 0.1, 0.0,
            1.0, 0.5, 0.1],

        [0.25, 0.1, 0.0,
            0.5, 0.1, 0.1,
            1.0, 1.0, 0.25],

        [0.0, 0.0, 0.1,
            0.1, 0.5, 0.25,
            0.25, 1.0, 0.25],

        [0.0, 0.25, 0.9,
            0.0, 0.5, 0.2,
            0.1, 0.25, 0.6],

        [0.25, 0.9, 1.0,
            0.1, 0.6, 0.5,
            0.0, 0.1, 0.2],

        [0.1, 0.3, 0.3,
            0.05, 0.15, 0.15,
            0.0, 0.0, 0.05],

        [0.02, 0.02, 0.02,
            0.02, 0.02, 0.02,
            0.02, 0.02, 0.02],

        [0.4, 0.05, 0.4,
            0.4, 0.05, 0.4,
            0.0, 0.4, 0.1],

        [1.0, 0.1, 1.0,
            1.0, 0.1, 1.0,
            0.0, 1.0, 0.25],

        [0.5, 0.25, 1.0,
            0.5, 0.5, 0.50,
            0.25, 1.0, 0.1],

        [0.0, 0.5, 1.0,
            0.0, 1.0, 0.0,
            0.5, 1.0, 0.0],

        [0.0, 0.75, 1.0,
            0.0, 1.0, 0.0,
            0.75, 1.0, 0.0],

        [1.0, 1.0, 0.0,
            1.0, 0.0, 1.0,
            1.0, 0.0, 1.0],

        [0.2, 0.5, 0.0,
            0.2, 0.2, 0.0,
            0.0, 0.0, 0.0],

        [0.0, 1.0, 0.0,
            0.0, 0.5, 0.0,
            0.0, 0.0, 0.0],

        [0.0, 1.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 0.0],

        [0.0, 0.5, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 0.0],

        [0.05, 0.4, 0.05,
            0.4, 0.8, 0.4,
            0.05, 0.4, 0.05],

        [0.1, 1.0, 0.1,
            1.0, 1.0, 1.0,
            0.1, 1.0, 0.1],

        [1.0, 1.0, 1.0,
            1.0, 0.15, 1.0,
            1.0, 1.0, 1.0],

        [0.15, 0.25, 0.15,
            0.25, 1.0, 0.25,
            0.15, 0.25, 0.15],

        [1.0, 0.1, 0.1,
            1.0, 0.1, 0.1,
            1.0, 1.0, 0.1],

        [0.9, 0.1, 0.9,
            0.9, 0.1, 0.9,
            0.9, 0.1, 0.9]
    ]

    readonly property var colors: [
        "#FF5E5E",
        "#FFA07A",
        "#4EA8DE",
        "#64B5F6"
    ]

    property int currentPatternIndex: 0
    property int currentColorIndex: 0

    property color activeColor: colors[currentColorIndex]
    Behavior on activeColor {
        ColorAnimation { duration: 500; easing.type: Easing.InOutQuad }
    }

    Grid {
        id: grid
        anchors.centerIn: parent
        rows: 3
        columns: 3
        spacing: root.pixelSpacing

        readonly property real cellSize: (root.width - (spacing * 2)) / 3

        Repeater {
            model: 9
            delegate: Rectangle {
                id: cell
                width: grid.cellSize
                height: grid.cellSize
                radius: 1
                color: root.activeColor

                readonly property real targetOpacity: root.patterns[root.currentPatternIndex][index]

                opacity: targetOpacity
                Behavior on opacity {
                    NumberAnimation {
                        duration: root.fadeDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }

    Timer {
        id: stepTimer
        interval: root.speed
        running: true
        repeat: true
        onTriggered: {
            root.currentPatternIndex = (root.currentPatternIndex + 1) % root.patterns.length

            if (root.currentPatternIndex === 0) {
                root.currentColorIndex = (root.currentColorIndex + 1) % root.colors.length
            }
        }
    }
}