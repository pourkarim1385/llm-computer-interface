#pragma once
#include <windows.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "Observation/Models/DesktopState.hpp"

class ProcessMonitor {
public:
    ProcessMonitor();
    std::vector<ProcessInfo> refresh(); 

private:
    struct ProcTimes {
        ULONGLONG kernel;
        ULONGLONG user;
        ULONGLONG wall;
    };


    std::unordered_map<DWORD, ProcTimes> prevTimes_;

    ULONGLONG fileTimeToU64(const FILETIME& ft) const;
};
