#include "BaseOsInfo.hpp"

std::map<std::string, std::string> getOsIinfo() {
    std::map<std::string, std::string> info;

    #ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        info["cpu_cores"] = std::to_string(si.dwNumberOfProcessors);

        MEMORYSTATUSEX mem{};
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);
        info["ram_total_gb"] = std::to_string(mem.ullTotalPhys / 1073741824.0).substr(0, 5);
        info["ram_used_percent"] = std::to_string(mem.dwMemoryLoad);

        char hostname[256];
        DWORD size = sizeof(hostname);
        GetComputerNameA(hostname, &size);
        info["hostname"] = hostname;

        info["os"] = "Windows";
    #else
        struct utsname u;
        uname(&u);
        info["os"] = u.sysname;
        info["os_version"] = u.release;
        info["architecture"] = u.machine;
        info["hostname"] = u.nodename;

        struct sysinfo si;
        sysinfo(&si);
        info["ram_total_gb"] = std::to_string(si.totalram * si.mem_unit / 1073741824.0).substr(0, 5);
        info["ram_used_gb"] = std::to_string((si.totalram - si.freeram) * si.mem_unit / 1073741824.0).substr(0, 5);
        info["cpu_cores"] = std::to_string(sysconf(_SC_NPROCESSORS_ONLN));
        info["pid"] = std::to_string(getpid());
    #endif
        
    return info;
}