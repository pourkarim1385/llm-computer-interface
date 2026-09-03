#include "IdleTime.hpp"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <X11/Xlib.h>
    #include <X11/extensions/scrnsaver.h>
#endif

static long long getIdleTimeMs() {
    #ifdef _WIN32
        LASTINPUTINFO lii;
        lii.cbSize = sizeof(LASTINPUTINFO);
        if (!GetLastInputInfo(&lii)) {
            return -1;
        }
        DWORD currentTick = GetTickCount();
        return static_cast<long long>(currentTick - lii.dwTime);
    #else
        Display* display = XOpenDisplay(nullptr);
        if (!display) {
            return -1;
        }
        XScreenSaverInfo* info = XScreenSaverAllocInfo();
        if (!info) {
            XCloseDisplay(display);
            return -1;
        }
        XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
        long long idleMs = static_cast<long long>(info->idle);
        XFree(info);
        XCloseDisplay(display);
        return idleMs;
    #endif
}

static bool isIdle(long long thresholdMs) {
        long long idle = getIdleTimeMs();
        return (idle >= 0) && (idle >= thresholdMs);
    }
