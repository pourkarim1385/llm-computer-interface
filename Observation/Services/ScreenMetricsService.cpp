#include "ScreenMetricsService.h"

#include <cmath>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif __linux__
#include <X11/Xlib.h>
    #include <X11/extensions/Xrandr.h>
#endif

ScreenMetricsService& ScreenMetricsService::getInstance() {
    static ScreenMetricsService instance;
    return instance;
}

ScreenSize ScreenMetricsService::measure() {
#ifdef _WIN32
    return ScreenSize{GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
#elif __linux__
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        // اصلاح شد: به جای 0 باید یک آبجکت ScreenSize برگردانیم
        return ScreenSize{0, 0};
    }

    int screen = DefaultScreen(display);
    const int width = DisplayWidth(display, screen);
    const int height = DisplayHeight(display, screen);
    XCloseDisplay(display);
    return ScreenSize{width, height};
#else
    return ScreenSize{0,0};
#endif
}

float ScreenMetricsService::measureDpiScale() {
#ifdef _WIN32
    POINT cursorPosition{};

    if (!GetCursorPos(&cursorPosition)) {
        std::cerr << "GetCursorPos failed.\n";
        return 1.0f;
    }

    HMONITOR hMonitor = MonitorFromPoint(cursorPosition, MONITOR_DEFAULTTONEAREST);
    if (hMonitor == nullptr) {
        std::cerr << "MonitorFromPoint failed.\n";
        return 1.0f;
    }

    UINT dpiX = 96;
    UINT dpiY = 96;

    HMODULE hShcore = LoadLibraryA("Shcore.dll");
    if (hShcore) {
        typedef HRESULT(WINAPI * PGETDPIFORMONITOR)(HMONITOR, int, UINT*, UINT*);
        PGETDPIFORMONITOR pGetDpiForMonitor = (PGETDPIFORMONITOR)GetProcAddress(hShcore, "GetDpiForMonitor");

        if (pGetDpiForMonitor) {
            pGetDpiForMonitor(hMonitor, 0, &dpiX, &dpiY);
        } else {
            std::cerr << "GetDpiForMonitor function not found in Shcore.dll.\n";
        }
        FreeLibrary(hShcore);
    } else {
        std::cerr << "Failed to load Shcore.dll.\n";
    }

    return static_cast<float>(dpiX) / 96.0f;

#elif __linux__
    Display* display = XOpenDisplay(nullptr);

    if (display == nullptr) {
        std::cerr << "Cannot connect to X11 display server.\n";
        return 1.0f;
    }

    Window root = DefaultRootWindow(display);
    Window rootReturn{}, childReturn{};
    int rootX = 0, rootY = 0, windowX = 0, windowY = 0;
    unsigned int mask{};

    if (!XQueryPointer(display, root, &rootReturn, &childReturn, &rootX, &rootY, &windowX, &windowY, &mask)) {
        std::cerr << "XQueryPointer failed.\n";
        XCloseDisplay(display);
        return 1.0f;
    }

    XRRScreenResources* resources = XRRGetScreenResourcesCurrent(display, root);
    if (resources == nullptr) {
        std::cerr << "XRRGetScreenResourcesCurrent failed.\n";
        XCloseDisplay(display);
        return 1.0f;
    }

    float scaleFactor = 1.0f;

    for (int i = 0; i < resources->noutput; ++i) {
        XRROutputInfo* outputInfo = XRRGetOutputInfo(display, resources, resources->outputs[i]);
        if (outputInfo == nullptr) continue;

        if (outputInfo->connection != RR_Connected || outputInfo->crtc == 0) {
            XRRFreeOutputInfo(outputInfo);
            continue;
        }

        XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, resources, outputInfo->crtc);
        if (crtcInfo == nullptr) {
            XRRFreeOutputInfo(outputInfo);
            continue;
        }

        bool mouseIsOnThisMonitor = rootX >= crtcInfo->x &&
                                    rootX < crtcInfo->x + static_cast<int>(crtcInfo->width) &&
                                    rootY >= crtcInfo->y &&
                                    rootY < crtcInfo->y + static_cast<int>(crtcInfo->height);

        if (mouseIsOnThisMonitor) {
            int widthPx = static_cast<int>(crtcInfo->width);
            int widthMm = outputInfo->mm_width;

            if (widthMm > 0) {
                constexpr double mmPerInch = 25.4;
                double dpiX = (static_cast<double>(widthPx) / static_cast<double>(widthMm)) * mmPerInch;

                int scalingPercent = static_cast<int>(std::lround(dpiX / 96.0 * 100.0));
                scaleFactor = static_cast<float>(scalingPercent) / 100.0f;
            }

            XRRFreeCrtcInfo(crtcInfo);
            XRRFreeOutputInfo(outputInfo);
            break;
        }

        XRRFreeCrtcInfo(crtcInfo);
        XRRFreeOutputInfo(outputInfo);
    }

    XRRFreeScreenResources(resources);
    XCloseDisplay(display);

    return scaleFactor;

#else
    return 1.0f;
#endif
}

ScreenMetricsState ScreenMetricsService::getCurrentState(bool calNew) {
    std::lock_guard<std::mutex> lock(cacheMutex);

    if (calNew || !isCached) {
        cachedScreenSize = measure();
        cachedScaleFactor = measureDpiScale();
        isCached = true;
    }

    return ScreenMetricsState(cachedScreenSize.width, cachedScreenSize.height, cachedScaleFactor);
}