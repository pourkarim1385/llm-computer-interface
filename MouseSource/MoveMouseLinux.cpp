#include "MoveMouseLinux.hpp"

void moveMouseSmooth(int targetX, int targetY, int durationMs = 300, int steps = 60) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return;
    }

    int screen = DefaultScreen(display);
    Window root = DefaultRootWindow(display);

    Window returnedRoot, returnedChild;
    int rootX, rootY, winX, winY;
    unsigned int mask;
    XQueryPointer(display, root, &returnedRoot, &returnedChild,
                  &rootX, &rootY, &winX, &winY, &mask);

    double startX = rootX;
    double startY = rootY;
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

        XTestFakeMotionEvent(display, screen, curX, curY, CurrentTime);
        XFlush(display);

        std::this_thread::sleep_for(std::chrono::microseconds(stepDelayUs));
    }

    XCloseDisplay(display);
}
