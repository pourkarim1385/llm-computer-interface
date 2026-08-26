#include "MouseClickLinux.hpp"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <iostream>

// button: 1=left, 2=middle, 3=right
void mouseButtonDown(unsigned int button) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return;
    }

    XTestFakeButtonEvent(display, button, True, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
}

void mouseButtonUp(unsigned int button) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return;
    }

    XTestFakeButtonEvent(display, button, False, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
}

void clickMouseLeft() {
    mouseButtonDown(1);
    mouseButtonUp(1);
}

void clickMouseRight() {
    mouseButtonDown(3);
    mouseButtonUp(3);
}

void clickMouseMiddle() {
    mouseButtonDown(2);
    mouseButtonUp(2);
}
