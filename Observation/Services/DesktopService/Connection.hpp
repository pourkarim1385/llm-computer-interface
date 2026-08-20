#include <iostream>
#include <string>

#ifdef _WIN32
    #include <windows.h>
    #include <wininet.h>
    #pragma comment(lib, "Wininet.lib")
#else
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <cstring>
#endif

bool IsNetworkConnected();