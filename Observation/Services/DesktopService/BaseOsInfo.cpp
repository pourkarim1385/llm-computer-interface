#include "BaseOsInfo.hpp"
#include <iomanip>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
    #include <unistd.h>
#endif

std::map<std::string, std::string> getOsIinfo() {
    std::map<std::string, std::string> info;

    #ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        info["cpu_cores"] = std::to_string(si.dwNumberOfProcessors);

        MEMORYSTATUSEX mem{};
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);

        double totalRamGb = static_cast<double>(mem.ullTotalPhys) / 1073741824.0;
        double usedRamGb = static_cast<double>(mem.ullTotalPhys - mem.ullAvailPhys) / 1073741824.0;

        std::ostringstream ssTotal, ssUsed;
        ssTotal << std::fixed << std::setprecision(2) << totalRamGb;
        ssUsed << std::fixed << std::setprecision(2) << usedRamGb;

        info["ram_total_gb"] = ssTotal.str();
        info["ram_used_gb"] = ssUsed.str();
        info["ram_used_percent"] = std::to_string(mem.dwMemoryLoad);

        char hostname[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(hostname);
        if (GetComputerNameA(hostname, &size)) {
            info["hostname"] = hostname;
        } else {
            info["hostname"] = "localhost";
        }

        info["os"] = "Windows";
        info["pid"] = std::to_string(GetCurrentProcessId());

    #else
        struct utsname u;
        if (uname(&u) == 0) {
            info["os"] = u.sysname;
            info["os_version"] = u.release;
            info["architecture"] = u.machine;
            info["hostname"] = u.nodename;
        }

        struct sysinfo si;
        if (sysinfo(&si) == 0) {
            double memUnit = static_cast<double>(si.mem_unit);
            double totalRamGb = (static_cast<double>(si.totalram) * memUnit) / 1073741824.0;
            double usedRamGb = (static_cast<double>(si.totalram - si.freeram) * memUnit) / 1073741824.0;

            std::ostringstream ssTotal, ssUsed;
            ssTotal << std::fixed << std::setprecision(2) << totalRamGb;
            ssUsed << std::fixed << std::setprecision(2) << usedRamGb;

            info["ram_total_gb"] = ssTotal.str();
            info["ram_used_gb"] = ssUsed.str();
            info["ram_used_percent"] = std::to_string(static_cast<int>((usedRamGb / totalRamGb) * 100.0));
        }

        info["cpu_cores"] = std::to_string(sysconf(_SC_NPROCESSORS_ONLN));
        info["pid"] = std::to_string(getpid());
    #endif

    return info;
}