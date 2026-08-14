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
    static VisionState& getInstance() {
        static VisionState instance;
        return instance;
    }

    VisionState(const VisionState&) = delete;
    VisionState& operator=(const VisionState&) = delete;
    VisionState(VisionState&&) = delete;
    VisionState& operator=(VisionState&&) = delete;


    std::vector<unsigned char> picture;

private:
    VisionState() = default;
    ~VisionState() = default;
};

#endif //ACCESSIBILITYSERVICE_VISIONSTATE_H
