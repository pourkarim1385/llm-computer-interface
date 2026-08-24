#include "MoveMouseLinux.hpp"

void moveMouseXWarp(int x, int y) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return;
    }

    Window root = DefaultRootWindow(display);
    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
    XFlush(display);
    XCloseDisplay(display);
}

void moveMouseXTest(int x, int y) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return;
    }

    Window root = DefaultRootWindow(display);
    XTestFakeMotionEvent(display, DefaultScreen(display), x, y, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
}
