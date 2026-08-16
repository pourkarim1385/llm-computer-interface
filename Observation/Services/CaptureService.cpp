#include "CaptureService.hpp"
#include "ImageCompare.hpp"


VisionState CaptureService::capture(){
    ImageFormat fmt;

    #if defined(_WIN32) || defined(_WIN64)
        return VisionState(captureScreenshotWindows(fmt), fmt);
    #elif defined(__linux__)
        return VisionState(captureScreenshotLinux(fmt), fmt);
    #endif
}

void CaptureService::compare(){
    // The number can be changed.
    cleanupDuplicateScreenshots("screenshots", 20);
}