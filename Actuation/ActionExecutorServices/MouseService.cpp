#include "MouseService.hpp"
#include "Actuation/Actions.h"
#include "MouseSource/MouseClickLinux.hpp"
#include "MouseSource/MoveMouseLinux.hpp"

void MouseService::moveMouse(int x, int y){
    #ifdef _WIN32
        moveMouseSmoothWin(x, y);
    #else
        moveMouseSmoothLinux(x, y);
    #endif
}

void MouseService::clickMouse(const Actions::MouseButton& op){
    #ifdef _WIN32
        if(op == Actions::MouseButton::Left){
            clickMouseLeftWin32();
        }else if(op == Actions::MouseButton::Right){
            clickMouseRightWin32();
        }else if (op == Actions::MouseButton::Middle) {
            clickMouseMiddleWin32();
        }else if(op == Actions::MouseButton::Double){
            doubleClick();
        }
    #else
        if(op == Actions::MouseButton::Right){
            clickMouseRight();
        }else if(op == Actions::MouseButton::Left){
            clickMouseLeft();
        }else if(op == Actions::MouseButton::Middle) {
            clickMouseMiddle();
        }else if(op == Actions::MouseButton::Double){
            doubleClick();
        }
    #endif
}  

void MouseService::scrollMouse(int direction, int amount){
    #ifdef Win32
        scroll(direction, amount);
    #else
        scroll(direction, amount);
    #endif
}

void MouseService::clickPresure(int bottom){
    #ifdef Win32
        mouseButtonHold(bottom);
    #else
        mouseButtonHold(bottom);
    #endif
}

void MouseService::clickRelease(int bottom){
    #ifdef Win32
        mouseButtonRelease(bottom);
    #else
        mouseButtonRelease(bottom);
    #endif
}

void MouseService::dragMouse(int start_x, int start_y, int duration, int step){
    #ifdef Win32
        dragMouseWindows(start_x, start_y, duration, step);
    #else
        dragMouseLinux(start_x, start_y, duration, step);
    #endif
}
