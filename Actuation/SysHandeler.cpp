#include "SysHandeler.hpp"


void SysHandeler::shutDown(int delayMinutes = 0){

}
void SysHandeler::restart(int delayMinutes = 0){

}
void SysHandeler::muteVolume(){

}
void SysHandeler::unmuteVolume(){

}
void SysHandeler::setVolume(float volume){

}
KillResult SysHandeler::killProcess(
const std::string& name,
    int  sig          = SIGTERM,
    bool matchCmdline = false,
    bool waitForExit  = true,
    int  waitMs       = 2000
){

}
bool SysHandeler::suspendSystem(){

}
bool SysHandeler::launchProgram(const std::string& programName){

}
bool SysHandeler::closeApp(const std::string& processName, bool force = false){

}