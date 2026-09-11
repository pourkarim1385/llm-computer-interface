#pragma once

#if defined(__linux__) || defined(__unix__)
    #include <sys/wait.h>
    #include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

#include <string>
#include <vector>

class ClipboardService
{
private:
    ClipboardService() = default;
    ClipboardService(const ClipboardService&) = delete;
    ClipboardService& operator=(const ClipboardService&) = delete;
    ClipboardService(ClipboardService&&) = delete;
    ClipboardService& operator=(ClipboardService&&) = delete;

public:
    static ClipboardService& getInstance() {
        static ClipboardService instance;
        return instance;
    }
    void type(std::string text);
    void hotKey(std::vector<std::string> keys);
    void keyPress(std::string Key);
};