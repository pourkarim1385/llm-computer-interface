#include "MouseClickWin.hpp"

#include <chrono>
#include <thread>
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

void doubleClick() {
    clickMouseLeftWin32();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    clickMouseLeftWin32();
}


void scroll(int direction, int amount) {
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    // WHEEL_DELTA = 120 per notch; direction > 0 = up, < 0 = down
    input.mi.mouseData = static_cast<DWORD>(direction > 0 ? WHEEL_DELTA : -WHEEL_DELTA);

    for (int i = 0; i < amount; ++i) {
        SendInput(1, &input, sizeof(INPUT));
    }
}
void mouseButtonHoldWin32(unsigned int button) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;

    if (button == 1) input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    else if (button == 2) input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
    else if (button == 3) input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    else return;

    SendInput(1, &input, sizeof(INPUT));
}

void mouseButtonReleaseWin32(unsigned int button) {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;

    if (button == 1) input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    else if (button == 2) input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
    else if (button == 3) input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    else return;

    SendInput(1, &input, sizeof(INPUT));
}