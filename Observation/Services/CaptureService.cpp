#include "CaptureService.hpp"
#include "ImageCompare.hpp"


void CapturPic::capture(){
    #if defined(_WIN32) || defined(_WIN64)
        captureScreenshotWindows();
    #elif defined(__linux__)
        captureScreenshotLinux();
    #endif
}
