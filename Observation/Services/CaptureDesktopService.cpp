#include "CaptureDesktopService.hpp"

DesktopState DesktopService::getCurrentState(){
    ProcessMonitor monitor;
#if defined(_WIN32) || defined(_WIN64)
    ActiveWindowInfo activeApp = getActiveApp();
    std::vector<ProcessInfo> openProcess = monitor.refresh();
#elif defined(__linux__)
    ActiveWindowInfo activeApp = getActiveApp();
        std::vector<ProcessInfo> openProcess = monitor.refresh();
#endif

    bool isConnected = IsNetworkConnected();
    if(!isCached) {
        std::map<std::string, std::string> baseOsData = getOsIinfo();
        isCached = true;
    }

    return DesktopState(activeApp, openProcess,
                                isConnected, cachedBaseOsData);
}