#pragma once

#include <stdexcept>
#include <string>
#include <windows.h>

class SysFunctionWin
{
private:
    SysFunctionWin() =default;
    ~SysFunctionWin() = default;
    SysFunctionWin operator=(const SysFunctionWin& other) = delete;
    SysFunctionWin (const SysFunctionWin& other) = delete;
    SysFunctionWin operator=(const SysFunctionWin&& other) = delete;
    SysFunctionWin (const SysFunctionWin&& other) = delete;
public:
    SysFunctionWin& getImstance(){
        static SysFunctionWin instance;
        return instance;
    } 
    void shutdown_windows(DWORD delay_seconds = 0,
                      bool force = false,
                      const std::wstring& message = L"");
};

