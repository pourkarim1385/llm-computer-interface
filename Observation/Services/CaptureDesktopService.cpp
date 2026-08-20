#include "CaptureDesktopService.hpp"


void DesktopService::getInfo(){
    ProcessMonitor monitor;
    #ifdef _WIN32
        ActiveWindowInfo activeApp = getActiveApp();
        std::vector<ProcessInfo> openProcess = monitor.refresh();
    #else
        ActiveWindowInfo activeApp = getActiveApp();        
        std::vector<ProcessInfo> openProcess = monitor.refresh();
    #endif

    bool isConnected = IsNetworkConnected();
    std::map<std::string, std::string> baseOsData = getOsIinfo();

    //Idle time shows the last movement in the system.
    // Idle time can be added but it needs a treshold(needs to be decide in the main state).

    desktopState = DesktopState(activeApp, openProcess, 
        isConnected, baseOsData);
}

DesktopState DesktopService::getCurrentDesktop(){
    return desktopState;
}