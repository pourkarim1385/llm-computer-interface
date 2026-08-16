// VisionState.hpp
#ifndef VISIONSTATE_H
#define VISIONSTATE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #define SCREENSHOT_PATH "screenshots\\1.png"
#elif defined(__linux__)
    #define SCREENSHOT_PATH "screenshots/1.png"
#else
    #error "Unsupported platform"
#endif

class VisionState {
    std::vector<unsigned char> picture;
public:
    std::vector<unsigned char> getContentBinary() const;
    explicit VisionState(std::vector<unsigned char> content) : picture(content) {}
    ~VisionState() = default;

private:
};

#endif //ACCESSIBILITYSERVICE_VISIONSTATE_H
