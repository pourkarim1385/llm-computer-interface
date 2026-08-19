#include "DesktopState.hpp"



DesktopState::DesktopState(ActiveWindowInfo activeApp, std::vector<ProcessInfo> openProcess, bool isConnected,
        std::map<std::string, std::string> baseOsData)
    : activeApp(activeApp), openProcess(openProcess), isConnected(isConnected), baseOsData(baseOsData)
{
}

DesktopState::~DesktopState()
{
}
