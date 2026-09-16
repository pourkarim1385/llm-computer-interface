#pragma once
#include <iostream>
#include "../Actions.h"



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
    void scrollMouse(int direction, int amount);
    void mouseClickHold(const Actions::MouseButton button);
    void mouseClickRelease(const Actions::MouseButton button);
    void dragMouse(int target_x, int target_y, int end_x, int end_y);


private:
    unsigned int convertMouseButton(Actions::MouseButton button);
    MouseService() = default;
    ~MouseService() = default;
};
