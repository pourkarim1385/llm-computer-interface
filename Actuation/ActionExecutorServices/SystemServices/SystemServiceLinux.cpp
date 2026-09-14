#include "../SystemService.h"
#include <algorithm>
#include <filesystem>
#include <regex>
#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <csignal>
#include <cstring>

SystemService& SystemService::getInstance() {
    static SystemService instance;
    return instance;
}

void SystemService::runCommand(const Actions::RunCmd& action) {
    CommandResult result = cmd.execute(action.command, terminalType::cmd);
    action.output = result.output;
    action.cycle = result.elapsedCycles;
    if (result.exitCode != 0 || result.timedOut) {
        throw std::runtime_error("Command execution failed or timed out.");
    }
}

void SystemService::runPowerShell(const Actions::RunPowerShell& action) {
    CommandResult result = cmd.execute(action.command, terminalType::shell);
    action.output = result.output;
    action.cycle = result.elapsedCycles;
    if (result.exitCode != 0 || result.timedOut) {
        throw std::runtime_error("Shell execution failed or timed out.");
    }
}

static std::vector<std::filesystem::path> getStartMenuDirectories() {
    std::vector<std::filesystem::path> dirs;
    dirs.emplace_back("/usr/share/applications");

    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        dirs.emplace_back(std::string(homeDir) + "/.local/share/applications");
    }

    return dirs;
}

static std::vector<std::filesystem::path> getCachedShortcuts() {
    static std::vector<std::filesystem::path> cachedShortcuts;
    static bool isCached = false;

    if (!isCached) {
        for (const auto& startDir : getStartMenuDirectories()) {
            if (!std::filesystem::exists(startDir)) continue;

            for (const auto& entry : std::filesystem::recursive_directory_iterator(
                startDir, std::filesystem::directory_options::skip_permission_denied)) {

                if (entry.is_regular_file() && entry.path().extension() == ".desktop") {
                    cachedShortcuts.push_back(entry.path());
                }
            }
        }
        isCached = true;
    }
    return cachedShortcuts;
}

static std::string getExecutableFromShortcut(const std::filesystem::path& shortcutPath) {
    std::string targetExe;
    std::ifstream file(shortcutPath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.rfind("Exec=", 0) == 0) {
            std::string execLine = line.substr(5);
            size_t spacePos = execLine.find(' ');
            if (spacePos != std::string::npos) {
                execLine = execLine.substr(0, spacePos);
            }
            std::filesystem::path target(execLine);
            targetExe = target.filename().string();
            break;
        }
    }
    return targetExe;
}

static std::string findAppWindow(const std::string& name) {
    CommandResult result = SystemService::getInstance().cmd.execute("xdotool search --name \"" + name + "\" | head -n 1", terminalType::cmd);
    if (result.exitCode == 0 && !result.output.empty()) {
        std::string wid = result.output;
        wid.erase(wid.find_last_not_of(" \n\r\t") + 1);
        return wid;
    }
    return "";
}

void SystemService::openApp(const std::string& name) {
    if (name.empty()) return;

    try {
        std::regex regPattern(name, std::regex_constants::icase);

        for (const auto& shortcutPath : getCachedShortcuts()) {
            std::string shortcutName = shortcutPath.stem().string();

            if (std::regex_search(shortcutName, regPattern)) {
                std::string exec = getExecutableFromShortcut(shortcutPath);
                if (!exec.empty()) {
                    CommandResult result = cmd.execute("nohup " + exec + " >/dev/null 2>&1 &", terminalType::cmd);
                    if (result.exitCode == 0) {
                        return;
                    }
                }
            }
        }
    }
    catch (const std::regex_error&) {}

    std::string fallbackCmd = "nohup " + name + " >/dev/null 2>&1 &";
    CommandResult result = cmd.execute(fallbackCmd, terminalType::cmd);

    if (result.exitCode != 0) {
        throw std::runtime_error("Failed to open application: " + name);
    }
}

void SystemService::closeApp(const std::string& name) {
    if (name.empty()) return;
    std::string trimmedName = name;
    trimmedName.erase(trimmedName.find_last_not_of(" \n\r\t") + 1);

    DIR* dir = opendir("/proc");
    if (!dir) throw std::runtime_error("Cannot open /proc");

    bool found = false;
    struct dirent* entry;
    std::regex regPattern;
    bool useRegex = true;

    try {
        regPattern = std::regex(trimmedName, std::regex_constants::icase);
    } catch (const std::regex_error&) {
        useRegex = false;
    }

    while ((entry = readdir(dir)) != nullptr) {
        std::string pidStr = entry->d_name;
        if (pidStr.empty() || !std::all_of(pidStr.begin(), pidStr.end(), ::isdigit)) continue;

        long pidLong = std::strtol(pidStr.c_str(), nullptr, 10);
        if (pidLong <= 0) continue;
        pid_t pid = static_cast<pid_t>(pidLong);

        std::ifstream commFile("/proc/" + pidStr + "/comm");
        std::string procName;
        if (commFile.is_open() && std::getline(commFile, procName)) {
            bool isMatch = false;

            if (useRegex) {
                isMatch = std::regex_search(procName, regPattern);
            } else {
                std::string candidate = procName.size() > 15 ? procName.substr(0, 15) : procName;
                std::string target = trimmedName.size() > 15 ? trimmedName.substr(0, 15) : trimmedName;
                isMatch = (candidate == target);
            }

            if (isMatch) {
                if (kill(pid, SIGTERM) == 0) found = true;
            }
        }
    }
    closedir(dir);

    if (!found) {
        std::string command = "pkill -15 -f -i \"" + trimmedName + "\"";
        if (cmd.execute(command, terminalType::cmd).exitCode == 0) {
            found = true;
        }
    }

    if (!found) {
        throw std::runtime_error("Failed to close application: " + name);
    }
}

void SystemService::focusWindow(const std::string& name) {
    std::string wid = findAppWindow(name);
    if (wid.empty()) throw std::runtime_error("Could not find window to focus: " + name);

    cmd.execute("xdotool windowactivate " + wid, terminalType::cmd);
}

void SystemService::minimizeWindow(const std::string& name) {
    std::string wid = findAppWindow(name);
    if (wid.empty()) throw std::runtime_error("Could not find window to minimize: " + name);

    cmd.execute("xdotool windowminimize " + wid, terminalType::cmd);
}

void SystemService::maximizeWindow(const std::string& name) {
    std::string command = "wmctrl -r \"" + name + "\" -b add,maximized_vert,maximized_horz";
    if (cmd.execute(command, terminalType::cmd).exitCode != 0) {
        throw std::runtime_error("Could not find window to maximize: " + name);
    }
}

void SystemService::restoreWindow(const std::string& name) {
    std::string command = "wmctrl -r \"" + name + "\" -b remove,maximized_vert,maximized_horz";
    if (cmd.execute(command, terminalType::cmd).exitCode != 0) {
        throw std::runtime_error("Could not find window to restore: " + name);
    }
}

static bool manipulateAudio(const std::string& commandStr) {
    return SystemService::getInstance().cmd.execute(commandStr, terminalType::cmd).exitCode == 0;
}

void SystemService::setVolume(int value) {
    int clampedVol = std::max(0, std::min(100, value));
    std::string strVol = std::to_string(clampedVol) + "%";

    if (!manipulateAudio("wpctl set-volume @DEFAULT_AUDIO_SINK@ " + strVol)) {
        if (!manipulateAudio("amixer sset Master " + strVol)) {
            if (!manipulateAudio("pactl set-sink-volume @DEFAULT_SINK@ " + strVol)) {
                throw std::runtime_error("Failed to set system volume.");
            }
        }
    }
}

void SystemService::muteVolume() {
    if (!manipulateAudio("wpctl set-mute @DEFAULT_AUDIO_SINK@ 1")) {
        if (!manipulateAudio("amixer sset Master mute")) {
            if (!manipulateAudio("pactl set-sink-mute @DEFAULT_SINK@ 1")) {
                throw std::runtime_error("Failed to mute system volume.");
            }
        }
    }
}

void SystemService::unmuteVolume() {
    if (!manipulateAudio("wpctl set-mute @DEFAULT_AUDIO_SINK@ 0")) {
        if (!manipulateAudio("amixer sset Master unmute")) {
            if (!manipulateAudio("pactl set-sink-mute @DEFAULT_SINK@ 0")) {
                throw std::runtime_error("Failed to unmute system volume.");
            }
        }
    }
}

void SystemService::sleep() {
    cmd.execute("systemctl suspend", terminalType::cmd);
}

void SystemService::shutdown() {
    cmd.execute("shutdown -h now", terminalType::cmd);
}

void SystemService::restart() {
    cmd.execute("shutdown -r now", terminalType::cmd);
}