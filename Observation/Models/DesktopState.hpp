#pragma once

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <cstdint>


struct ActiveWindowInfo {
    std::string title;
    std::string processName;
    unsigned long pid;
};

struct ProcessInfo {
    int pid = 0;
    std::string name;
    double cpu_percent = 0.0;
    std::uint64_t ram_kb = 0;
    double cpuPercent;
};

class DesktopState
{
private:
    ActiveWindowInfo activeApp;
    std::vector<ProcessInfo> openProcess;
    bool isConnected;
    std::map<std::string, std::string> baseOsData;

public:  
    DesktopState() = default;
    DesktopState(ActiveWindowInfo activeApp, std::vector<ProcessInfo> openProcess, bool isConnected,
        std::map<std::string, std::string> baseOsData);
    ~DesktopState();
};
