#pragma once

#include "Models/CaptureSource/CaptureMain.hpp"
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
    CapturPic& capturPic = CapturPic::getInstance();
    void captureAndCompare();
    
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



