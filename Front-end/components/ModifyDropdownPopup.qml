import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id: root
    width: 290
    padding: 0
    modal: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    transformOrigin: Popup.BottomLeft

    // Navigation State: "root", "model", "observation"
    property string currentView: "root"

    // Default Observation Flags state matching struct ObservationFlags
    property bool flagScreen: true
    property bool flagFullAccessibility: true
    property bool flagActiveWindowAccessibility: false
    property bool flagClipboard: true
    property bool flagSystemInfo: true
    property bool flagNewScreenMetrics: false

    function getFlagsMap() {
        return {
            "captureVision": flagScreen,
            "captureFullAccessibility": flagFullAccessibility,
            "captureActiveWindowAccessibility": flagActiveWindowAccessibility,
            "captureTargetWindowAccessibility": false,
            "targetWindow": "",
            "captureClipboard": flagClipboard,
            "captureDesktop": flagSystemInfo,
            "captureNewScreenMetrics": flagNewScreenMetrics
        }
    }

    onClosed: {
        currentView = "root"
    }

    // Smooth Pop-in & Pop-out Animations
    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 180; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.93; to: 1.0; duration: 200; easing.type: Easing.OutBack }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 140; easing.type: Easing.InCubic }
            NumberAnimation { property: "scale"; from: 1.0; to: 0.95; duration: 140; easing.type: Easing.InCubic }
        }
    }

    background: Rectangle {
        radius: 18
        color: "#161724"
        border.color: "#282B3E"
        border.width: 1
    }

    contentItem: Item {
        id: container
        clip: true
        implicitHeight: {
            if (root.currentView === "root") return rootView.implicitHeight
            if (root.currentView === "model") return modelView.implicitHeight
            if (root.currentView === "observation") return obsView.implicitHeight
            return 100
        }

        Behavior on implicitHeight {
            NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
        }

        // ==========================================
        // 1. ROOT MENU VIEW
        // ==========================================
        Item {
            id: rootView
            width: parent.width
            implicitHeight: rootCol.implicitHeight + 16

            x: root.currentView === "root" ? 0 : -width * 0.35
            opacity: root.currentView === "root" ? 1.0 : 0.0
            visible: opacity > 0.01

            Behavior on x { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
            Behavior on opacity { NumberAnimation { duration: 200 } }

            Column {
                id: rootCol
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 4

                // Model Row Button
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 42
                    radius: 10
                    color: rootModelMouse.pressed ? "#2B2E44" : (rootModelMouse.containsMouse ? "#24273A" : "transparent")

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.right: modelArrow.left
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Text {
                            text: "Model"
                            color: "#FFFFFF"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: {
                                if (typeof agentBridge !== "undefined" && agentBridge.activeProviderName.length > 0)
                                    return "(" + agentBridge.activeProviderName + ")"
                                return "(#name of active provider)"
                            }
                            color: "#7E849A"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                            width: Math.min(implicitWidth, 140)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        id: modelArrow
                        text: ">"
                        color: "#7E849A"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    MouseArea {
                        id: rootModelMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.currentView = "model"
                    }
                }

                // Divider Line
                Rectangle {
                    width: parent.width - 24
                    height: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#25283A"
                }

                // Observation Row Button
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 42
                    radius: 10
                    color: rootObsMouse.pressed ? "#2B2E44" : (rootObsMouse.containsMouse ? "#24273A" : "transparent")

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        text: "Observation"
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: ">"
                        color: "#7E849A"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    MouseArea {
                        id: rootObsMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.currentView = "observation"
                    }
                }
            }
        }

        // ==========================================
        // 2. MODEL SUBMENU VIEW (Scrollable + Inset)
        // ==========================================
        Item {
            id: modelView
            width: parent.width
            implicitHeight: modelCol.implicitHeight + 16

            x: root.currentView === "model" ? 0 : width
            opacity: root.currentView === "model" ? 1.0 : 0.0
            visible: opacity > 0.01

            Behavior on x { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
            Behavior on opacity { NumberAnimation { duration: 200 } }

            Column {
                id: modelCol
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 4

                // Back Button
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 10
                    color: backModelMouse.pressed ? "#2B2E44" : (backModelMouse.containsMouse ? "#24273A" : "transparent")

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Text {
                            text: "<"
                            color: backModelMouse.containsMouse ? "#FFFFFF" : "#8B90A0"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Back"
                            color: backModelMouse.containsMouse ? "#FFFFFF" : "#8B90A0"
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: backModelMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.currentView = "root"
                    }
                }

                // Divider Line
                Rectangle {
                    width: parent.width - 24
                    height: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#25283A"
                }

                // Scrollable Providers ListView with Max Height
                ListView {
                    id: modelListView
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    implicitHeight: Math.min(contentHeight, 220)
                    height: implicitHeight
                    spacing: 2

                    model: (typeof agentBridge !== "undefined") ? agentBridge.providers : []

                    delegate: Rectangle {
                        width: modelListView.width
                        height: 38
                        radius: 10
                        color: (modelData && modelData.isActive) ? "#272A3E" : (provMouse.containsMouse ? "#1F2233" : "transparent")

                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            id: providerNameText
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.right: selectedBadge.visible ? selectedBadge.left : parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            text: (modelData && modelData.displayName) ? modelData.displayName : ""
                            color: "#FFFFFF"
                            font.pixelSize: 12
                            font.weight: (modelData && modelData.isActive) ? Font.DemiBold : Font.Normal
                            elide: Text.ElideRight
                        }

                        Text {
                            id: selectedBadge
                            visible: Boolean(modelData && modelData.isActive)
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: "(Selected)"
                            color: "#8B90A0"
                            font.pixelSize: 10
                        }

                        MouseArea {
                            id: provMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (typeof agentBridge !== "undefined" && modelData && modelData.id) {
                                    agentBridge.setActiveProvider(modelData.id)
                                }
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        id: vbar
                        policy: modelListView.contentHeight > modelListView.height ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
                        contentItem: Rectangle {
                            implicitWidth: 3
                            radius: 2
                            color: "#3F4360"
                        }
                    }
                }
            }
        }

        // ==========================================
        // 3. OBSERVATION SUBMENU VIEW
        // ==========================================
        Item {
            id: obsView
            width: parent.width
            implicitHeight: obsCol.implicitHeight + 16

            x: root.currentView === "observation" ? 0 : width
            opacity: root.currentView === "observation" ? 1.0 : 0.0
            visible: opacity > 0.01

            Behavior on x { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
            Behavior on opacity { NumberAnimation { duration: 200 } }

            Column {
                id: obsCol
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 3

                // Back Button
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 10
                    color: backObsMouse.pressed ? "#2B2E44" : (backObsMouse.containsMouse ? "#24273A" : "transparent")

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Text {
                            text: "<"
                            color: backObsMouse.containsMouse ? "#FFFFFF" : "#8B90A0"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Back"
                            color: backObsMouse.containsMouse ? "#FFFFFF" : "#8B90A0"
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: backObsMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.currentView = "root"
                    }
                }

                // Divider Line
                Rectangle {
                    width: parent.width - 24
                    height: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#25283A"
                }

                // 1. Screen (captureVision)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 8
                    color: itemMouse1.containsMouse ? "#1F2233" : "transparent"
                    Behavior on color { ColorAnimation { duration: 120 } }

                    Text {
                        text: "Screen"
                        color: "#C2CBD5"
                        font.pixelSize: 12
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CustomToggleSwitch {
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        checked: root.flagScreen
                        onToggled: (val) => root.flagScreen = val
                    }

                    MouseArea {
                        id: itemMouse1
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }

                // 2. Full Accessibility
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 8
                    color: itemMouse2.containsMouse ? "#1F2233" : "transparent"
                    Behavior on color { ColorAnimation { duration: 120 } }

                    Text {
                        text: "Full Accessibility"
                        color: "#C2CBD5"
                        font.pixelSize: 12
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CustomToggleSwitch {
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        checked: root.flagFullAccessibility
                        onToggled: (val) => root.flagFullAccessibility = val
                    }

                    MouseArea {
                        id: itemMouse2
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }

                // 3. Active Window Accessibility
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 8
                    color: itemMouse3.containsMouse ? "#1F2233" : "transparent"
                    Behavior on color { ColorAnimation { duration: 120 } }

                    Text {
                        text: "Active Window Accessibility"
                        color: "#C2CBD5"
                        font.pixelSize: 12
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CustomToggleSwitch {
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        checked: root.flagActiveWindowAccessibility
                        onToggled: (val) => root.flagActiveWindowAccessibility = val
                    }

                    MouseArea {
                        id: itemMouse3
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }

                // 4. Clipboard
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 8
                    color: itemMouse4.containsMouse ? "#1F2233" : "transparent"
                    Behavior on color { ColorAnimation { duration: 120 } }

                    Text {
                        text: "Clipboard"
                        color: "#C2CBD5"
                        font.pixelSize: 12
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CustomToggleSwitch {
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        checked: root.flagClipboard
                        onToggled: (val) => root.flagClipboard = val
                    }

                    MouseArea {
                        id: itemMouse4
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }

                // 5. System info (captureDesktop)
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 8
                    color: itemMouse5.containsMouse ? "#1F2233" : "transparent"
                    Behavior on color { ColorAnimation { duration: 120 } }

                    Text {
                        text: "System info"
                        color: "#C2CBD5"
                        font.pixelSize: 12
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CustomToggleSwitch {
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        checked: root.flagSystemInfo
                        onToggled: (val) => root.flagSystemInfo = val
                    }

                    MouseArea {
                        id: itemMouse5
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }

                // 6. New Screen Metrics
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 16
                    height: 34
                    radius: 8
                    color: itemMouse6.containsMouse ? "#1F2233" : "transparent"
                    Behavior on color { ColorAnimation { duration: 120 } }

                    Text {
                        text: "New Screen Metrics"
                        color: "#C2CBD5"
                        font.pixelSize: 12
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CustomToggleSwitch {
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        checked: root.flagNewScreenMetrics
                        onToggled: (val) => root.flagNewScreenMetrics = val
                    }

                    MouseArea {
                        id: itemMouse6
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }
            }
        }
    }
}