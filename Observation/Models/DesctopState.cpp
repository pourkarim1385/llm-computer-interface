#include "DesktopState.hpp"



DesktopState::DesktopState(/* args */)
{
}

DesktopState::~DesktopState()
{
}


void DesktopState::setOsInfo(std::map<std::string, std::string> new_info){
    osInfo = new_info;
}  

void DesktopState::setActiveApp(ActiveWindowInfo activeApp){
    activeApp = activeApp;
}