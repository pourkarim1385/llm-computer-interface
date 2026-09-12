import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Effects
import QtQuick.Dialogs
import ".."

Item {
    id: root
    width: 650
    height: 104

    Palette { id: palette }

    property real glowIntensity: 0.3
    property real targetGlow: 0.3
    property real rotationAngle: 0.0
    property real rotationSpeed: 0.02
    property real lastKeyTime: 0
    property alias text: textInput.text

    readonly property bool isBusy: (typeof agentBridge !== "undefined") ? agentBridge.isWorking : false

    signal submitted(string prompt, var observationFlags)
    signal stopRequested()

    function clear() {
        textInput.clear()
    }

    function submitPrompt() {
        let trimmed = textInput.text.trim()
        if (trimmed.length === 0 || root.isBusy) return
        root.submitted(trimmed, modifyPopup.getFlagsMap())
    }

    function handleButtonAction() {
        if (root.isBusy) {
            root.stopRequested()
            if (typeof agentBridge !== "undefined" && agentBridge.stopExecution) {
                agentBridge.stopExecution()
            }
        } else {
            root.submitPrompt()
        }
    }

    function getRightEdgeColor(angleRad) {
        let twoPi = Math.PI * 2
        let norm = ((-angleRad % twoPi) + twoPi) % twoPi / twoPi

        let cPurple = Qt.rgba(0.486, 0.227, 0.929, 1.0)
        let cPink   = Qt.rgba(0.925, 0.282, 0.600, 1.0)
        let cOrange = Qt.rgba(0.976, 0.451, 0.086, 1.0)

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

    // Border Canvas
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

    // Glow Layer
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

        Item {
            anchors.fill: parent
            anchors.margins: 14

            // Prompt Input Field
            TextField {
                id: textInput
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: 80
                height: 38
                verticalAlignment: TextInput.AlignVCenter
                placeholderText: "Ask anything..."
                placeholderTextColor: "#6B7280"
                color: "#FFFFFF"
                font.pixelSize: 15
                font.weight: Font.Medium
                selectByMouse: true
                background: null

                onTextChanged: root.registerKeystroke()
                onAccepted: root.submitPrompt()

                cursorDelegate: Item {
                    width: 2.5
                    height: 22

                    Rectangle {
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
            }

            // Bottom-Left: Modify Observation/Thinking Button
            ModifyPillButton {
                id: modifyBtn
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2
                active: modifyPopup.visible
                onClicked: {
                    if (modifyPopup.visible) {
                        modifyPopup.close()
                    } else {
                        modifyPopup.open()
                    }
                }
            }

            ModifyDropdownPopup {
                id: modifyPopup
                x: modifyBtn.x
                y: modifyBtn.y - height - 8
            }

            // Bottom-Right / Right Row: Import & Send Buttons
            Row {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2
                spacing: 10

                // Import / Attach Files Button
                Rectangle {
                    id: importButton
                    width: 30
                    height: 30
                    radius: 15
                    anchors.verticalCenter: parent.verticalCenter
                    color: importMouse.pressed ? "#2B2E42" : (importMouse.containsMouse ? "#222534" : "transparent")
                    scale: importMouse.pressed ? 0.92 : (importMouse.containsMouse ? 1.08 : 1.0)

                    Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                        Canvas {
                            anchors.centerIn: parent
                            width: 18
                            height: 18
                            onPaint: {
                                let ctx = getContext("2d")
                                ctx.reset()
                                ctx.strokeStyle = importMouse.containsMouse ? "#C084FC" : "#8B90A0"
                                ctx.lineWidth = 1.6
                                ctx.lineCap = "round"

                                ctx.beginPath()
                                ctx.moveTo(4, 9)
                                ctx.lineTo(14, 9)
                                ctx.moveTo(9, 4)
                                ctx.lineTo(9, 14)

                                ctx.stroke()
                            }
                        }

                    MouseArea {
                        id: importMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: importButton.children[0].requestPaint()
                        onExited: importButton.children[0].requestPaint()
                        onClicked: importMenu.open()
                    }

                    ImportMenuPopup {
                        id: importMenu
                        y: -height - 6
                        x: -width + 32
                        onAddFilesClicked: fileDialog.open()
                        onAddFolderClicked: folderDialog.open()
                    }
                }

                // Send / Stop Button
                SendButton {
                    id: sendButton
                    width: 32
                    height: 32
                    anchors.verticalCenter: parent.verticalCenter
                    activeColor: root.getRightEdgeColor(root.rotationAngle)
                    isStop: root.isBusy
                    isActive: root.isBusy || (textInput.text.trim().length > 0)
                    onClicked: root.handleButtonAction()
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select Files to Append"
        fileMode: FileDialog.OpenFiles
        onAccepted: {
            console.log("[GlowingTextBox] FileDialog accepted, files count:", selectedFiles.length)
            if (typeof appendedFilesModel !== "undefined") {
                for (let i = 0; i < selectedFiles.length; ++i) {
                    appendedFilesModel.addFile(selectedFiles[i])
                }
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Select Folder to Append"
        onAccepted: {
            console.log("[GlowingTextBox] FolderDialog accepted, folder:", selectedFolder)
            if (typeof appendedFilesModel !== "undefined") {
                appendedFilesModel.addFolder(selectedFolder)
            }
        }
    }
}