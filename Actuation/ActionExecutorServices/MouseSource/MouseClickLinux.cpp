#include "MouseClickLinux.hpp"

#include <chrono>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <iostream>
#include <thread>

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
    mouseButtonDownLinux(1);
    mouseButtonUpLinux(1);
}

void clickMouseRightLinux() {
    mouseButtonDownLinux(3);
    mouseButtonUpLinux(3);
}

void clickMouseMiddleLinux() {
    mouseButtonDownLinux(2);
    mouseButtonUpLinux(2);
}

void doubleClickLinux() {
    mouseButtonDownLinux(1);
    mouseButtonUpLinux(1);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    mouseButtonDownLinux(1);
    mouseButtonUpLinux(1);
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