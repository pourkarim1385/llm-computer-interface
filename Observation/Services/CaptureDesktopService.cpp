#include "CaptureDesktopService.hpp"


void DesktopService::get_system_info() {
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
    
    desktopState.setOsInfo(info);
}

void DesktopService::getActiveWindow(){
    #ifdef _WIN32
        #include <windows.h>
        #include <psapi.h>

        ActiveWindowInfo info{"", "", 0};

        HWND hwnd = GetForegroundWindow();
        if (!hwnd){
            desktopState.setActiveApp(info);
            return;
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

        desktopState.setActiveApp(info);
        

    #else

        #include <X11/Xlib.h>
        #include <X11/Xatom.h>
        #include <fstream>
        #include <cstring>

        ActiveWindowInfo info{"", "", 0};

        Display* display = XOpenDisplay(nullptr);
        if (!display){
            desktopState.setActiveApp(info);
            return;
        }

        Window root = DefaultRootWindow(display);
        Atom activeAtom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);

        Atom actualType;
        int actualFormat;
        unsigned long nItems, bytesAfter;
        unsigned char* data = nullptr;

        if (XGetWindowProperty(display, root, activeAtom, 0, 1, False,
                                AnyPropertyType, &actualType, &actualFormat,
                                &nItems, &bytesAfter, &data) == Success && data) {
            Window activeWindow = *(Window*)data;
            XFree(data);

            Atom nameAtom = XInternAtom(display, "_NET_WM_NAME", True);
            if (XGetWindowProperty(display, activeWindow, nameAtom, 0, 1024, False,
                                    AnyPropertyType, &actualType, &actualFormat,
                                    &nItems, &bytesAfter, &data) == Success && data) {
                info.title = std::string((char*)data);
                XFree(data);
            }

            Atom pidAtom = XInternAtom(display, "_NET_WM_PID", True);
            if (XGetWindowProperty(display, activeWindow, pidAtom, 0, 1, False,
                                    AnyPropertyType, &actualType, &actualFormat,
                                    &nItems, &bytesAfter, &data) == Success && data) {
                info.pid = *(unsigned long*)data;
                XFree(data);

                std::ifstream commFile("/proc/" + std::to_string(info.pid) + "/comm");
                if (commFile) {
                    std::getline(commFile, info.processName);
                }
            }
        }

        XCloseDisplay(display);
        desktopState.setActiveApp(info);

    #endif

}