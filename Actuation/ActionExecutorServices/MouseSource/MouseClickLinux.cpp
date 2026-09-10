#include "MouseClickLinux.hpp"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <iostream>

// button: 1=left, 2=middle, 3=right
void mouseButtonDownLinux(unsigned int button) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return;
    }

    XTestFakeButtonEvent(display, button, True, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
}

void mouseButtonUpLinux(unsigned int button) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return;
    }

    XTestFakeButtonEvent(display, button, False, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
}

void clickMouseLeftLinux() {
    mouseButtonDown(1);
    mouseButtonUp(1);
}

void clickMouseRightLinux() {
    mouseButtonDown(3);
    mouseButtonUp(3);
}

void clickMouseMiddleLinux() {
    mouseButtonDown(2);
    mouseButtonUp(2);
}

void doubleClickLinux() {
    mouseButtonDown(1);
    mouseButtonUp(1);

    usleep(50000);
    mouseButtonDown(1);
    mouseButtonUp(1);
}

// Button4 -> up   Button5-> Down
void scrollLinux(int direction, int amount) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;

    int button = (direction > 0) ? 4 : 5;

    for (int i = 0; i < amount; ++i) {
        XTestFakeButtonEvent(display, button, True, CurrentTime);
        XTestFakeButtonEvent(display, button, False, CurrentTime);
        XFlush(display);
    }

    XCloseDisplay(display);
}

void mouseButtonHoldLinux(unsigned int button) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;
    XTestFakeButtonEvent(display, button, True, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
}

void mouseButtonReleaseLinux(unsigned int button) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;
    XTestFakeButtonEvent(display, button, False, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
}