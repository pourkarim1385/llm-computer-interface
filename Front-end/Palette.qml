import QtQuick

QtObject {
    id: root


    // Backgrounds & Surfaces
    readonly property color windowBg: "#041b36"      
    readonly property color surfaceBg: "#001d3d"      
    readonly property color surfaceHover: "#002b5c"
    readonly property color surfaceActive: "#003566"

    // Borders & Dividers
    readonly property color borderSubtle: "#002b5c"
    readonly property color borderDefault: "#003566"
    readonly property color borderActive: "#38bdf8"

   // Sidebar Specifics

    readonly property color sidebarBg: "#00162e"
    readonly property color sidebarBorder: "#002b5c"
    readonly property color dividerColor: "#002b5c"

    // Chat List & Navigation States
    readonly property color activeChatBg: "#003566"
    readonly property color chatHoverBg: "#002b5c"
    readonly property color chatTransparentBg: "transparent"


    // Buttons & Icons
    readonly property color iconHoverCircle: "#003566"
    readonly property color iconNormal: "#94a3b8"
    readonly property color iconHover: "#FFFFFF"

    // Typography Hierarchy
    readonly property color textMuted: "#94a3b8"
    readonly property color textActive: "#FFFFFF"
    readonly property color textHover: "#f1f5f9"
    readonly property color textSubtle: "#64748b"

    // Accents & Send Button Color
    readonly property color sendButtonColor: "#003566" 
    readonly property color textBoxGlowColorOne: "#001d3d"
    readonly property color textBoxGlowColorTwo: "#003566"
    readonly property color textBoxGlowColorThree: "#38bdf8"
    
    readonly property color successColor: "#10B981"
    readonly property color errorColor: "#ef4444"

    // Layout Constants
    readonly property int baseSidebarWidth: 240
    readonly property int sidebarRadius: 20
}