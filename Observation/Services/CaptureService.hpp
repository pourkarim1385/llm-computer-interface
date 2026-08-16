#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include "../Models/VisionState.hpp"

#if defined(_WIN32) || defined(_WIN64)
    #include "CaptureServices/CaptureWin.hpp"
#elif defined(__linux__)
    #include "CaptureServices/CaptureLinux.hpp"
#endif

class CaptureService {
public:
    static CaptureService& getInstance() {
        static CaptureService instance;
        return instance;
    }

    CaptureService(const CaptureService&) = delete;
    CaptureService& operator=(const CaptureService&) = delete;
    CaptureService(CaptureService&&) = delete;
    CaptureService& operator=(CaptureService&&) = delete;

    VisionState capture();

private:
    CaptureService() = default;
    ~CaptureService() = default;
};
