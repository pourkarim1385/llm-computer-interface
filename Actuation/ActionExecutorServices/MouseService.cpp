#include "MouseService.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include "Actuation/ActionExecutorServices/MouseSource/MouseClickWin.hpp"
#include "Actuation/ActionExecutorServices/MouseSource/MoveMouseWin.hpp"
#elif defined(__linux__)
#include "Actuation/ActionExecutorServices/MouseSource/MouseClickLinux.hpp"
#include "Actuation/ActionExecutorServices/MouseSource/MoveMouseLinux.hpp"
#else
#error "Unsupported operating system: Mouse actuation not implemented."
#endif

void MouseService::moveMouse(int x, int y) {
#if defined(_WIN32) || defined(_WIN64)
    moveMouseSmoothWin(x, y);
#else
    moveMouseSmoothLinux(x, y);
#endif
}

void MouseService::clickMouse(const Actions::MouseButton& op) {
#if defined(_WIN32) || defined(_WIN64)
    if (op == Actions::MouseButton::Left) {
        clickMouseLeftWin32();
    }
    else if (op == Actions::MouseButton::Right) {
        clickMouseRightWin32();
    }
    else if (op == Actions::MouseButton::Middle) {
        clickMouseMiddleWin32();
    }
    else if (op == Actions::MouseButton::Double) {
        doubleClick();
    }
#else
    if (op == Actions::MouseButton::Right) {
        clickMouseRightLinux();
    }
    else if (op == Actions::MouseButton::Left) {
        clickMouseLeftLinux();
    }
    else if (op == Actions::MouseButton::Middle) {
        clickMouseMiddleLinux();
    }
    else if (op == Actions::MouseButton::Double) {
        doubleClickLinux();
    }
#endif
}

void MouseService::scrollMouse(int direction, int amount) {
#if defined(_WIN32) || defined(_WIN64)
    scroll(direction, amount);
#else
    scrollLinux(direction, amount);
#endif
}

void MouseService::mouseClickHold(const Actions::MouseButton button) {
#if defined(_WIN32) || defined(_WIN64)
    mouseButtonHoldWin32(convertMouseButton(button));
#else
    mouseButtonHoldLinux(convertMouseButton(button));
#endif
}

void MouseService::mouseClickRelease(const Actions::MouseButton button) {
#if defined(_WIN32) || defined(_WIN64)
    mouseButtonReleaseWin32(convertMouseButton(button));
#else
    mouseButtonReleaseLinux(convertMouseButton(button));
#endif
}

void MouseService::dragMouse(int start_x, int start_y, int duration, int step) {
#if defined(_WIN32) || defined(_WIN64)
    dragMouseWindows(start_x, start_y, duration, step);
#else
    dragMouseLinux(start_x, start_y, duration, step);
#endif
}

unsigned int MouseService::convertMouseButton(Actions::MouseButton button) {
    switch (button) {
    case Actions::MouseButton::Left:
        return 1;
    case Actions::MouseButton::Right:
        return 3;
    case Actions::MouseButton::Middle:
        return 2;
    default:
        return 0;
    }
}