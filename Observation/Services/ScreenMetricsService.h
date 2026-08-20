#ifndef ACCESSIBILITYSERVICE_SCREENMETRICSSERVICE_H
#define ACCESSIBILITYSERVICE_SCREENMETRICSSERVICE_H

#include <mutex>
#include "../Models/ScreenMetricsState.h"

struct ScreenSize {
    int width;
    int height;
};

class ScreenMetricsService {
private:
    ScreenMetricsService() : cachedScreenSize{0, 0}, cachedScaleFactor(0.0f), isCached(false) {}

    float measureDpiScale();
    ScreenSize measure();

    ScreenSize cachedScreenSize;
    float cachedScaleFactor;
    bool isCached;

    std::mutex cacheMutex;

public:
    static ScreenMetricsService& getInstance();

    ScreenMetricsService(const ScreenMetricsService&) = delete;
    ScreenMetricsService& operator=(const ScreenMetricsService&) = delete;

    ScreenMetricsState getCurrentState(bool calNew = false);
};

#endif //ACCESSIBILITYSERVICE_SCREENMETRICSSERVICE_H