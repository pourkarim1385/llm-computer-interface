#pragma once

#include <iostream>
#include <string.h>
#include <vector>



class DesktopState
{
private:
    std::string language;
    std::string activateApp;
    int mouseX, mouseY;
    std::vector<std::string>openApps;

public:
    DesktopState(/* args */);
    ~DesktopState();
};
