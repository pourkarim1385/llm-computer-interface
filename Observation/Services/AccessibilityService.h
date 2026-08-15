#pragma once

#include <memory>
#include <string>

#include <stdexcept>

// Dynamic Structure Models
#include "../Models/AccessibilityState.h"

// Platform-Specific Headers
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <uiautomation.h>
    #include <wrl/client.h>
#elif defined(__linux__)
    #include <atspi/atspi.h>
    #include <glib.h>
#endif

#if defined(__linux__)
// RAII helper for gchar* allocated by GLib / AT-SPI
struct smartGchar {
    gchar* p = nullptr;
    explicit smartGchar(gchar* ptr = nullptr) : p(ptr) {}
    ~smartGchar() { if (p) g_free(p); }
    std::string str() const { return p ? std::string(p) : std::string{}; }
    smartGchar(const smartGchar&) = delete;
    smartGchar& operator=(const smartGchar&) = delete;
};

// RAII helper for GObject reference management
template <typename T>
struct smartGObject {
    T* p = nullptr;
    explicit smartGObject(T* ptr = nullptr) : p(ptr) {}
    ~smartGObject() { if (p) g_object_unref(p); }
    T* get() const { return p; }
    T* release() { T* tmp = p; p = nullptr; return tmp; }
    explicit operator bool() const { return p != nullptr; }
    smartGObject(const smartGObject&) = delete;
    smartGObject& operator=(const smartGObject&) = delete;
};

// Alias for compatibility
template <typename T>
using GObjGuard = smartGObject<T>;
using GCharGuard = smartGchar;
#endif


class AccessibilityException : public std::runtime_error {
public:
    explicit AccessibilityException(const std::string& message)
        : std::runtime_error("Accessibility Error: " + message) {}
};

class AccessibilityService {
public:
    static AccessibilityService& getInstance();

    AccessibilityService(const AccessibilityService&) = delete;
    AccessibilityService& operator=(const AccessibilityService&) = delete;

    AccessibilityState captureFullState();
    AccessibilityState captureForegroundWindowState();
    AccessibilityState captureTargetWindowState(const std::string& processNameOrTitle);

    ~AccessibilityService();

private:
    AccessibilityService();

#if defined(_WIN32) || defined(_WIN64)
    Microsoft::WRL::ComPtr<IUIAutomation> automation_;
    Microsoft::WRL::ComPtr<IUIAutomationTreeWalker> walker_;

    // RAII helper for BSTR on Windows
    class ScopedBSTR {
    public:
        explicit ScopedBSTR(BSTR bstr = nullptr) : bstr_(bstr) {}
        ~ScopedBSTR() { if (bstr_) SysFreeString(bstr_); }
        BSTR get() const { return bstr_; }
        BSTR* receive() { return &bstr_; }
        ScopedBSTR(const ScopedBSTR&) = delete;
        ScopedBSTR& operator=(const ScopedBSTR&) = delete;
    private:
        BSTR bstr_;
    };

    static std::string fromBSTR_WIN(BSTR bstr);
    static ElementType mapControlType_WIN(CONTROLTYPEID id);

    void walkTree_WIN(IUIAutomationElement* element,
                      int parentId,
                      int depth,
                      AccessibilityState& state);

    AccessibilityState captureFullState_WIN();
    AccessibilityState captureForegroundWindowState_WIN();
    AccessibilityState captureTargetWindowState_WIN(const std::string& processNameOrTitle);
#endif

#if defined(__linux__)
    static bool atspiInitialized_;
    void ensureAtspiInit();

    static void readStates(AtspiAccessible* acc, SemanticNode& node);
    static void readValue(AtspiAccessible* acc, SemanticNode& node);
    static void readAttributes(AtspiAccessible* acc, SemanticNode& node);
    static void readClickable(AtspiAccessible* acc, SemanticNode& node);

    static ElementType mapAtSpiRole_LINUX(AtspiRole role);

    void walkTree_LINUX(AtspiAccessible* element,
                       int parentId,
                       int depth,
                       AccessibilityState& state);

    AccessibilityState captureFullState_LINUX();
    AccessibilityState captureForegroundWindowState_LINUX();
    AccessibilityState captureTargetWindowState_LINUX(const std::string& processNameOrTitle);
#endif

    int nextId_ = 1;
    bool comInitialized_ = false;
    static constexpr int kMaxDepth = 40;
};
