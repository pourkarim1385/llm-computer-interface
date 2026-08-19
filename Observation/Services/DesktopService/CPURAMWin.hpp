#pragma once
#include <windows.h>
#include <psapi.h>
#include <string>
#include <vector>

struct ProcessInfo {
    DWORD   pid;
    std::string name;
    double  cpuPercent;
    SIZE_T  memBytes;      
};

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