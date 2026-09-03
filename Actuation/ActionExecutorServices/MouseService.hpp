#pragma once
#include <iostream>
#include "../Actions.h"

#ifdef _WIN32
    #include "Actuation/ActionExecutorServices/MouseSource/MouseClickWin.hpp"
    #include "Actuation/ActionExecutorServices/MouseSource/MoveMouseWin.hpp"
#else
    #include "Actuation/ActionExecutorServices/MouseSource/MouseClickLinux.hpp"
    #include "Actuation/ActionExecutorServices/MouseSource/MoveMouseLinux.hpp"
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
    void clickMouse(const Actions::MouseButton& op);

private:
    MouseService() = default;
    ~MouseService() = default;
};
