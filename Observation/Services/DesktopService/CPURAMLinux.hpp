#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include "Observation/Models/DesktopState.hpp"

class ProcessMonitor {
public:
    ProcessMonitor();
    ~ProcessMonitor();

    std::vector<ProcessInfo> refresh();

private:
    struct Impl;
    Impl* impl_;
};