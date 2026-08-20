#include "DesktopState.hpp"
#include <sstream>


DesktopState::DesktopState(ActiveWindowInfo activeApp, std::vector<ProcessInfo> openProcess, bool isConnected,
        std::map<std::string, std::string> baseOsData)
    : activeApp(activeApp), openProcess(openProcess), isConnected(isConnected), baseOsData(baseOsData)
{}

DesktopState::~DesktopState(){}

std::string DesktopState::resolve() {
    std::ostringstream oss;

    // 1. Base OS Data & Connection Status
    oss << "{ OS_Data: [";
    bool first = true;
    for (const auto& [key, value] : baseOsData) {
        if (!first) oss << ", ";
        oss << key << ": " << value;
        first = false;
    }
    oss << "], Network: " << (isConnected ? "Connected" : "Disconnected") << " }\n";

    // 2. Active Window Details
    oss << "Active_Window: { Title: \"" << activeApp.title
        << "\", Process: \"" << activeApp.processName
        << "\", PID: " << activeApp.pid << " }\n";

    // 3. Running Processes (Formatted as a compact list)
    oss << "Processes:\n";
    for (const auto& proc : openProcess) {
        // Note: Using cpu_percent here. You may want to remove the redundant
        // 'double cpuPercent' from your ProcessInfo struct.
        oss << "- [" << proc.pid << "] " << proc.name
            << " | CPU: " << proc.cpu_percent << "%"
            << " | RAM: " << proc.ram_kb << " KB\n";
    }

    return oss.str();
}