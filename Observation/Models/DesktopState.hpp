#pragma once

#include <iostream>
#include <string.h>
#include <vector>
#include <map>

struct ActiveWindowInfo {
    std::string title;
    std::string processName;
    unsigned long pid;
};


class DesktopState
{
private:
    std::map<std::string, std::string> osInfo;
    ActiveWindowInfo activeApp;

public:    

    void setOsInfo(std::map<std::string, std::string> new_os_info);
    void setActiveApp(ActiveWindowInfo activeApp);
    DesktopState();
    ~DesktopState();
};
