#pragma once
#include "ImageCompare.hpp"

#ifdef _WIN32
    #include "CaptureServices/CaptureWin.hpp"
#else
    #include "CaptureServices/CaptureWin.hpp"
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
