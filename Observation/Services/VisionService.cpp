#include "VisionService.hpp"

VisionService::VisionService(/* args */)
{
}

VisionService::~VisionService()
{
}

VisionState& VisionService::get_current_state(){
    return visionState;
}