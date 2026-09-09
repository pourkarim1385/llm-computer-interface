#pragma once

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
};
