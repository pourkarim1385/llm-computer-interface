#include "../SystemService.h"
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <csignal>
#include <cstring>

SystemService& SystemService::getInstance() {
    static SystemService instance;
    return instance;
}

void SystemService::openApp(const std::string& name) {
    if (name.empty()) return;
    std::string command = "\"" + name + "\" &";
    int result = std::system(command.c_str());
    if (result == -1 || WEXITSTATUS(result) == 127) {
        throw std::runtime_error("Failed to launch application: " + name);
    }
}

void SystemService::runCommand(const Actions::RunCmd& action) {
    // Rely on your existing AsyncCommand implementation for Linux
}

void SystemService::runPowerShell(const Actions::RunPowerShell& action) {
    // Optionally route to pwsh if installed on Linux
}

void SystemService::openApp(const std::string& name) {
    if (name.empty()) return;
    std::string command = "\"" + name + "\" &";
    int result = std::system(command.c_str());
    if (result == -1 || WEXITSTATUS(result) == 127) {
        throw std::runtime_error("Failed to launch application: " + name);
    }
}

void SystemService::closeApp(const std::string& name) {
    if (name.empty() || name.size() > 255 || name.find('/') != std::string::npos) return;

    DIR* dir = opendir("/proc");
    if (!dir) throw std::runtime_error("Cannot open /proc");

    bool found = false;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string pidStr = entry->d_name;
        if (pidStr.empty() || !std::all_of(pidStr.begin(), pidStr.end(), ::isdigit)) continue;

        long pidLong = std::strtol(pidStr.c_str(), nullptr, 10);
        if (pidLong <= 0) continue;
        pid_t pid = static_cast<pid_t>(pidLong);

        std::ifstream commFile("/proc/" + pidStr + "/comm");
        std::string procName;
        if (commFile.is_open() && std::getline(commFile, procName)) {
            std::string candidate = procName.size() > 15 ? procName.substr(0, 15) : procName;
            std::string target = name.size() > 15 ? name.substr(0, 15) : name;

            if (candidate == target) {
                if (kill(pid, SIGTERM) == 0) found = true;
            }
        }
    }
    closedir(dir);
    if (!found) throw std::runtime_error("Failed to close application: " + name);
}

// --- Window Management ---
// Requires window system specific tools (e.g., wmctrl for X11, or compositor-specific DBus calls for Wayland)
void SystemService::focusWindow(const std::string& name) { /* TODO: Implement X11/Wayland logic */ }
void SystemService::minimizeWindow(const std::string& name) { /* TODO: Implement X11/Wayland logic */ }
void SystemService::maximizeWindow(const std::string& name) { /* TODO: Implement X11/Wayland logic */ }
void SystemService::restoreWindow(const std::string& name) { /* TODO: Implement X11/Wayland logic */ }

// --- Audio Management ---
void SystemService::setVolume(int value) {
    float volume = std::max(0.0f, std::min(1.0f, value / 100.0f));
    char cmd[64];
    std::snprintf(cmd, sizeof(cmd), "wpctl set-volume @DEFAULT_AUDIO_SINK@ %.2f", volume);
    if (std::system(cmd) != 0) throw std::runtime_error("wpctl set-volume failed");
}

void SystemService::muteVolume() {
    if (std::system("amixer set Master mute") != 0) throw std::runtime_error("Failed to mute");
}

void SystemService::unmuteVolume() {
    if (std::system("amixer set Master unmute") != 0) throw std::runtime_error("Failed to unmute");
}

// --- Power Management ---
void SystemService::sleep() {
    std::ofstream file("/sys/power/state");
    if (file.is_open()) file << "mem";
}

void SystemService::shutdown() {
    if (geteuid() != 0) throw std::runtime_error("Need root access");
    pid_t pid = fork();
    if (pid == 0) {
        char* const args[] = { const_cast<char*>("/sbin/shutdown"), const_cast<char*>("-h"), const_cast<char*>("now"), nullptr };
        execv("/sbin/shutdown", args);
        _exit(127);
    }
    waitpid(pid, nullptr, 0);
}

void SystemService::restart() {
    if (geteuid() != 0) throw std::runtime_error("Need root access");
    pid_t pid = fork();
    if (pid == 0) {
        char* const args[] = { const_cast<char*>("/sbin/shutdown"), const_cast<char*>("-r"), const_cast<char*>("now"), nullptr };
        execv("/sbin/shutdown", args);
        _exit(127);
    }
    waitpid(pid, nullptr, 0);
}