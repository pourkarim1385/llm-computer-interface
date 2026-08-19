#include <iostream>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <X11/Xlib.h>
    #include <X11/extensions/scrnsaver.h>
#endif


static long long getIdleTimeMs();