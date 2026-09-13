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

void SysFunctionWin::shutdownWindows(DWORD delay_seconds = 0,
                      bool force = false,
                      const std::wstring& message = L"") {
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

void SysFunctionWin::restartWindows(DWORD         delay_seconds,
                                     bool          force,
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