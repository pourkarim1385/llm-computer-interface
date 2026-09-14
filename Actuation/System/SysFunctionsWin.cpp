#include "SysFunctionWin.hpp"

void SysFunctionWin::enableShutdownPrivilege() {
    HANDLE token = nullptr;

    // Step 1: open the access token of the current process
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &token)) {
        throw std::runtime_error(
            "OpenProcessToken failed: " + std::to_string(GetLastError()));
    }

    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Step 2: resolve the LUID for SeShutdownPrivilege
    if (!LookupPrivilegeValueW(nullptr,
                               SE_SHUTDOWN_NAME,          // L"SeShutdownPrivilege"
                               &tp.Privileges[0].Luid)) {
        DWORD err = GetLastError();
        CloseHandle(token);
        throw std::runtime_error(
            "LookupPrivilegeValueW failed: " + std::to_string(err));
    }

    // Step 3: enable the privilege inside the token
    if (!AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr)) {
        DWORD err = GetLastError();
        CloseHandle(token);
        throw std::runtime_error(
            "AdjustTokenPrivileges failed: " + std::to_string(err));
    }

    // AdjustTokenPrivileges can return TRUE even when no privilege was assigned
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        CloseHandle(token);
        throw std::runtime_error(
            "SeShutdownPrivilege could not be assigned — "
            "run the process as Administrator.");
    }

    CloseHandle(token);
}

void SysFunctionWin::shutdownWindows(DWORD delay_seconds,
                      bool force,
                      const std::wstring& message) {
    enable_shutdown_privilege();

    DWORD flags = SHUTDOWN_POWEROFF;
    if (force) {
        flags |= SHUTDOWN_FORCE_OTHERS | SHUTDOWN_FORCE_SELF;
    }

    // if it is empty => WCHAR* turns that into nullptr.
    const wchar_t* msg_ptr = message.empty() ? nullptr : message.c_str();

    if (!InitiateSystemShutdownExW(
            nullptr,
            const_cast<LPWSTR>(msg_ptr),
            delay_seconds,
            force ? TRUE : FALSE,   // ForceAppsClosed
            FALSE,                  // RebootAfterShutdown
            SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_FLAG_PLANNED)) {
        throw std::runtime_error(
            "InitiateSystemShutdownExW unsuccesfull with the warning code " +
            std::to_string(GetLastError()));
    }
}

static void SysFunctionWin::enableRestartPrivilege() {
    HANDLE token = nullptr;

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &token)) {
        throw std::runtime_error(
            "OpenProcessToken failed, error: " +
            std::to_string(GetLastError()));
    }

    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!LookupPrivilegeValueW(nullptr,
                               SE_SHUTDOWN_NAME,
                               &tp.Privileges[0].Luid)) {
        DWORD err = GetLastError();
        CloseHandle(token);
        throw std::runtime_error(
            "LookupPrivilegeValueW failed, error: " + std::to_string(err));
    }

    BOOL ok = AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr);
    DWORD err = GetLastError();  // Should be called before close handel.
    CloseHandle(token);

    if (!ok) {
        throw std::runtime_error(
            "AdjustTokenPrivileges failed, error: " + std::to_string(err));
    }

    if (err == ERROR_NOT_ALL_ASSIGNED) {
        throw std::runtime_error(
            "SeShutdownPrivilege could not be assigned — "
            "process must run as Administrator.");
    }
}

void SysFunctionWin::restartWindows(DWORD delay_seconds,
                                     bool force,
                                     const std::wstring& message) {
    enable_shutdown_privilege();

    const wchar_t* msg_ptr = message.empty() ? nullptr : message.c_str();

    BOOL result = InitiateSystemShutdownExW(
        nullptr,                                          // Local
        const_cast<LPWSTR>(msg_ptr),                      // Message to be shown.
        delay_seconds,                                    // Delay
        force ? TRUE : FALSE,                             // ForceAppsClosed
        TRUE,                                             // RebootAfterShutdown → restart
        SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_FLAG_PLANNED
    );

    if (!result) {
        throw std::runtime_error(
            "InitiateSystemShutdownExW (restart) failed, error: " +
            std::to_string(GetLastError()));
    }
}

void SysFunctionWin::mute() {
    // nircmd should be acceable - in the line where the .exe file exist - or in the system path.
    int ret = std::system("nircmd.exe mutesysvolume 1");
    if (ret != 0) throw std::runtime_error("nircmd mute failed");
}

void SysFunctionWin::unmute() {
    int ret = std::system("nircmd.exe mutesysvolume 0");
    if (ret != 0) throw std::runtime_error("nircmd unmute failed");
}

void SysFunctionWin::toggle() {
    std::system("nircmd.exe mutesysvolume 2");
}

static void SysFunctionWin::setVolume(float volume) {
    if (volume < 0.0f || volume > 1.0f)
        throw std::invalid_argument("Volume must be in [0.0, 1.0]");

    IAudioEndpointVolume* endpointVol = nullptr;

    // COM init (safe to call multiple times per thread)
    CoInitialize(nullptr);

    IMMDeviceEnumerator* enumerator = nullptr;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&enumerator
    );
    if (FAILED(hr)) throw std::runtime_error("CoCreateInstance failed");

    IMMDevice* device = nullptr;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    enumerator->Release();
    if (FAILED(hr)) throw std::runtime_error("GetDefaultAudioEndpoint failed");

    hr = device->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL, nullptr,
        (void**)&endpointVol
    );
    device->Release();
    if (FAILED(hr)) throw std::runtime_error("Activate failed");

    // RAII guard
    struct Guard {
        IAudioEndpointVolume* p;
        ~Guard() { if (p) p->Release(); }
    } guard{endpointVol};

    hr = endpointVol->SetMasterVolumeLevelScalar(volume, nullptr);
    if (FAILED(hr)) throw std::runtime_error("SetMasterVolumeLevelScalar failed");
}

std::vector<DWORD> SysFunctionWin::findPIDs(const std::string& name, bool matchSubstring) {
    std::vector<DWORD> result;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return result;

    PROCESSENTRY32 pe{};
    pe.dwSize = sizeof(pe);

    if (!Process32First(snap, &pe)) {
        CloseHandle(snap);
        return result;
    }

    do {
        std::string exeName(pe.szExeFile);

        bool match = matchSubstring
            ? (exeName.find(name) != std::string::npos)
            : (exeName == name);

        if (match && pe.th32ProcessID != GetCurrentProcessId())
            result.push_back(pe.th32ProcessID);

    } while (Process32Next(snap, &pe));

    CloseHandle(snap);
    return result;
}

bool SysFunctionWin::waitForExit(DWORD pid, DWORD timeoutMs) {
    HANDLE hProc = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (!hProc) return true; // already gone

    DWORD ret = WaitForSingleObject(hProc, timeoutMs);
    CloseHandle(hProc);
    return (ret == WAIT_OBJECT_0);
}

KillResult SysFunctionWin::killProcess(
    const std::string& name,
    bool matchSubstring,
    DWORD waitMs
) {
    KillResult res;

    auto pids = findPIDs(name, matchSubstring);
    res.found = static_cast<int>(pids.size());

    for (DWORD pid : pids) {ESS_TERMINATE | SYNCHRONIZE
        HANDL SYNCHRONIZE
        HANDLE hProc = OpenProcess(
            PROCESS_TERMINATE | SYNCHRONIZE,
            FALSE,
            pid
        );

        if (!hProc) {
            std::cerr << "[killProcess] OpenProcess(" << pid
                      << ") failed: " << GetLastError() << "\n";
            ++res.failed;
            continue;
        }

        BOOL ok = TerminateProcess(hProc, 1 /* exit code */);
        if (!ok) {
            std::cerr << "[killProcess] TerminateProcess(" << pid
                      << ") failed: " << GetLastError() << "\n";
            CloseHandle(hProc);
            ++res.failed;
            continue;
        }

        // Opme signalled (process full (process fully gone)
        if (waitMs > 0) {
            DWORD ret = WaitForSingleObject(hProc, waitMs);
            if (ret != WAIT_OBJECT_0) {
                std::cerr << "[killProcess] PID " << pid
                          << " did not exit within " << waitMs << " ms\n";
                CloseHandle(hProc);
                ++res.failed;
                continue;
            }
        }

        CloseHandle(hProc);
        ++res.killed;
    }

    return res;
}

// returns true if the operation was successful.
bool SysFunctionWin::suspendSystem() {
    const char* path = "/sys/power/state";

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Can not open the file" << path << "unsuccessbful"
                  << std::strerror(errno) << "\n"
                  << "Must be done with a root access\n";
        return false;
    }

    file << "mem";
    file.flush();

    if (file.fail()) {
        std::cerr << "Error in the writing" << path << " unsuccesful "
                  << std::strerror(errno) << "\n";
        return false;
    }

    return true;
}

// making string lower case for the command
std::string SysFunctionWin::toLower(const std::string& s) {
    std::string result = s;
    for (char& c : result)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

// Validation of the proccess. (The end of the proccess should not make problem in the os)
bool SysFunctionWin::isValidProcessName(const std::string& name) {
    static const std::regex pattern(R"(^[a-zA-Z0-9_\-]+\.exe$)");
    if (!std::regex_match(name, pattern)) {
        std::cerr << "[WARN] Invalid proccess name" << name << "\n";
        return false;
    }

    std::string lower = toLower(name);
    if (ALLOWED_PROCESS_NAMES.find(lower) == ALLOWED_PROCESS_NAMES.end()) {
        std::cerr << "[WARN] The proccess is not in the white list" << name << "\n";
        return false;
    }

    return true;
}

// Finding the appropirate PIDs.
std::vector<DWORD> SysFunctionWin::findRunningPIDs(const std::string& processName) {
    std::vector<DWORD> pids;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] CreateToolhelp32Snapshot unsuccessful code : "
                  << GetLastError() << "\n";
        return pids;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snapshot, &entry)) {
        do {
            // make a wide string from narrow string.
            char narrow[MAX_PATH] = {};
            WideCharToMultiByte(CP_UTF8, 0,
                entry.szExeFile, -1,
                narrow, sizeof(narrow),
                nullptr, nullptr);

            if (toLower(std::string(narrow)) == toLower(processName))
                pids.push_back(entry.th32ProcessID);

        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return pids;
}

bool SysFunctionWin::closeByPID(DWORD pid, bool force) {
    if (force) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) {
            std::cerr << "[ERROR] OpenProcess unsuccessful PID=" << pid
                      << " code : " << GetLastError() << "\n";
            return false;
        }

        BOOL ok = TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);

        if (!ok) {
            std::cerr << "[ERROR] TerminateProcess unsuccessful PID=" << pid
                      << " code : " << GetLastError() << "\n";
            return false;
        }

        return true;

    } else {
        // finding the target PID
        struct EnumData { DWORD pid; bool sent; };
        EnumData data = { pid, false };

        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            auto* d = reinterpret_cast<EnumData*>(lParam);
            DWORD windowPID = 0;
            GetWindowThreadProcessId(hwnd, &windowPID);
            if (windowPID == d->pid && IsWindowVisible(hwnd)) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                d->sent = true;
            }
            return TRUE; // continuing enumerations.
        }, reinterpret_cast<LPARAM>(&data));

        if (!data.sent)
            std::cerr << "[WARN] Any window with PID=" << pid << " not found.\n";

        return data.sent;
    }
}

/**
 *
 *
 * @param processName  process name
 * @param force       
 * @param timeoutMs   
 * @return            
 */
bool SysFunctionWin::closeApplication(const std::string& processName,
    bool force, DWORD timeoutMs)
{
    // 1- validation
    if (processName.empty()) {
        std::cerr << "[ERROR] The proccess name can not be empty\n";
        return false;
    }

    if (!isValidProcessName(processName)) {
        return false;
    }

    // 2- Finding runnig proccesses.
    std::vector<DWORD> pids = findRunningPIDs(processName);

    if (pids.empty()) {
        std::cout << "[INFO] procces with name" << processName
                  << " nit runnig \n";
        return true; // that is not a error.
    }

    std::cout << "[INFO] " << pids.size()
              << " sample of'" << processName << " found\n";

    bool allSuccess = true;

    for (DWORD pid : pids) {
        std::cout << "[INFO] closing PID = " << pid << " ...\n";

        if (!closeByPID(pid, force)) {
            allSuccess = false;
            continue;
        }

        // 3- Waiting to end the proccess
        HANDLE hWait = OpenProcess(SYNCHRONIZE, FALSE, pid);
        if (hWait) {
            DWORD waitResult = WaitForSingleObject(hWait, timeoutMs);
            CloseHandle(hWait);

            if (waitResult == WAIT_TIMEOUT) {
                std::cerr << "[WARN] procces PID=" << pid
                          << " after " << timeoutMs << "ms not ended.\n";
                allSuccess = false;
            } else {
                std::cout << "[INFO] PID=" << pid << " successfuly ended.\n";
            }
        }
    }
    return allSuccess;
}