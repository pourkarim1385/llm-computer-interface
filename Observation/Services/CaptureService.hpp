#pragma once

#include <vector>
#include <cstdint>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #include "CaptureServices/CaptureWin.hpp"
#elif defined(__linux__)
    #include "CaptureServices/CaptureLinux.hpp"
#endif

class CapturPic {
public:
    static CapturPic& getInstance() {
        static CapturPic instance;
        return instance;
    }

    CapturPic(const CapturPic&) = delete;
    CapturPic& operator=(const CapturPic&) = delete;
    CapturPic(CapturPic&&) = delete;
    CapturPic& operator=(CapturPic&&) = delete;

    void capture();
    void compare();

private:
    CapturPic() = default;
    ~CapturPic() = default;
};
