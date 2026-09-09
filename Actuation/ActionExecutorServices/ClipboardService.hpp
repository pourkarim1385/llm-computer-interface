#pragma once

#ifdef __linux__
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <sys/wait.h>
#include <vector>


using namespace std;

class ClipboardService
{
private:
    ClipboardService() = default;
    ClipboardService(const ClipboardService&) = delete;
    ClipboardService& operator=(const ClipboardService&) = delete;
    ClipboardService(ClipboardService&&) = delete;
    ClipboardService& operator=(ClipboardService&&) = delete;

public:
    static ClipboardService& getInstance(){
        static ClipboardService instance;
        return instance;
    }
    void type(string text);
    void hotKey(std::vector<std::string> keys);
    void keyPress(string Key);
};
