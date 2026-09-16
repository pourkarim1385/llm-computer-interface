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

void moveMouseSmoothWin(int targetX, int targetY, int durationMs, int steps) {
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

inline double easeInOut(double t) {
    return t < 0.5
        ? 2 * t * t
        : 1 - std::pow(-2 * t + 2, 2) / 2;
}

void dragMouseWindows(int targetX, int targetY, int durationMs, int steps) {
    // Get current cursor position
    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        throw std::runtime_error("Failed to get cursor position");
    }

    double startX = static_cast<double>(cursorPos.x);
    double startY = static_cast<double>(cursorPos.y);
    double deltaX = targetX - startX;
    double deltaY = targetY - startY;

    int stepDelayUs = (durationMs * 1000) / steps;

    // Get screen dimensions for MOUSEEVENTF_ABSOLUTE coordinate normalization
    int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Helper: convert pixel coords to MOUSEEVENTF_ABSOLUTE units (0–65535)
    auto toAbsolute = [&](int x, int y) -> std::pair<LONG, LONG> {
        return {
            static_cast<LONG>((x * 65535) / (screenWidth  - 1)),
            static_cast<LONG>((y * 65535) / (screenHeight - 1))
        };
    };

    // Press left button at current position
    {
        auto [ax, ay] = toAbsolute(static_cast<int>(startX), static_cast<int>(startY));
        INPUT input{};
        input.type           = INPUT_MOUSE;
        input.mi.dx          = ax;
        input.mi.dy          = ay;
        input.mi.dwFlags     = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Smooth drag
    for (int i = 1; i <= steps; ++i) {
        double t      = static_cast<double>(i) / steps;
        double easedT = easeInOut(t);

        int curX = static_cast<int>(startX + deltaX * easedT);
        int curY = static_cast<int>(startY + deltaY * easedT);

        auto [ax, ay] = toAbsolute(curX, curY);

        INPUT input{};
        input.type       = INPUT_MOUSE;
        input.mi.dx      = ax;
        input.mi.dy      = ay;
        input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));

        std::this_thread::sleep_for(std::chrono::microseconds(stepDelayUs));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Release left button at target position
    {
        auto [ax, ay] = toAbsolute(targetX, targetY);
        INPUT input{};
        input.type       = INPUT_MOUSE;
        input.mi.dx      = ax;
        input.mi.dy      = ay;
        input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}