import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15
import "../.."
import "."

Item {
    id: root

    Palette { id: palette }

    signal backRequested()

    property bool isFormNewMode: false
    property string selectedProviderId: ""

    Component.onCompleted: {
        selectInitialProvider();
    }

    Connections {
        target: settingsController
        function onProvidersChanged() {
            refreshProviderList();
        }
    }

    function refreshProviderList() {
        var list = settingsController.providers;
        provList.model = list;
        if (!root.isFormNewMode) {
            if (root.selectedProviderId !== "") {
                selectProvider(root.selectedProviderId);
            } else if (list && list.length > 0) {
                selectProvider(list[0].id);
            }
        }
    }

    function selectInitialProvider() {
        if (!isFormNewMode) {
            var provs = settingsController.providers;
            if (provs && provs.length > 0) {
                var targetId = settingsController.activeProviderId !== "" ? settingsController.activeProviderId : provs[0].id;
                selectProvider(targetId);
            }
        }
    }

    function selectProvider(pId) {
        isFormNewMode = false;
        selectedProviderId = pId;
        var p = settingsController.getProviderDetails(pId);
        nameInput.text = p.name || "";
        baseUrlInput.text = p.baseUrl || "";
        apiKeyInput.text = p.apiKeyMasked || "";
        formatCombo.currentIndex = p.format !== undefined ? p.format : 0;
        modelInput.text = p.modelId || "";
    }

    function startNewProvider() {
        isFormNewMode = true;
        selectedProviderId = "";
        nameInput.text = "";
        baseUrlInput.text = "";
        apiKeyInput.text = "";
        formatCombo.currentIndex = 0;
        modelInput.text = "";
    }

    readonly property bool isFormComplete: nameInput.text.trim().length > 0 &&
        baseUrlInput.text.trim().length > 0 &&
        apiKeyInput.text.trim().length > 0 &&
        modelInput.text.trim().length > 0

    Flickable {
        id: scrollArea
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: Math.max(880, rightCol.y + rightCol.implicitHeight + 60)
        clip: true

        Item {
            width: Math.max(940, root.width)
            height: Math.max(880, rightCol.y + rightCol.implicitHeight + 60)

            // --- Back Button ---
            Item {
                id: backBtn
                anchors.top: parent.top
                anchors.topMargin: 20
                anchors.left: parent.left
                anchors.leftMargin: 28
                width: backRow.implicitWidth + 20
                height: 32

                Row {
                    id: backRow
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    spacing: 6

                    Text {
                        text: "< Back to Assistant Page"
                        color: backMouse.containsMouse ? "#FFFFFF" : "#8B90A0"
                        font.pixelSize: 13
                        font.bold: true
                        Behavior on color { ColorAnimation { duration: 140 } }
                    }
                }

                MouseArea {
                    id: backMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.backRequested()
                }
            }

            // ==========================================
            // LEFT COLUMN: General Section
            // ==========================================
            Column {
                id: leftCol
                anchors.top: backBtn.bottom
                anchors.topMargin: 20
                anchors.left: parent.left
                anchors.leftMargin: 28
                width: 320
                spacing: 14

                Text {
                    text: "General"
                    color: "#FFFFFF"
                    font.pixelSize: 24
                    font.bold: true
                }

                // Alerts & Notifications
                Rectangle {
                    width: parent.width
                    height: 120
                    radius: 12
                    color: "#0E1017"
                    border.color: "#191B26"
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8

                        Text {
                            text: "Alerts & notifications"
                            color: "#FFFFFF"
                            font.pixelSize: 13; font.bold: true
                        }
                        Text {
                            text: "Keep track of server logs, user activities, and background system checks."
                            color: "#6B7280"
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }

                        Item { width: 1; height: 2 }

                        Row {
                            width: parent.width
                            Item {
                                width: parent.width - toggleSwitch.width
                                height: 30
                                Column {
                                    spacing: 2
                                    Text {
                                        text: "Notifications"
                                        color: "#E2E8F0"
                                        font.pixelSize: 12; font.bold: true
                                    }
                                    Text {
                                        text: "Receive push notifications for updates and alerts"
                                        color: "#6B7280"
                                        font.pixelSize: 10
                                    }
                                }
                            }

                            Rectangle {
                                id: toggleSwitch
                                width: 44; height: 24; radius: 12
                                color: settingsController.sendNotif ? "#7C3AED" : "#26293A"
                                Behavior on color { ColorAnimation { duration: 150 } }

                                Rectangle {
                                    width: 18; height: 18; radius: 9
                                    anchors.verticalCenter: parent.verticalCenter
                                    x: settingsController.sendNotif ? (parent.width - width - 3) : 3
                                    color: "#FFFFFF"
                                    Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsController.sendNotif = !settingsController.sendNotif
                                }
                            }
                        }
                    }
                }

                // Storage
                Rectangle {
                    width: parent.width
                    height: 80
                    radius: 12
                    color: "#0E1017"
                    border.color: "#191B26"
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8

                        Text {
                            text: "Storage"
                            color: "#FFFFFF"
                            font.pixelSize: 13; font.bold: true
                        }

                        Rectangle {
                            width: 80; height: 28; radius: 6
                            color: clearMouse.containsMouse ? "#3B181C" : "#241316"
                            border.color: clearMouse.containsMouse ? "#EF4444" : "#451A20"
                            border.width: 1
                            Behavior on color { ColorAnimation { duration: 120 } }

                            Row {
                                anchors.centerIn: parent
                                spacing: 6
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: "🗑"
                                    font.pixelSize: 11
                                    color: "#EF4444"
                                }
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: "Clear"
                                    color: "#EF4444"
                                    font.pixelSize: 11; font.bold: true
                                }
                            }

                            MouseArea {
                                id: clearMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: confirmPopup.open()
                            }
                        }
                    }
                }

                // About Us
                Rectangle {
                    width: parent.width
                    height: 88
                    radius: 12
                    color: "#0E1017"
                    border.color: "#191B26"
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text {
                            text: "About us"
                            color: "#FFFFFF"
                            font.pixelSize: 13; font.bold: true
                        }

                        Rectangle {
                            width: 130; height: 30; radius: 6
                            color: aboutMouse.containsMouse ? "#6366F1" : "#4F46E5"
                            Behavior on color { ColorAnimation { duration: 120 } }

                            Text {
                                anchors.centerIn: parent
                                text: "Open Repo Page"
                                color: "#FFFFFF"
                                font.pixelSize: 11; font.bold: true
                            }
                            MouseArea {
                                id: aboutMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: settingsController.openAboutUrl()
                            }
                        }
                    }
                }
            }

            // ==========================================
            // RIGHT COLUMN: LLM & WebSearch Providers
            // ==========================================
            Column {
                id: rightCol
                anchors.top: backBtn.bottom
                anchors.topMargin: 20
                anchors.left: leftCol.right
                anchors.leftMargin: 32
                anchors.right: parent.right
                anchors.rightMargin: 28
                spacing: 16

                Text {
                    text: "LLM Provider"
                    color: "#FFFFFF"
                    font.pixelSize: 24
                    font.bold: true
                }

                Rectangle {
                    id: llmCard
                    width: parent.width
                    height: 380
                    radius: 12
                    color: "#0E1017"
                    border.color: "#191B26"
                    border.width: 1

                    // Left sub-panel: Providers list
                    Item {
                        id: listPanel
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.margins: 16
                        width: 150

                        Text {
                            id: provLabel
                            text: "PROVIDERS"
                            color: "#6B7280"
                            font.pixelSize: 10; font.bold: true
                        }

                        ListView {
                            id: provList
                            anchors.top: provLabel.bottom
                            anchors.topMargin: 8
                            anchors.bottom: addProvBtn.top
                            anchors.bottomMargin: 8
                            width: parent.width
                            spacing: 6
                            clip: true
                            model: settingsController.providers

                            delegate: Rectangle {
                                width: provList.width
                                height: 32
                                radius: 6
                                readonly property bool isSelected: (!root.isFormNewMode && root.selectedProviderId === modelData.id)
                                color: isSelected ? "#1F2333" : (rowMouse.containsMouse ? "#181B26" : "#13151F")
                                border.color: isSelected ? "#7C3AED" : (rowMouse.containsMouse ? "#2B3044" : "#1D202E")
                                border.width: 1

                                Behavior on color { ColorAnimation { duration: 120 } }
                                Behavior on border.color { ColorAnimation { duration: 120 } }

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8; anchors.rightMargin: 8
                                    spacing: 4

                                    Text {
                                        width: parent.width - (modelData.isActive ? 16 : 0)
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: modelData.name
                                        font.pixelSize: 11
                                        font.bold: isSelected || modelData.isActive
                                        color: isSelected ? "#FFFFFF" : (rowMouse.containsMouse ? "#E2E8F0" : "#9CA3AF")
                                        elide: Text.ElideRight
                                    }

                                    Rectangle {
                                        visible: modelData.isActive
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 8; height: 8; radius: 4
                                        color: "#A855F7"
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 14; height: 14; radius: 7
                                            color: "#30A855F7"
                                            z: -1
                                        }
                                    }
                                }

                                MouseArea {
                                    id: rowMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.selectProvider(modelData.id)
                                }
                            }
                        }

                        Rectangle {
                            id: addProvBtn
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 32
                            radius: 6
                            color: addMouse.containsMouse ? "#1A1433" : "#12101F"
                            border.color: addMouse.containsMouse ? "#A855F7" : "#7C3AED"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "Add Provider"
                                color: addMouse.containsMouse ? "#C084FC" : "#A855F7"
                                font.pixelSize: 11; font.bold: true
                            }

                            MouseArea {
                                id: addMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.startNewProvider()
                            }
                        }
                    }

                    Rectangle {
                        id: cardDivider
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: listPanel.right
                        anchors.leftMargin: 12
                        anchors.topMargin: 16
                        anchors.bottomMargin: 16
                        width: 1
                        color: "#191B26"
                    }

                    Item {
                        id: formPanel
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: cardDivider.right
                        anchors.right: parent.right
                        anchors.margins: 16

                        Flickable {
                            anchors.top: parent.top
                            anchors.bottom: formActionsRow.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottomMargin: 8
                            contentWidth: width
                            contentHeight: formColumn.implicitHeight
                            clip: true

                            Column {
                                id: formColumn
                                width: parent.width
                                spacing: 4

                                SettingsInputField {
                                    id: nameInput
                                    labelText: "Name"
                                    placeholderText: "e.g. DeepSeek"
                                    readOnly: !root.isFormNewMode
                                }

                                SettingsInputField {
                                    id: baseUrlInput
                                    labelText: "Base URL"
                                    placeholderText: "https://api.example.com/v1"
                                    readOnly: !root.isFormNewMode
                                }

                                SettingsInputField {
                                    id: apiKeyInput
                                    labelText: "API key"
                                    placeholderText: "Enter API key"
                                    readOnly: !root.isFormNewMode
                                }

                                SettingsComboBox {
                                    id: formatCombo
                                    labelText: "API format"
                                    readOnly: !root.isFormNewMode
                                }

                                SettingsInputField {
                                    id: modelInput
                                    labelText: "Model"
                                    placeholderText: "Model name"
                                    readOnly: !root.isFormNewMode
                                }
                            }
                        }

                        Row {
                            id: formActionsRow
                            anchors.bottom: parent.bottom
                            anchors.right: parent.right
                            spacing: 10

                            Rectangle {
                                id: removeProvBtn
                                width: 116; height: 32; radius: 6
                                readonly property bool canRemove: (!root.isFormNewMode && root.selectedProviderId !== "" && root.selectedProviderId !== "dummy_provider")
                                enabled: canRemove
                                opacity: canRemove ? 1.0 : 0.4
                                color: removeProvMouse.containsMouse ? "#2B1E22" : "#1B171A"
                                border.color: removeProvMouse.containsMouse ? "#EF4444" : "#382329"
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "Remove provider"
                                    color: removeProvBtn.canRemove ? (removeProvMouse.containsMouse ? "#EF4444" : "#D1D5DB") : "#6B7280"
                                    font.pixelSize: 11; font.bold: true
                                }
                                MouseArea {
                                    id: removeProvMouse
                                    anchors.fill: parent
                                    hoverEnabled: parent.enabled
                                    cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (removeProvBtn.canRemove) {
                                            var ok = settingsController.removeProvider(root.selectedProviderId);
                                            if (ok) {
                                                root.refreshProviderList();
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                id: addProvActionBtn
                                width: 100; height: 32; radius: 6
                                readonly property bool canAdd: root.isFormNewMode && root.isFormComplete
                                enabled: canAdd
                                opacity: canAdd ? 1.0 : 0.4
                                color: canAdd ? (addProvActionMouse.containsMouse ? "#6D28D9" : "#7C3AED") : "#281F38"

                                Text {
                                    anchors.centerIn: parent
                                    text: "Add provider"
                                    color: addProvActionBtn.canAdd ? "#FFFFFF" : "#6B7280"
                                    font.pixelSize: 11; font.bold: true
                                }
                                MouseArea {
                                    id: addProvActionMouse
                                    anchors.fill: parent
                                    hoverEnabled: parent.enabled
                                    cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        if (addProvActionBtn.canAdd) {
                                            var ok = settingsController.addProvider(
                                                nameInput.text,
                                                baseUrlInput.text,
                                                apiKeyInput.text,
                                                formatCombo.currentIndex,
                                                modelInput.text
                                            );
                                            if (ok) {
                                                root.isFormNewMode = false;
                                                var list = settingsController.providers;
                                                provList.model = list;
                                                if (list && list.length > 0) {
                                                    root.selectProvider(list[list.length - 1].id);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Item { width: 1; height: 4 }

                Text {
                    text: "WebSearch Provider"
                    color: "#FFFFFF"
                    font.pixelSize: 24
                    font.bold: true
                }

                Rectangle {
                    id: webSearchCard
                    width: parent.width
                    height: 154
                    radius: 12
                    color: "#0E1017"
                    border.color: "#191B26"
                    border.width: 1

                    Row {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 20

                        Item {
                            width: (parent.width - 20) * 0.62
                            height: parent.height

                            Column {
                                anchors.fill: parent
                                spacing: 4

                                SettingsInputField {
                                    id: tavilyKeyInput
                                    labelText: "Tavily Provider API key"
                                    text: settingsController.tavilyApiKeyMasked
                                    placeholderText: "Enter API key"
                                    readOnly: false
                                    onTextChanged: {
                                        if (text.indexOf("****") === -1) {
                                            settingsController.tavilyApiKey = text;
                                        }
                                    }
                                }

                                SettingsInputField {
                                    id: tavilyLimitInput
                                    labelText: "Tavily Provider credit limit"
                                    text: settingsController.tavilyCreditLimit.toString()
                                    placeholderText: "Enter limit"
                                    onTextChanged: {
                                        var val = parseInt(text);
                                        if (!isNaN(val)) settingsController.tavilyCreditLimit = val;
                                    }
                                }
                            }
                        }

                        Item {
                            width: (parent.width - 20) * 0.38
                            height: parent.height

                            Column {
                                anchors.fill: parent
                                spacing: 12

                                Text {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: "We are sorry but this project supports only Tavily for main web search provider, and DuckDuckGo as fallback provider if Tavily API is not set or credit limit reached."
                                    color: "#6B7280"
                                    font.pixelSize: 10
                                    lineHeight: 1.3
                                }

                                Rectangle {
                                    width: 100; height: 30; radius: 6
                                    color: resetUsageMouse.containsMouse ? "#1E2230" : "#131622"
                                    border.color: resetUsageMouse.containsMouse ? "#3B425A" : "#202436"
                                    border.width: 1
                                    Behavior on color { ColorAnimation { duration: 120 } }

                                    Text {
                                        anchors.centerIn: parent
                                        text: "Reset Usage"
                                        color: resetUsageMouse.containsMouse ? "#FFFFFF" : "#C2CBD5"
                                        font.pixelSize: 11; font.bold: true
                                    }

                                    MouseArea {
                                        id: resetUsageMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: settingsController.resetUsage()
                                    }
                                }
                            }
                        }
                    }
                }

                Item { width: 1; height: 4 }

                // Actions Footer Row
                Item {
                    width: parent.width
                    height: 38

                    Row {
                        anchors.right: parent.right
                        spacing: 12

                        Rectangle {
                            width: 130; height: 36; radius: 8
                            color: resetDefMouse.containsMouse ? "#1E2230" : "#12141D"
                            border.color: resetDefMouse.containsMouse ? "#373E54" : "#1D202D"
                            border.width: 1
                            Behavior on color { ColorAnimation { duration: 120 } }

                            Text {
                                anchors.centerIn: parent
                                text: "Reset to Defaults"
                                color: resetDefMouse.containsMouse ? "#FFFFFF" : "#C2CBD5"
                                font.pixelSize: 12; font.bold: true
                            }
                            MouseArea {
                                id: resetDefMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: settingsController.resetToDefaults()
                            }
                        }

                        Rectangle {
                            width: 90; height: 36; radius: 8
                            color: applyMouse.containsMouse ? "#6366F1" : "#4F46E5"
                            Behavior on color { ColorAnimation { duration: 120 } }

                            Text {
                                anchors.centerIn: parent
                                text: "Apply"
                                color: "#FFFFFF"
                                font.pixelSize: 12; font.bold: true
                            }
                            MouseArea {
                                id: applyMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: settingsController.saveSettings()
                            }
                        }
                    }
                }
            }
        }
    }

    ClearConfirmPopup {
        id: confirmPopup
        z: 999
        onConfirmed: settingsController.clearStorage()
    }
}