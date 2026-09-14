#include "SysHandeler.hpp"


void SysHandeler::shutDown(int delayMinutes){
    #ifdef Win
        SysFunctionWin instance;
        instance.shutDown(delayMinutes);
    #else
        SysfunctionsLinux instance;
        instance.shutDown(delayMinutes);
    #endif
}
void SysHandeler::restart(int delayMinutes){
    #ifdef Win
        SysFunctionWin instance;
        instance.restart(delayMinutes);
    #else
        SysfunctionsLinux instance;
        instance.restart(delayMinutes);
    #endif
}
void SysHandeler::muteVolume(){
    #ifdef Win
        SysFunctionWin instance;
        instance.muteVolume();
    #else
        SysfunctionsLinux instance;
        instance.muteVolume();
    #endif
}
void SysHandeler::unmuteVolume(){
    #ifdef Win
        SysFunctionWin instance;
        instance.unmuteVolume();
    #else
        SysfunctionsLinux instance;
        instance.unmuteVolume();
    #endif
}
void SysHandeler::setVolume(float volume){
    #ifdef Win
        SysFunctionWin instance;
        instance.setVolume(volume);
    #else
        SysfunctionsLinux instance;
        instance.setVolume(volume);
    #endif
}
KillResult SysHandeler::killProcess(
const std::string& name,
    int  sig,
    bool matchCmdline,
    bool waitForExit,
    int  waitMs
){
    #ifdef Win
        SysFunctionWin instance;
        instance.killProcess(name, sig, matchCmdline, waitForExit, waitMs);
    #else
        SysfunctionsLinux instance;
        instance.killProcess(name, sig, matchCmdline, waitForExit, waitMs);
    #endif
}
bool SysHandeler::suspendSystem(){
    #ifdef Win
        SysFunctionWin instance;
        instance.suspendSystem();
    #else
        SysfunctionsLinux instance;
        instance.suspendSystem();
    #endif
}
bool SysHandeler::launchProgram(const std::string& programName){
    #ifdef Win
        SysFunctionWin instance;
        instance.launchProgram(programName);
    #else
        SysfunctionsLinux instance;
        instance.launchProgram(programName);
    #endif
}
bool SysHandeler::closeApp(const std::string& processName, bool force){
    #ifdef Win
        SysFunctionWin instance;
        instance.closeApp(processName, force);
    #else
        SysfunctionsLinux instance;
        instance.closeApp(processName, force);
    #endif
}