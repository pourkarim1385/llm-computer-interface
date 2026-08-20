#include "ScreenMetricsState.h"


std::string ScreenMetricsState::resolve() {
    return "Resolution " + std::to_string(width) + "x" + std::to_string(height) +
            ", DPI Scale: " + std::to_string(scaleFactor);
}