#ifndef ACCESSIBILITYSERVICE_CAPTURECLIPBOARDSERVICE_H
#define ACCESSIBILITYSERVICE_CAPTURECLIPBOARDSERVICE_H

#include <memory>

#include "AccessibilityService.h"
#include "../Models/ClipboardState.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <shellapi.h>
#endif
#if defined(__linux__)
    #include <gtk/gtk.h>
#endif


class CaptureClipboardService {
private:
    CaptureClipboardService() = default;

    static std::string utf16ToUtf8(const wchar_t* w, int wlen);

#if defined(_WIN32) || defined(_WIN64)
    ClipboardState getCurrentState_WIN() const;
    void captureText_WIN(ClipboardState& state) const;
    void captureFiles_WIN(ClipboardState& state) const;
    void captureImage_WIN(ClipboardState& state) const;

    //Minimal RAII
    class ClipboardGuard {
    public:
        explicit ClipboardGuard(const HWND owner = nullptr) : open_(::OpenClipboard(owner) != FALSE) {}
        ~ClipboardGuard() { if (open_) ::CloseClipboard(); }
        bool ok() const { return open_; }
    private:
        bool open_;
    };

#endif
#if defined(__linux__)
    void ensureGtk();
    ClipboardState getCurrentState_LINUX() const;
    void captureText_LINUX(ClipboardState& state, GtkClipboard* clip);
    void captureImage_LINUX(ClipboardState& state, GtkClipboard* clip);
#endif

public:
    static CaptureClipboardService& getInstance();
    CaptureClipboardService(const CaptureClipboardService&) = delete;
    CaptureClipboardService& operator=(const CaptureClipboardService&) = delete;

    ~CaptureClipboardService() = default;

    ClipboardState getCurrentState() const;
};

#endif //ACCESSIBILITYSERVICE_CAPTURECLIPBOARDSERVICE_H