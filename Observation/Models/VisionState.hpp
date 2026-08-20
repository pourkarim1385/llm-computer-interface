// VisionState.hpp
#ifndef VISIONSTATE_H
#define VISIONSTATE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "../Public.h"

class VisionState {
public:
    std::vector<unsigned char> getContentBinary() const;
    explicit VisionState(std::vector<unsigned char> content, ImageFormat fmt) : picture(content) , format(fmt) {}
    ImageFormat getFormat() const {return format;}
    VisionState() = default;
    ~VisionState() = default;

private:
    std::vector<unsigned char> picture;
    ImageFormat format;
};

#endif //ACCESSIBILITYSERVICE_VISIONSTATE_H
