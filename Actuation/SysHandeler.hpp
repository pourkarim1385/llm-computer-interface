#pragma once

#ifdef Win
    #include "System/SysFunctionWin.hpp"
#else
    #include "System/SysFunctionsLinux.hpp"
#endif


class SysHandeler
{
private:
    SysHandeler() = default;
    ~SysHandeler() = default;
    SysHandeler operator=(const SysHandeler& other) = delete;
    SysHandeler (const SysHandeler& other) = delete;
    SysHandeler operator=(const SysHandeler&& other) = delete;
    SysHandeler (const SysHandeler&& other) = delete;
    
public:
    SysHandeler& getInstance(){
        static SysHandeler instance;
        return instance;
    }
    void shutDown(int delayMinutes = 0);
    void restart(int delayMinutes = 0);
    void muteVolume();
    void unmuteVolume();
    void setVolume(float volume);
    KillResult killProcess(
    const std::string& name,
        int  sig          = SIGTERM,
        bool matchCmdline = false,
        bool waitForExit  = true,
        int  waitMs       = 2000
    );
    bool suspendSystem();
    bool launchProgram(const std::string& programName);
    bool closeApp(const std::string& processName, bool force = false);
};
