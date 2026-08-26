#pragma  once

#include <iostream>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cmath>
#include <thread>
#include <chrono>



void moveMouseSmooth(int targetX, int targetY, int durationMs = 300, int steps = 60);
