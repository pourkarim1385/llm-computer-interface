#include "AccessibilityService.h"
#include <iostream>
#include <algorithm>

#include <stdexcept>

#if defined(__linux__)
    #include <cstring>
    bool AccessibilityService::atspiInitialized_ = false;
#endif

AccessibilityService& AccessibilityService::getInstance() {
    static AccessibilityService instance;
    return instance;
}

AccessibilityService::AccessibilityService() : comInitialized_(false) {
#if defined(_WIN32) || defined(_WIN64)
    HRESULT hResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hResult)) {
        comInitialized_ = true;
    }


    hResult = CoCreateInstance(
        CLSID_CUIAutomation,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IUIAutomation,
        reinterpret_cast<void**>(automation_.GetAddressOf())
    );

    if (FAILED(hResult) || !automation_) {
        throw AccessibilityException("[AccessibilityService] Failed to create IUIAutomation instance");
    }

    hResult = automation_->get_ControlViewWalker(walker_.GetAddressOf());
    if (FAILED(hResult) || !walker_) {
        throw AccessibilityException("[AccessibilityService] Failed to get ControlViewWalker.");
    }
#elif defined(__linux__)
    ensureAtspiInit();
#endif
}

AccessibilityService::~AccessibilityService() {
#if defined(_WIN32) || defined(_WIN64)
    if (walker_) walker_.Reset();
    if (automation_) automation_.Reset();

    if (comInitialized_) {
        CoUninitialize();
        comInitialized_ = false;
    }
#elif defined(__linux__)
    if (atspiInitialized_) {
        atspi_exit();
        atspiInitialized_ = false;
    }
#endif
}

#if defined(_WIN32) || defined(_WIN64)

std::string AccessibilityService::fromBSTR_WIN(BSTR bstr) {
    if (!bstr) return "";
    int length = SysStringLen(bstr);
    if (length == 0) return "";

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, bstr, length, nullptr, 0, nullptr, nullptr);
    std::string str(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, bstr, length, &str[0], sizeNeeded, nullptr, nullptr);
    return str;
}

ElementType AccessibilityService::mapControlType_WIN(const CONTROLTYPEID id) {
    switch (id) {
        // Window & Container Types
        case UIA_WindowControlTypeId:           return ElementType::Window;
        case UIA_PaneControlTypeId:             return ElementType::Panel;
        case UIA_GroupControlTypeId:            return ElementType::GroupBox;
        case UIA_TabControlTypeId:              return ElementType::TabControl;
        case UIA_TreeControlTypeId:             return ElementType::TreeView;
        case UIA_ListControlTypeId:             return ElementType::ListView;
        case UIA_DataGridControlTypeId:         return ElementType::Grid;
        case UIA_TableControlTypeId:            return ElementType::Table;
        case UIA_ToolTipControlTypeId:          return ElementType::ToolTip;
        case UIA_MenuControlTypeId:             return ElementType::Menu;
        case UIA_MenuBarControlTypeId:          return ElementType::MenuBar;
        case UIA_ToolBarControlTypeId:          return ElementType::ToolBar;
        case UIA_StatusBarControlTypeId:        return ElementType::StatusBar;
        case UIA_ScrollBarControlTypeId:        return ElementType::ScrollBar;

        // Interactive Controls
        case UIA_ButtonControlTypeId:           return ElementType::Button;
        case UIA_EditControlTypeId:             return ElementType::TextBox;
        case UIA_CheckBoxControlTypeId:         return ElementType::CheckBox;
        case UIA_RadioButtonControlTypeId:      return ElementType::RadioButton;
        case UIA_ComboBoxControlTypeId:         return ElementType::ComboBox;
        case UIA_ListItemControlTypeId:         return ElementType::ListItem;
        case UIA_TreeItemControlTypeId:         return ElementType::TreeItem;
        case UIA_TabItemControlTypeId:          return ElementType::TabItem;
        case UIA_MenuItemControlTypeId:         return ElementType::MenuItem;
        case UIA_SliderControlTypeId:           return ElementType::Slider;
        case UIA_ProgressBarControlTypeId:      return ElementType::ProgressBar;

        // Text Types
        case UIA_TextControlTypeId:             return ElementType::Text;
        case UIA_DocumentControlTypeId:         return ElementType::Document;

        // Media & Graphics
        case UIA_ImageControlTypeId:            return ElementType::Image;
        case UIA_HyperlinkControlTypeId:        return ElementType::Hyperlink;

        default:                                return ElementType::Unknown;
    }
}
void AccessibilityService::walkTree_WIN(IUIAutomationElement* element,
                                         int parentId,
                                         int depth,
                                         AccessibilityState& state) {
    if (!element || depth > kMaxDepth) return;

    int currentId = nextId_++;
    SemanticNode node;
    node.id = currentId;
    node.parentId = parentId;

    //Name & ID
    ScopedBSTR nameBstr;
    if (SUCCEEDED(element->get_CurrentName(nameBstr.receive()))) {
        node.name = fromBSTR_WIN(nameBstr.get());
    }

    ScopedBSTR autoIdBstr;
    if (SUCCEEDED(element->get_CurrentAutomationId(autoIdBstr.receive()))) {
        node.automationId = fromBSTR_WIN(autoIdBstr.get());
    }

    ScopedBSTR classNameBstr;
    if (SUCCEEDED(element->get_CurrentClassName(classNameBstr.receive()))) {
        node.className = fromBSTR_WIN(classNameBstr.get());
    }

    BOOL isOffscreen = FALSE;
    if (SUCCEEDED(element->get_CurrentIsOffscreen(&isOffscreen))) {
        node.isVisible = !isOffscreen;
    }

    BOOL isEnabled = FALSE;
    if (SUCCEEDED(element->get_CurrentIsEnabled(&isEnabled))) {
        node.isEnabled = isEnabled;
    }

    BOOL hasFocus = FALSE;
    if (SUCCEEDED(element->get_CurrentHasKeyboardFocus(&hasFocus))) {
        node.isFocused = hasFocus;
    }

    BOOL isKeyboardFocusable = FALSE;
    if (SUCCEEDED(element->get_CurrentIsKeyboardFocusable(&isKeyboardFocusable))) {
        node.isClickable = isKeyboardFocusable;
    }

    RECT rect;
    if (SUCCEEDED(element->get_CurrentBoundingRectangle(&rect))) {
        node.bounds.x = rect.left;
        node.bounds.y = rect.top;
        node.bounds.width = rect.right - rect.left;
        node.bounds.height = rect.bottom - rect.top;
    }

    CONTROLTYPEID typeId = 0;
    if (SUCCEEDED(element->get_CurrentControlType(&typeId))) {
        node.type = mapControlType_WIN(typeId);
    }

    VARIANT val;
    VariantInit(&val);
    if (SUCCEEDED(element->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &val))) {
        if (val.vt == VT_BSTR && val.bstrVal != nullptr) {
            node.value = fromBSTR_WIN(val.bstrVal);
        }
        VariantClear(&val);
    }

    if (SUCCEEDED(element->GetCurrentPropertyValue(UIA_ToggleToggleStatePropertyId, &val))) {
        if (val.vt == VT_I4) {
            node.isPressed = (val.lVal == 1);
        }
        VariantClear(&val);
    }

    if (SUCCEEDED(element->GetCurrentPropertyValue(UIA_SelectionItemIsSelectedPropertyId, &val))) {
        if (val.vt == VT_BOOL) {
            node.isSelected = (val.boolVal == VARIANT_TRUE);
        }
        VariantClear(&val);
    }

    if (SUCCEEDED(element->GetCurrentPropertyValue(UIA_ExpandCollapseExpandCollapseStatePropertyId, &val))) {
        if (val.vt == VT_I4) {
            node.isExpanded = (val.lVal == 1);
        }
        VariantClear(&val);
    }

    state.addNode(std::move(node));

    Microsoft::WRL::ComPtr<IUIAutomationElement> child;
    HRESULT hr = walker_->GetFirstChildElement(element, child.GetAddressOf());
    while (SUCCEEDED(hr) && child) {
        walkTree_WIN(child.Get(), currentId, depth + 1, state);

        Microsoft::WRL::ComPtr<IUIAutomationElement> next;
        hr = walker_->GetNextSiblingElement(child.Get(), next.GetAddressOf());
        child = next;
    }
}

AccessibilityState AccessibilityService::captureFullState_WIN() {
    AccessibilityState state;
    if (!automation_ || !walker_) return state;

    nextId_ = 1;
    Microsoft::WRL::ComPtr<IUIAutomationElement> root;
    HRESULT hresult = automation_->GetRootElement(root.GetAddressOf());
    if (FAILED(hresult) || root == nullptr)
        throw AccessibilityException("GetRootElement failed");

    if (SUCCEEDED(hresult) && root)
        walkTree_WIN(root.Get(), 0, 0, state);

    return state;
}

AccessibilityState AccessibilityService::captureForegroundWindowState_WIN() {
    AccessibilityState state;
    if (!automation_ || !walker_) return state;

    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return state;

    nextId_ = 1;
    Microsoft::WRL::ComPtr<IUIAutomationElement> targetElement;
    const HRESULT hresult = automation_->ElementFromHandle(reinterpret_cast<UIA_HWND>(hwnd), targetElement.GetAddressOf());
    if (FAILED(hresult) || targetElement == nullptr)
        throw AccessibilityException("ElementFromHandle failed");

    if (SUCCEEDED(hresult) && targetElement)
        walkTree_WIN(targetElement.Get(), 0, 0, state);

    return state;
}

AccessibilityState AccessibilityService::captureTargetWindowState_WIN(const std::string& processNameOrTitle) {
    AccessibilityState state;
    if (!automation_ || !walker_) return state;

    nextId_ = 1;
    Microsoft::WRL::ComPtr<IUIAutomationElement> root;
    HRESULT hr = automation_->GetRootElement(root.GetAddressOf());
    if (FAILED(hr) || !root) return state;

    Microsoft::WRL::ComPtr<IUIAutomationElement> child;
    hr = walker_->GetFirstChildElement(root.Get(), child.GetAddressOf());

    std::string lowerTarget = processNameOrTitle;
    std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

    while (SUCCEEDED(hr) && child) {
        ScopedBSTR nameBstr;
        if (SUCCEEDED(child->get_CurrentName(nameBstr.receive()))) {
            std::string name = fromBSTR_WIN(nameBstr.get());
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

            if (!lowerName.empty() && lowerName.find(lowerTarget) != std::string::npos) {
                walkTree_WIN(child.Get(), 0, 0, state);
                return state;
            }
        }

        Microsoft::WRL::ComPtr<IUIAutomationElement> next;
        hr = walker_->GetNextSiblingElement(child.Get(), next.GetAddressOf());
        child = next;
    }

    return state;
}

#endif // Windows

#if defined(__linux__)

void AccessibilityService::ensureAtspiInit() {
    if (!atspiInitialized_) {
        int r = atspi_init();
        if (r == 0 || r == 1) { // 1 = already initialized
            atspiInitialized_ = true;
        } else {
            throw AccessibilityException("[AccessibilityService] Failed to initialize AT-SPI2.");
        }
    }
}

ElementType AccessibilityService::mapAtSpiRole_LINUX(AtspiRole role) {
    switch (role) {
        case ATSPI_ROLE_PUSH_BUTTON:      return ElementType::Button;
        case ATSPI_ROLE_CHECK_BOX:        return ElementType::CheckBox;
        case ATSPI_ROLE_COMBO_BOX:        return ElementType::ComboBox;
        case ATSPI_ROLE_ENTRY:
        case ATSPI_ROLE_PASSWORD_TEXT:
        case ATSPI_ROLE_TEXT:             return ElementType::TextField;
        case ATSPI_ROLE_LIST:             return ElementType::List;
        case ATSPI_ROLE_LIST_ITEM:        return ElementType::ListItem;
        case ATSPI_ROLE_MENU:             return ElementType::Menu;
        case ATSPI_ROLE_MENU_BAR:         return ElementType::MenuBar;
        case ATSPI_ROLE_MENU_ITEM:
        case ATSPI_ROLE_CHECK_MENU_ITEM:
        case ATSPI_ROLE_RADIO_MENU_ITEM: return ElementType::MenuItem;
        case ATSPI_ROLE_PROGRESS_BAR:     return ElementType::ProgressBar;
        case ATSPI_ROLE_RADIO_BUTTON:     return ElementType::RadioButton;
        case ATSPI_ROLE_SCROLL_BAR:       return ElementType::ScrollBar;
        case ATSPI_ROLE_SLIDER:           return ElementType::Slider;
        case ATSPI_ROLE_PAGE_TAB:         return ElementType::TabItem;
        case ATSPI_ROLE_PAGE_TAB_LIST:    return ElementType::Tab;
        case ATSPI_ROLE_LABEL:            return ElementType::Label;
        case ATSPI_ROLE_TOOL_BAR:         return ElementType::ToolBar;
        case ATSPI_ROLE_TOOL_TIP:         return ElementType::ToolTip;
        case ATSPI_ROLE_TREE:             return ElementType::Tree;
        case ATSPI_ROLE_TREE_ITEM:        return ElementType::TreeItem;
        case ATSPI_ROLE_FRAME:
        case ATSPI_ROLE_WINDOW:
        case ATSPI_ROLE_DIALOG:           return ElementType::Window;
        case ATSPI_ROLE_PANEL:
        case ATSPI_ROLE_FILLER:           return ElementType::Pane;
        case ATSPI_ROLE_ICON:
        case ATSPI_ROLE_IMAGE:            return ElementType::Image;
        case ATSPI_ROLE_SEPARATOR:        return ElementType::Separator;
        case ATSPI_ROLE_TABLE:            return ElementType::Table;
        default:                          return ElementType::Unknown;
    }
}

void AccessibilityService::readStates(AtspiAccessible* acc, DynamicSemanticNode& node) {
    smartGObject<AtspiStateSet> stateSet(atspi_accessible_get_state_set(acc));
    if (!stateSet) return;

    node.isVisible = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_VISIBLE) &&
                     !atspi_state_set_contains(stateSet.get(), ATSPI_STATE_DEFUNCT);
    node.isEnabled  = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_ENABLED);
    node.isFocused  = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_FOCUSED);
    node.isPressed  = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_PRESSED);
    node.isChecked  = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_CHECKED);
    node.isExpanded = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_EXPANDED);
    node.isSelected = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_SELECTED);
    node.isModal    = atspi_state_set_contains(stateSet.get(), ATSPI_STATE_MODAL);
    node.isReadOnly = !atspi_state_set_contains(stateSet.get(), ATSPI_STATE_EDITABLE);
}

void AccessibilityService::readValue(AtspiAccessible* acc, DynamicSemanticNode& node) {
    GError* error = nullptr;
    smartGObject<AtspiValue> valIf(atspi_accessible_get_value(acc));
    if (valIf) {
        gdouble currentVal = atspi_value_get_current_value(valIf.get(), &error);
        if (!error) {
            node.value = std::to_string(currentVal);
        } else {
            g_clear_error(&error);
        }
    }
}

void AccessibilityService::readAttributes(AtspiAccessible* acc, DynamicSemanticNode& node) {
    GError* error = nullptr;
    GHashTable* attribs = atspi_accessible_get_attributes(acc, &error);
    if (error) {
        g_clear_error(&error);
        return;
    }
    if (!attribs) return;

    gpointer key, value;
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, attribs);

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        if (!key || !value) continue;
        const char* k = static_cast<const char*>(key);
        const char* v = static_cast<const char*>(value);

        if (std::strcmp(k, "id") == 0) {
            node.automationId = v;
        } else if (std::strcmp(k, "class") == 0) {
            node.className = v;
        }
    }
    g_hash_table_unref(attribs);
}

void AccessibilityService::readClickable(AtspiAccessible* acc, DynamicSemanticNode& node) {
    smartGObject<AtspiAction> actionIf(atspi_accessible_get_action(acc));
    if (actionIf) {
        GError* error = nullptr;
        gint count = atspi_action_get_n_actions(actionIf.get(), &error);
        if (!error && count > 0) {
            node.isClickable = true;
        } else if (error) {
            g_clear_error(&error);
        }
    }
}

void AccessibilityService::walkTree_LINUX(AtspiAccessible* element,
                                          int parentId,
                                          int depth,
                                          AccessibilityState& state) {
    if (!element || depth >= kMaxDepth) return;

    GError* error = nullptr;

    AtspiRole role = atspi_accessible_get_role(element, &error);
    if (error) {
        g_clear_error(&error);
        return;
    }

    int currentId = nextId_++;

    DynamicSemanticNode node;
    node.id       = currentId;
    node.parentId = parentId;
    node.type     = mapAtSpiRole_LINUX(role);

    smartGchar rawName(atspi_accessible_get_name(element, &error));
    if (error) g_clear_error(&error);
    else node.name = rawName.str();

    smartGchar rawDesc(atspi_accessible_get_description(element, &error));
    if (error) g_clear_error(&error);
    else node.description = rawDesc.str();

    smartGchar rawRoleName(atspi_accessible_get_role_name(element, &error));
    if (error) g_clear_error(&error);

    readStates(element, node);
    readValue(element, node);
    readAttributes(element, node);
    readClickable(element, node);

    if (node.className.empty() && rawRoleName.p) {
        node.className = rawRoleName.str();
    }

    smartGObject<AtspiComponent> compIf(atspi_accessible_get_component(element));
    if (compIf) {
        smartGObject<AtspiRect> rect(atspi_component_get_extents(compIf.get(), ATSPI_COORD_TYPE_SCREEN, &error));
        if (!error && rect) {
            node.bounds.x      = rect.get()->x;
            node.bounds.y      = rect.get()->y;
            node.bounds.width  = rect.get()->width;
            node.bounds.height = rect.get()->height;
        } else if (error) {
            g_clear_error(&error);
        } else if (rect) {
            g_free(rect);
        }
    }

    state.addNode(std::move(node));

    gint childCount = atspi_accessible_get_child_count(element, &error);
    if (error) {
        g_clear_error(&error);
        return;
    }

    for (gint i = 0; i < childCount; ++i) {
        smartGObject<AtspiAccessible> child(atspi_accessible_get_child_at_index(element, i, &error));
        if (error) {
            g_clear_error(&error);
            continue;
        }
        if (child) {
            walkTree_LINUX(child.get(), currentId, depth + 1, state);
        }
    }
}

AccessibilityState AccessibilityService::captureFullState_LINUX() {
    AccessibilityState state;
    ensureAtspiInit();
    if (!atspiInitialized_) return state;

    nextId_ = 1;
    gint desktopCount = atspi_get_desktop_count();
    for (gint i = 0; i < desktopCount; ++i) {
        smartGObject<AtspiAccessible> desktop(atspi_get_desktop(i));
        if (desktop) {
            walkTree_LINUX(desktop.get(), 0, 0, state);
        }
    }
    return state;
}

AccessibilityState AccessibilityService::captureForegroundWindowState_LINUX() {
    AccessibilityState state;
    ensureAtspiInit();
    if (!atspiInitialized_) return state;

    nextId_ = 1;
    gint desktopCount = atspi_get_desktop_count();
    for (gint d = 0; d < desktopCount; ++d) {
        smartGObject<AtspiAccessible> desktop(atspi_get_desktop(d));
        if (!desktop) continue;

        GError* error = nullptr;
        gint appCount = atspi_accessible_get_child_count(desktop.get(), &error);
        if (error) { g_clear_error(&error); continue; }

        for (gint a = 0; a < appCount; ++a) {
            smartGObject<AtspiAccessible> app(atspi_accessible_get_child_at_index(desktop.get(), a, &error));
            if (error || !app) { g_clear_error(&error); continue; }

            smartGObject<AtspiStateSet> stateSet(atspi_accessible_get_state_set(app.get()));
            if (stateSet && atspi_state_set_contains(stateSet.get(), ATSPI_STATE_ACTIVE)) {
                walkTree_LINUX(app.get(), 0, 0, state);
                return state;
            }
        }
    }
    return state;
}

AccessibilityState AccessibilityService::captureTargetWindowState_LINUX(const std::string& processNameOrTitle) {
    AccessibilityState state;
    ensureAtspiInit();
    if (!atspiInitialized_) return state;

    nextId_ = 1;
    std::string lowerTarget = processNameOrTitle;
    std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

    gint desktopCount = atspi_get_desktop_count();
    for (gint d = 0; d < desktopCount; ++d) {
        smartGObject<AtspiAccessible> desktop(atspi_get_desktop(d));
        if (!desktop) continue;

        GError* error = nullptr;
        gint appCount = atspi_accessible_get_child_count(desktop.get(), &error);
        if (error) { g_clear_error(&error); continue; }

        for (gint a = 0; a < appCount; ++a) {
            smartGObject<AtspiAccessible> app(atspi_accessible_get_child_at_index(desktop.get(), a, &error));
            if (error || !app) { g_clear_error(&error); continue; }

            smartGchar rawName(atspi_accessible_get_name(app.get(), &error));
            if (error) { g_clear_error(&error); continue; }

            std::string appName = rawName.str();
            std::transform(appName.begin(), appName.end(), appName.begin(), ::tolower);

            if (!appName.empty() && appName.find(lowerTarget) != std::string::npos) {
                walkTree_LINUX(app.get(), 0, 0, state);
                return state;
            }
        }
    }
    return state;
}

#endif // Linux

AccessibilityState AccessibilityService::captureFullState() {
#if defined(_WIN32) || defined(_WIN64)
    return captureFullState_WIN();
#elif defined(__linux__)
    return captureFullState_LINUX();
#else
    return AccessibilityState{};
#endif
}

AccessibilityState AccessibilityService::captureForegroundWindowState() {
#if defined(_WIN32) || defined(_WIN64)
    return captureForegroundWindowState_WIN();
#elif defined(__linux__)
    return captureForegroundWindowState_LINUX();
#else
    return AccessibilityState{};
#endif
}

AccessibilityState AccessibilityService::captureTargetWindowState(const std::string& processNameOrTitle) {
#if defined(_WIN32) || defined(_WIN64)
    return captureTargetWindowState_WIN(processNameOrTitle);
#elif defined(__linux__)
    return captureTargetWindowState_LINUX(processNameOrTitle);
#else
    return AccessibilityState{};
#endif
}