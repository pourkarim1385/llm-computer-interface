#pragma once

#include <stdexcept>
#include <string>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <tlhelp32.h>

struct KillResult {
    int  found  = 0;
    int  killed = 0;
    int  failed = 0;
};

class SysFunctionWin
{
private:
    SysFunctionWin() =default;
    ~SysFunctionWin() = default;
    SysFunctionWin operator=(const SysFunctionWin& other) = delete;
    SysFunctionWin (const SysFunctionWin& other) = delete;
    SysFunctionWin operator=(const SysFunctionWin&& other) = delete;
    SysFunctionWin (const SysFunctionWin&& other) = delete;

    std::vector<DWORD> findPIDs(const std::string& name, bool matchSubstring = false);
    bool waitForExit(DWORD pid, DWORD timeoutMs);
    KillResult killProcess(
        const std::string& name,
        bool matchSubstring = false,
        DWORD waitMs        = 3000
    )

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
    bool suspendSustem();
};

