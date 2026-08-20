#include "Connection.hpp"

bool IsNetworkConnected() {
    #ifdef _WIN32
        DWORD flags;
        return InternetGetConnectedState(&flags, 0);

    #else 
        struct ifaddrs *ifaddr, *ifa;
        bool connected = false;

        if (getifaddrs(&ifaddr) == -1)
            return false;

        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr)
                continue;

            if (strcmp(ifa->ifa_name, "lo") == 0)
                continue;

            if ((ifa->ifa_flags & IFF_UP) && (ifa->ifa_flags & IFF_RUNNING)) {
                connected = true;
                break;
            }
        }

        freeifaddrs(ifaddr);
        return connected;
    
    #endif
}