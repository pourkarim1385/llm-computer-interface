#pragma once

#include "CaptureService.hpp"
#include "Models/VisionState.hpp"
#include <vector>
#include <fstream>
#include <iostream>

class VisionService
{
private:
    VisionService() = default;
    ~VisionService() = default;

    VisionState visionState;
    CaptureService& captureService = CaptureService::getInstance();
    
public:
    static VisionService& getInstance() {
        static VisionService instance;
        return instance;
    }
    
    VisionState& get_current_state();

    VisionService(const VisionService&) = delete;
    VisionService& operator=(const VisionService&) = delete;
    VisionService(VisionService&&) = delete;
    VisionService& operator=(VisionService&&) = delete;

};



