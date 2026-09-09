import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import ".."

Item {
    id: root
    width: 650
    height: 80

    Palette { id: palette }

    property real glowIntensity: 0.3
    property real targetGlow: 0.3
    property real rotationAngle: 0.0
    property real rotationSpeed: 0.02
    property real lastKeyTime: 0
    property alias text: textInput.text

    signal submitted(string prompt)

    function clear() {
        textInput.clear()
    }

    function getRightEdgeColor(angleRad) {
            let twoPi = Math.PI * 2
            let norm = ((-angleRad % twoPi) + twoPi) % twoPi / twoPi

            let cPurple = Qt.rgba(0.486, 0.227, 0.929, 1.0) // #7C3AED
            let cPink   = Qt.rgba(0.925, 0.282, 0.600, 1.0) // #EC4899
            let cOrange = Qt.rgba(0.976, 0.451, 0.086, 1.0) // #F97316

            function lerpColor(c1, c2, t) {
                let clampedT = Math.max(0.0, Math.min(1.0, t))
                return Qt.rgba(
                    c1.r + (c2.r - c1.r) * clampedT,
                    c1.g + (c2.g - c1.g) * clampedT,
                    c1.b + (c2.b - c1.b) * clampedT,
                    1.0
                )
            }

            if (norm < 0.25) {
                return lerpColor(cPurple, cPink, norm / 0.25)
            } else if (norm < 0.45) {
                return lerpColor(cPink, cOrange, (norm - 0.25) / 0.20)
            } else if (norm < 0.85) {
                return lerpColor(cOrange, cPurple, (norm - 0.45) / 0.40)
            } else {
                return cPurple
            }
        }

    //
    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: {
            root.rotationAngle = (root.rotationAngle + root.rotationSpeed) % (Math.PI * 2)
            root.rotationSpeed = Math.max(0.015, root.rotationSpeed * 0.95)
            root.glowIntensity = root.glowIntensity + (root.targetGlow - root.glowIntensity) * 0.1
            root.targetGlow = Math.max(0.3, root.targetGlow * 0.92)

            borderCanvas.requestPaint()
        }
    }

    function registerKeystroke() {
        let currentTime = Date.now()
        let delta = Math.max(30, currentTime - root.lastKeyTime)
        root.lastKeyTime = currentTime

        let boost = Math.min(0.2, 18.0 / delta)
        root.rotationSpeed = Math.min(0.25, root.rotationSpeed + boost)
        root.targetGlow = Math.min(1.0, root.targetGlow + 0.35)
    }

    //Border
    Canvas {
        id: borderCanvas
        anchors.fill: parent
        anchors.margins: -15
        renderTarget: Canvas.FramebufferObject
        renderStrategy: Canvas.Threaded

        onPaint: {
            let ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)

            let pad = 15
            let bx = pad
            let by = pad
            let bw = width - (pad * 2)
            let bh = height - (pad * 2)
            let r = 24

            // Gradient
            let grad = ctx.createConicalGradient(width / 2, height / 2, root.rotationAngle)
            grad.addColorStop(0.00, palette.textBoxGlowColorOne)
            grad.addColorStop(0.25, palette.textBoxGlowColorTwo)
            grad.addColorStop(0.45, palette.textBoxGlowColorThree)
            grad.addColorStop(0.55, "transparent")
            grad.addColorStop(0.85, "transparent")
            grad.addColorStop(1.00, palette.textBoxGlowColorOne)

            ctx.beginPath()
            ctx.roundedRect(bx, by, bw, bh, r, r)
            ctx.lineWidth = 2.5
            ctx.strokeStyle = grad
            ctx.stroke()
        }
    }

    //Glow Layer
    MultiEffect {
        source: borderCanvas
        anchors.fill: borderCanvas
        blurEnabled: true
        blur: root.glowIntensity * 0.9
        blurMax: 32
        opacity: root.glowIntensity * 1.5
    }

    Rectangle {
        id: innerBg
        anchors.fill: parent
        radius: 24
        color: "#16161E"

        Row {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Item {
                    id: inputContainer
                    width: parent.width - 20
                    height: parent.height

                    MultiEffect {
                        source: textInput
                        anchors.fill: textInput
                        blurEnabled: true
                        blur: 0.35
                        blurMax: 16
                        opacity: 0.6
                    }

                    TextField {
                        id: textInput
                        anchors.fill: parent
                        anchors.topMargin: 0
                        anchors.bottomMargin: 26
                        verticalAlignment: TextInput.AlignVCenter
                        placeholderText: "Ask Anything..."
                        placeholderTextColor: "#6B7280"
                        color: "#FFFFFF"
                        font.pixelSize: 15
                        font.weight: Font.Medium
                        selectByMouse: true
                        background: null

                        onTextChanged: root.registerKeystroke()

                        onAccepted: {
                            if (textInput.text.trim().length > 0) {
                                root.submitted(textInput.text)
                                textInput.text = ""
                            }
                        }

                        cursorDelegate: Item {
                            width: 2.5
                            height: 22

                            Rectangle {
                                id: cursorBody
                                anchors.fill: parent
                                radius: 1.5
                                color: "#FFFFFF"

                                SequentialAnimation on opacity {
                                    loops: Animation.Infinite
                                    running: textInput.activeFocus
                                    NumberAnimation { to: 0.2; duration: 450; easing.type: Easing.InOutQuad }
                                    NumberAnimation { to: 1.0; duration: 450; easing.type: Easing.InOutQuad }
                                }
                            }
                        }

                        SendButton {
                            id: sendButton
                            x: 595
                            y: 0
                            width: 27
                            height: 27
                            activeColor: root.getRightEdgeColor(root.rotationAngle)
                            anchors.right: parent.right
                            anchors.rightMargin: -24
                            anchors.verticalCenterOffset: 3
                            isActive: textInput.text.trim().length > 0
                            onClicked: {
                                root.submitted(textInput.text)
                                textInput.text = ""
                            }
                        }
                    }
            }
        }
    }
}
