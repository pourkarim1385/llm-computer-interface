#pragma once
#include <iostream>

#ifdef _WIN32
    #include "Models/MouseSource/MouseClickWin.hpp"
    #include "Models/MouseSource/MoveMouseWin.hpp"
#else
    #include "Models/MouseSource/MouseClickLinux.hpp"
    #include "Models/MouseSource/MoveMouseLinux.hpp"
#endif

class MouseService {
public:
    static MouseService& getInstance() {
        static MouseService instance;
        return instance;
    }

    MouseService(const MouseService&) = delete;
    MouseService& operator=(const MouseService&) = delete;
    MouseService(MouseService&&) = delete;
    MouseService& operator=(MouseService&&) = delete;

    void moveMouse(int x, int y);
    void clickMouse(const std::string& op);

private:
    MouseService() = default;
    ~MouseService() = default;
};
