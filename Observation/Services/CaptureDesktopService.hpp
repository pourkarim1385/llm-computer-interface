#pragma once

#include "Models/DesktopState.hpp"
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

class DesktopService
{
private:
    DesktopService() = default;
    ~DesktopService() = default;

    DesktopService& operator=(const DesktopService&) = delete;
    DesktopService(const DesktopService&) = delete;

    DesktopState desktopState;

    void get_system_info();
    void getActiveWindow();

public:
    static DesktopService& get_instance()
    {
        static DesktopService instance;
        return instance;
    }
};
