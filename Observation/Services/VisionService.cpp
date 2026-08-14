#include "VisionService.hpp"

VisionService::VisionService(/* args */)
{
}

VisionService::~VisionService()
{
}


void VisionService::captureAndCompare(){
    capturPic.capture();
    capturPic.compare();

    std::ifstream file(SCREENSHOT_PATH, std::ios::binary);
    visionState.picture = std::vector<unsigned char>(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
}

VisionState& VisionService::get_current_state(){
    return visionState;
}