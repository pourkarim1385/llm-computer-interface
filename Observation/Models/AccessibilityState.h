#ifndef ACCESSIBILITYSERVICE_ACCESSIBILITYSTATE_H
#define ACCESSIBILITYSERVICE_ACCESSIBILITYSTATE_H
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using NodeId = uint64_t;

enum class ElementType {
    // Control Types
    Window, Button, TextBox, CheckBox, RadioButton,
    ComboBox, ListBox, ListItem, TreeItem, TabItem,
    MenuItem, ToolBar, StatusBar, ScrollBar, Slider,
    Hyperlink, Image, Video, Audio,

    // Container Types
    Panel, GroupBox, TabControl, TreeView, ListView,
    Grid, Table, ToolTip, Popup, Menu, MenuBar, ProgressBar, Document,

    // Text Types
    Text, Heading, Label, Paragraph, StaticText,

    // Unknown
    Unknown
};

struct Rect {
    long x, y, width, height;
};

struct SemanticNode {
    //Main Structure & ID
    NodeId id;
    NodeId parentId;
    std::string automationId;
    std::string className;

    //Body & Content
    ElementType type;
    std::string name;
    std::string value;
    std::string description; //Help text / Tooltip

    //Position
    Rect bounds;

    //States
    bool isVisible;
    bool isEnabled;
    bool isFocused;
    bool isPressed; //for button
    bool isChecked; //for cehckBox/radioButton
    bool isExpanded; //for menu
    bool isSelected; //for list
    bool isModal; //for modal
    bool hasPopup; //for menu
    bool isReadOnly; //for fields

    //Patterns
    bool isClickable;
    bool isEditable;
    bool isScrollable;
};

class AccessibilityState {
private:
    std::vector<SemanticNode> nodes;
    std::unordered_map<NodeId, size_t> idToIndex;

    static std::string escapeJsonString(const std::string& input);

    static std::string getTypeName(ElementType type);

public:
    AccessibilityState() =default;
    ~AccessibilityState() =default;
    void addNode(SemanticNode node);
    std::optional<SemanticNode> getNodeById(NodeId id) const;
    const std::vector<SemanticNode>& getAllNodes() const;
    void clearNodes();
    static std::string generateJsonPrompt(const AccessibilityState& state);
};

#endif //ACCESSIBILITYSERVICE_ACCESSIBILITYSTATE_H