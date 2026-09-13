#pragma once

#include <stdexcept>
#include <string>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

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
    void enableShutdownPrivilege();
    void shutdownWindows(DWORD delay_seconds = 0,
        bool force = false,const std::wstring& message = L"");
    static void enableRestartPrivilege();
    void restartWindows(DWORD delay_seconds,
        bool force,const std::wstring& message);
    void mute();
    void unmute();
    void toggle();
    void setVolume(float volume);
};

