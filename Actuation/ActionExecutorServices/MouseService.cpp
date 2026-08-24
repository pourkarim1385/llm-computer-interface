#include "MouseService.hpp"
#include "Actuation/Actions.h"

void MouseService::moveMouse(int x, int y){
    #ifdef _WIN32
        moveMouseWin32Input(x, y);
    #else
        moveMouseXTest(x, y);
    #endif
}

void MouseService::clickMouse(const Actions::MouseButton& op){
    #ifdef _WIN32
        if(op == Actions::MouseButton::Left){
            clickMouseLeftWin32();
        }else if(op == Actions::MouseButton::Right){
            clickMouseRightWin32();
        }
    #else
        if(op == Actions::MouseButton::Right){
            clickMouseRight();
        }else if(op == Actions::MouseButton::Left){
            clickMouseLeft();
        }
    #endif
}   
