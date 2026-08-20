#pragma once


#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#else
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
    #include <unistd.h>
#endif

std::map<std::string, std::string> getOsIinfo();