#include "CaptureService.hpp"
#include "ImageCompare.hpp"


VisionState CaptureService::capture(){
    #if defined(_WIN32) || defined(_WIN64)
        return VisionState(captureScreenshotWindows());
    #elif defined(__linux__)
        return VisionState(captureScreenshotLinux());
    #endif
}

void CaptureService::compare(){
    // The number can be changed.
    cleanupDuplicateScreenshots("screenshots", 20);
}