#include "VisionService.hpp"

VisionService::VisionService(/* args */)
{
}

VisionService::~VisionService()
{
}


void VisionService::visionInitializer(){
    visionState = captureService.capture();
}

VisionState& VisionService::get_current_state(){
    return visionState;
}