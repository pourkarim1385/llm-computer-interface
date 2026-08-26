#include "MoveMouseWin.hpp"


static void sendAbsoluteMove(int x, int y) {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dx = static_cast<LONG>((x * 65536.0) / screenW);
    input.mi.dy = static_cast<LONG>((y * 65536.0) / screenH);
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &input, sizeof(INPUT));
}

void moveMouseSmoothWin(int targetX, int targetY, int durationMs = 300, int steps = 60) {
    POINT current;
    GetCursorPos(&current);

    double startX = current.x;
    double startY = current.y;
    double deltaX = targetX - startX;
    double deltaY = targetY - startY;

    int stepDelayUs = (durationMs * 1000) / steps;

    for (int i = 1; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        double easedT = t < 0.5
            ? 2 * t * t
            : 1 - std::pow(-2 * t + 2, 2) / 2;

        int curX = static_cast<int>(startX + deltaX * easedT);
        int curY = static_cast<int>(startY + deltaY * easedT);

        sendAbsoluteMove(curX, curY);

        std::this_thread::sleep_for(std::chrono::microseconds(stepDelayUs));
    }
}
