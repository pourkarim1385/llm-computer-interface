#ifndef ACCESSIBILITYSERVICE_SCREENMETRICSSERVICE_H
#define ACCESSIBILITYSERVICE_SCREENMETRICSSERVICE_H

#include "../Models/ScreenMetricsState.h"

namespace {
    struct ScreenSize {
        int width; int height;
    };
}

class ScreenMetricsService {
private:
    ScreenMetricsService() {cachedScreenSize = measure(); cachedScaleFactor = measureDpiScale();}
    float measureDpiScale();
    ScreenSize cachedScreenSize;
    float cachedScaleFactor;
    ScreenSize measure();

public:
    static ScreenMetricsService& getInstance();

    ScreenMetricsService(const ScreenMetricsService&) = delete;
    ScreenMetricsService& operator=(const ScreenMetricsService&) = delete;

    ScreenMetricsState getCurrentState(bool calNew = false);
};

#endif //ACCESSIBILITYSERVICE_SCREENMETRICSSERVICE_H
