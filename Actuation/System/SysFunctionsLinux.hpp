#pragma once

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

class SysfunctionsLinux
{
private:
    SysfunctionsLinux() = default;
    ~SysfunctionsLinux() = default;
    SysfunctionsLinux operator=(const SysfunctionsLinux& other) = delete;
    SysfunctionsLinux (SysfunctionsLinux& other) = delete;
    SysfunctionsLinux operator=(const SysfunctionsLinux&& other) = delete;
    SysfunctionsLinux (SysfunctionsLinux&& other) = delete;

public:
    SysfunctionsLinux& getInstance(){
        static SysfunctionsLinux instance;
        return instance;
    };

    void shutDown(int delay_minutes = 0);
};

