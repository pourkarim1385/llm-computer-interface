#include "MouseService.hpp"

void MouseService::moveMouse(int x, int y){
    #ifdef _WIN32
        moveMouseWin32Input(x, y);
    #else
        moveMouseXTest(x, y);
    #endif
}

void MouseService::clickMouse(const std::string& op){
    #ifdef _WIN32
        if(op == "right"){
            clickMouseLeftWin32();
        }else if(op == "left"){
            clickMouseRightWin32();
        }
    #else
        if(op == "right"){
            clickMouseRight();
        }else if(op == "left"){
            clickMouseLeft();
        }
    #endif
}   
