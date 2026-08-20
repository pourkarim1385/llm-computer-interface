#include "CPURAMWin.hpp"
#include <tlhelp32.h>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <psapi.h>

ProcessMonitor::ProcessMonitor() {}

ULONGLONG ProcessMonitor::fileTimeToU64(const FILETIME& ft) const {
    ULARGE_INTEGER ul;
    ul.LowPart  = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    return ul.QuadPart;
}

std::vector<ProcessInfo> ProcessMonitor::refresh() {
    // ---- 1. enumerate all PIDs ----
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return {};

    std::vector<ProcessInfo> results;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);

    // wall-clock "now" in 100-ns units
    FILETIME now_ft;
    GetSystemTimeAsFileTime(&now_ft);
    ULONGLONG now = fileTimeToU64(now_ft);

    DWORD numCPUs = 0;
    {
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        numCPUs = si.dwNumberOfProcessors;
        if (numCPUs < 1) numCPUs = 1;
    }

    if (!Process32FirstW(snap, &pe)) {
        CloseHandle(snap);
        return {};
    }

    do {
        DWORD pid = pe.th32ProcessID;
        if (pid == 0) continue;   

        HANDLE hProc = OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
            FALSE, pid);
        if (!hProc) continue;

        // ---- CPU times ----
        FILETIME cre, ex, kern, user;
        double cpuPct = 0.0;
        if (GetProcessTimes(hProc, &cre, &ex, &kern, &user)) {
            ULONGLONG k = fileTimeToU64(kern);
            ULONGLONG u = fileTimeToU64(user);

            auto it = prevTimes_.find(pid);
            if (it != prevTimes_.end()) {
                ULONGLONG dCPU  = (k + u) - (it->second.kernel + it->second.user);
                ULONGLONG dWall = now - it->second.wall;
                if (dWall > 0)
                    cpuPct = 100.0 * static_cast<double>(dCPU) / (static_cast<double>(dWall) * numCPUs);
            }
            prevTimes_[pid] = { k, u, now };
        }

        // ---- RAM (Working Set) ----
        PROCESS_MEMORY_COUNTERS pmc{};
        SIZE_T memBytes = 0;
        if (GetProcessMemoryInfo(hProc,
                reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                sizeof(pmc)))
            memBytes = pmc.WorkingSetSize;

        // ---- name ----
        char name[MAX_PATH]{};
        WideCharToMultiByte(CP_UTF8, 0,
            pe.szExeFile, -1,
            name, MAX_PATH, nullptr, nullptr);

        CloseHandle(hProc);

        // Explicitly cast DWORD to int to fix the narrowing conversion error
        // (Note: If your struct defines memory as 'int' instead of 'SIZE_T' or 'long long', 
        // you may also need to do static_cast<int>(memBytes) here.)
        results.push_back({ static_cast<int>(pid), name, cpuPct, memBytes });

    } while (Process32NextW(snap, &pe));

    CloseHandle(snap);

    // sort by CPU desc
    std::sort(results.begin(), results.end(),
        [](const ProcessInfo& a, const ProcessInfo& b){
            return a.cpuPercent > b.cpuPercent;
        });

    return results;
}
