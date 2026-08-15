#include "CaptureService.hpp"


void CapturPic::capture(){
    #ifdef _WIN32
        captureScreenshotWindows();
    #else
        captureScreenshotLinux();
    #endif
}

void CapturPic::compare(){
    // The number can be changed.
    cleanupDuplicateScreenshots("screenshots", 20);
}
