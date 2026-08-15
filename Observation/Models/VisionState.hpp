// VisionState.hpp
#ifndef VISIONSTATE_H
#define VISIONSTATE_H

#include <iostream>
#include <fstream>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    #define SCREENSHOT_PATH "screenshots\\1.png"
#elif defined(__linux__)
    #define SCREENSHOT_PATH "screenshots/1.png"
#else
    #error "Unsupported platform"
#endif

class VisionState {
public:
    std::vector<unsigned char> picture;
    
    VisionState() = default;
    ~VisionState() = default;

private:
};

#endif //ACCESSIBILITYSERVICE_VISIONSTATE_H
