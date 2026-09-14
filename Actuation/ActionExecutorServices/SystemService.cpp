#include "SystemService.h"

#if defined (_WIN32)||(_WIN64)
#include "SystemServices/SystemServiceWin.cpp"
#elif defined(__linux__)
#include "SystemServices/SystemServiceLinux.cpp"
#else
#error "Unsupported operating system"
#endif