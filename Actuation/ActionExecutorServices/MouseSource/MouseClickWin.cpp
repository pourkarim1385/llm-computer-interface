#include "MouseClickWin.hpp"
#include <windows.h>

void clickMouseLeftWin32() {
    INPUT down = {0};
    down.type = INPUT_MOUSE;
    down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    INPUT up = {0};
    up.type = INPUT_MOUSE;
    up.mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(1, &down, sizeof(INPUT));
    SendInput(1, &up, sizeof(INPUT));
}

void clickMouseRightWin32() {
    INPUT down = {0};
    down.type = INPUT_MOUSE;
    down.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

    INPUT up = {0};
    up.type = INPUT_MOUSE;
    up.mi.dwFlags = MOUSEEVENTF_RIGHTUP;

    SendInput(1, &down, sizeof(INPUT));
    SendInput(1, &up, sizeof(INPUT));
}

void clickMouseMiddleWin32() {
    INPUT down = {0};
    down.type = INPUT_MOUSE;
    down.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

    INPUT up = {0};
    up.type = INPUT_MOUSE;
    up.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;

    SendInput(1, &down, sizeof(INPUT));
    SendInput(1, &up, sizeof(INPUT));
}