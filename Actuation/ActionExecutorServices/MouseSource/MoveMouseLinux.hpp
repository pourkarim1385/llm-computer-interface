#pragma  once

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cmath>
#include <thread>
#include <chrono>
#include "exception"

inline double easeInOut(double t);
void moveMouseSmoothLinux(int targetX, int targetY, int durationMs = 300, int steps = 60);
void dragMouseLinux(int targetX, int targetY, int durationMs, int steps);