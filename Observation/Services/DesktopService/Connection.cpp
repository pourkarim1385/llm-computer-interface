#include "Connection.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <vector>

bool IsNetworkConnected() {
    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = htonl(0x08080808); // 8.8.8.8 (Google DNS)

    DWORD bestIfIndex = 0;
    if (GetBestInterfaceEx(reinterpret_cast<sockaddr*>(&target), &bestIfIndex) == NO_ERROR) {
        return true;
    }

    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    ULONG bufLen = 15000;
    std::vector<BYTE> buffer(bufLen);
    PIP_ADAPTER_ADDRESSES addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

    if (GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses, &bufLen) == ERROR_SUCCESS) {
        for (auto curr = addresses; curr != nullptr; curr = curr->Next) {
            if (curr->IfType != IF_TYPE_SOFTWARE_LOOPBACK && curr->OperStatus == IfOperStatusUp) {
                return true;
            }
        }
    }
    return false;
}

#else
#include <ifaddrs.h>
#include <net/if.h>
#include <cstring>

bool IsNetworkConnected() {
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
}
#endif