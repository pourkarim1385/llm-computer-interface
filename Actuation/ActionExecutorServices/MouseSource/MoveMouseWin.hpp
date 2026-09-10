#pragma once

#include <windows.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <stdexcept>

static void sendAbsoluteMove(int x, int y);
void moveMouseSmoothWin(int targetX, int targetY, int durationMs = 300, int steps = 60);
inline double easeInOut(double t);
void dragMouseWindows(int targetX, int targetY, int durationMs, int steps);