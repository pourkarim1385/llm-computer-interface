#pragma once

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <dirent.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <sys/types.h>
#include <vector>

struct KillResult {
    int  killed  = 0;
    int  failed  = 0;
    bool found   = false;
};

class SysfunctionsLinux
{
private:
    std::optional<std::string> readFile(const std::string& path);
    std::vector<pid_t> findPIDs(const std::string& name, bool matchCmdline = false);

public:
    SysfunctionsLinux() = default;
    ~SysfunctionsLinux() = default;
    void shutDown(int delayMinutes = 0);
    void restart(int delayMinutes = 0);
    void muteVolume();
    void unmuteVolume();
    void setVolume(float volume);
    KillResult killProcess(
    const std::string& name,
        int  sig          = SIGTERM,
        bool matchCmdline = false,
        bool waitForExit  = true,
        int  waitMs       = 2000
    );
    bool suspendSystem();
    bool launchProgram(const std::string& programName);
    bool closeApp(const std::string& processName, bool force = false);
};

