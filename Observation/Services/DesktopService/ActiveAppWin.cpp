#include "ActiveAppWin.hpp"

ActiveWindowInfo getActiveApp(){
    ActiveWindowInfo info{"", "", 0};

    HWND hwnd = GetForegroundWindow();
    if (!hwnd){
        return info;
    }

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    info.title = title;

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    info.pid = pid;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        char processPath[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameA(hProcess, 0, processPath, &size)) {
            std::string fullPath(processPath);
            size_t pos = fullPath.find_last_of("\\/");
            info.processName = (pos != std::string::npos)
                ? fullPath.substr(pos + 1)
                : fullPath;
        }
        CloseHandle(hProcess);
    }

    return info;
}