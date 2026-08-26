#ifndef MOVEMOUSE_HPP
#define MOVEMOUSE_HPP

#include <windows.h>
#include <cmath>
#include <thread>
#include <chrono>
static void sendAbsoluteMove(int x, int y);
void moveMouseSmoothWin(int targetX, int targetY, int durationMs = 300, int steps = 60);

#endif
