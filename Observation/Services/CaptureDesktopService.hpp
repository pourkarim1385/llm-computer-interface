#pragma once

#include "Observation/Models/DesktopState.hpp"
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "DesktopService/IdleTime.hpp"
#include "DesktopService/Connection.hpp"
#include "DesktopService/BaseOsInfo.hpp"



#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
    #include "DesktopService/ActiveAppWin.hpp"
    #include "DesktopService/CPURAMWin.hpp"
#else
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
    #include <unistd.h>
    #include "DesktopService/ActiveAppLinux.hpp"
    #include "DesktopService/CPURAMLinux.hpp"
#endif

class DesktopService
{
private:
    DesktopService() = default;
    ~DesktopService() = default;

    DesktopService& operator=(const DesktopService&) = delete;
    DesktopService(const DesktopService&) = delete;

    DesktopState desktopState;

    DesktopState getCurrentDesktop();


    void getInfo();

public:
    static DesktopService& get_instance()
    {
        static DesktopService instance;
        return instance;
    }
};
