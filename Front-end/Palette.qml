import QtQuick 2.15

QtObject {
    id: root

    // SideBar
    readonly property color sidebarBg: "#0e1017"
    readonly property color sidebarBorder: "#191b26"
    readonly property color dividerColor: "#222533"

    //Chat Button and Hover
    readonly property color activeChatBg: "#000000"
    readonly property color chatHoverBg: "#181a25"
    readonly property color chatTransparentBg: "transparent"

    //Buttons and Icons
    readonly property color iconHoverCircle: "#252838"
    readonly property color iconNormal: "#c2cbd5"
    readonly property color iconHover: "#ffffff"

    //Texts
    readonly property color textMuted: "#8b90a0"
    readonly property color textActive: "#ffffff"
    readonly property color textHover: "#d0d4e4"

    //Shadow
    readonly property color shadowColor: "#40000000"
    readonly property int baseSidebarWidth: 240
    readonly property int sidebarRadius: 24

    //Glowing TextBox
    readonly property color textBoxGlowColorOne: "#7C3AED"
    readonly property color textBoxGlowColorTwo: "#EC4899"
    readonly property color textBoxGlowColorThree: "#F97316"
}