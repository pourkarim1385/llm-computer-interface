#pragma once

#include "AsyncCommand.h"
#include "../Actions.h"
#include "../ActionDispatcher.h" // For ActionStatus enum

class SystemService {
private:
    SystemService() = default;
    AsyncCommand cmd;
public:
    static SystemService& getInstance();

    SystemService(const SystemService&) = delete;
    SystemService& operator=(const SystemService&) = delete;

    ActionStatus runCommand(const Actions::RunCmd& action);
    ActionStatus runPowerShell(const Actions::RunPowerShell& action);
};