#include "../AccessibilityState.h"
#include <sstream>


void AccessibilityState::addNode(SemanticNode node) {
    auto it = idToIndex.find(node.id);
    if (it != idToIndex.end()) {
        nodes[it->second] = std::move(node);
        return;
    }
    idToIndex[node.id] = nodes.size();
    nodes.push_back(std::move(node));
}

std::optional<SemanticNode> AccessibilityState::getNodeById(const NodeId id) const {
    const auto it = idToIndex.find(id);
    if (it != idToIndex.end()) {
        return nodes[it->second];
    }
    return std::nullopt; //not found case
}

const std::vector<SemanticNode>& AccessibilityState::getAllNodes() const {
    return nodes;
}

void AccessibilityState::clearNodes() {
    nodes.clear();
    idToIndex.clear();
}

std::string AccessibilityState::generateJsonPrompt(const AccessibilityState& state) {
    std::ostringstream json;
    json << "[\n";

    const auto& nodes = state.getAllNodes();
    bool isFirst = true;

    for (const auto& node : nodes) {
        //Filtering invisible Nodes
        if (!node.isVisible) continue;

        if (!isFirst) {
            json << ",\n";
        }
        isFirst = false;

        json << "  {\n"
             << "    \"id\": " << node.id << ",\n"
             << "    \"parentId\": " << node.parentId << ",\n"
             << "    \"type\": \"" << getTypeName(node.type) << "\",\n"
             << "    \"name\": \"" << escapeJsonString(node.name) << "\",\n"
             << "    \"value\": \"" << escapeJsonString(node.value) << "\",\n"
             << "    \"isEnabled\": " << (node.isEnabled ? "true" : "false") << ",\n"
             << "    \"isClickable\": " << (node.isClickable ? "true" : "false") << ",\n"
             << "    \"isEditable\": " << (node.isEditable ? "true" : "false") << "\n"
             << "  }";
    }

    json << "\n]";
    return json.str();
}

std::string AccessibilityState::escapeJsonString(const std::string &input) {
    std::ostringstream ss;
    for (char c : input) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else ss << c;
    }
    return ss.str();
}

std::string AccessibilityState::getTypeName(const ElementType type) {
    switch (type) {
        case ElementType::Window: return "Window";
        case ElementType::Button: return "Button";
        case ElementType::TextBox: return "TextBox";
        case ElementType::CheckBox: return "CheckBox";
        case ElementType::Text: return "Text";
        default: return "Unknown";
    }
}
