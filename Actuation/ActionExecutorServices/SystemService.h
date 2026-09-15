#pragma once

#include "AsyncCommand.h"
#include "../Actions.h"
#include "../ActionDispatcher.h"
#include <string>

class SystemService {
private:
    SystemService() = default;
    AsyncCommand cmd;
public:
    static SystemService& getInstance();

    SystemService(const SystemService&) = delete;
    SystemService& operator=(const SystemService&) = delete;

    void runCommand(const Actions::RunCmd& action);
    void runPowerShell(const Actions::RunPowerShell& action);

    // App Management
    void openApp(const std::string& name);
    void closeApp(const std::string& name);

    // Window Management
    void focusWindow(const std::string& name);
    void minimizeWindow(const std::string& name);
    void maximizeWindow(const std::string& name);
    void restoreWindow(const std::string& name);

    // Audio Management
    void setVolume(int value);
    void muteVolume();
    void unmuteVolume();

    // Power Management
    void sleep();
    void shutdown();
    void restart();
};