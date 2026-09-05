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
        }
    #else
        if(op == Actions::MouseButton::Right){
            clickMouseRight();
        }else if(op == Actions::MouseButton::Left){
            clickMouseLeft();
        }else if(op == Actions::MouseButton::Middle) {
            clickMouseMiddle();
        }
    #endif
}   
