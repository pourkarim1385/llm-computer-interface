#include "../SystemService.h"

#if defined (_WIN32)||(_WIN64)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define INITGUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <shlobj.h>
#include <KnownFolders.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <shellapi.h>
#include <algorithm>
#include <filesystem>
#include <regex>
#include <stdexcept>

#include "../SystemService.h"

SystemService& SystemService::getInstance() {
    static SystemService instance;
    return instance;
}

void SystemService::runCommand(const Actions::RunCmd& action) {
    CommandResult result = cmd.execute(action.command, terminalType::cmd);
    action.output = result.output;
    action.cycle = result.elapsedCycles;
    if (result.exitCode != 0 || result.timedOut) {
        throw std::runtime_error("Command execution failed or timed out.");
    }
}

void SystemService::runPowerShell(const Actions::RunPowerShell& action) {
    CommandResult result = cmd.execute(action.command, terminalType::shell);
    action.output = result.output;
    action.cycle = result.elapsedCycles;
    if (result.exitCode != 0 || result.timedOut) {
        throw std::runtime_error("PowerShell execution failed or timed out.");
    }
}

// App Management
std::vector<std::filesystem::path> getStartMenuDirectories() {
    std::vector<std::filesystem::path> dirs;
    PWSTR pPath = nullptr;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs, 0, NULL, &pPath))) {
        dirs.emplace_back(pPath);
        CoTaskMemFree(pPath);
    }

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, NULL, &pPath))) {
        dirs.emplace_back(pPath);
        CoTaskMemFree(pPath);
    }

    return dirs;
}

std::vector<std::filesystem::path> getCachedShortcuts() {
    static std::vector<std::filesystem::path> cachedShortcuts;
    static bool isCached = false;

    if (!isCached) {
        for (const auto& startDir : getStartMenuDirectories()) {
            if (!std::filesystem::exists(startDir)) continue;

            for (const auto& entry : std::filesystem::recursive_directory_iterator(
                startDir, std::filesystem::directory_options::skip_permission_denied)) {

                if (entry.is_regular_file() && entry.path().extension() == ".lnk") {
                    cachedShortcuts.push_back(entry.path());
                }
            }
        }
        isCached = true;
    }
    return cachedShortcuts;
}

std::string getExecutableFromShortcut(const std::filesystem::path& shortcutPath) {
    std::string targetExe;
    HRESULT hr = CoInitialize(NULL);
    bool cleanupCOM = SUCCEEDED(hr);

    IShellLinkW* psl;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl))) {
        IPersistFile* ppf;
        if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
            if (SUCCEEDED(ppf->Load(shortcutPath.c_str(), STGM_READ))) {
                WCHAR szPath[MAX_PATH];
                if (SUCCEEDED(psl->GetPath(szPath, MAX_PATH, NULL, SLGP_UNCPRIORITY))) {
                    std::filesystem::path target(szPath);
                    targetExe = target.filename().string();
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    if (cleanupCOM) CoUninitialize();
    return targetExe;
}

struct WindowSearch {
    std::string targetName;
    HWND foundHwnd = nullptr;
};

// Callback to search all open windows for a partial, case-insensitive title match
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    WindowSearch* search = reinterpret_cast<WindowSearch*>(lParam);
    char title[256];

    if (IsWindowVisible(hwnd) && GetWindowTextA(hwnd, title, sizeof(title))) {
        std::string windowTitle(title);
        std::string target = search->targetName;

        std::transform(windowTitle.begin(), windowTitle.end(), windowTitle.begin(), ::tolower);
        std::transform(target.begin(), target.end(), target.begin(), ::tolower);

        if (windowTitle.find(target) != std::string::npos) {
            search->foundHwnd = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

HWND findAppWindow(const std::string& name) {
    WindowSearch search{ name };
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&search));
    return search.foundHwnd;
}

void SystemService::openApp(const std::string& name) {
    try {
        std::regex regPattern(name, std::regex_constants::icase);

        for (const auto& shortcutPath : getCachedShortcuts()) {
            std::string shortcutName = shortcutPath.stem().string();

            if (std::regex_search(shortcutName, regPattern)) {
                HINSTANCE hInst = ShellExecuteW(
                    NULL, L"open", shortcutPath.c_str(), NULL, NULL, SW_SHOWNORMAL
                );

                if ((INT_PTR)hInst > 32) {
                    return;
                }
            }
        }
    }
    catch (const std::regex_error&) {
    }

    HINSTANCE hInst = ShellExecuteA(NULL, "open", name.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)hInst > 32) {
        return;
    }

    std::string fallbackCmd = "start \"\" \"" + name + "\"";
    CommandResult result = cmd.execute(fallbackCmd, terminalType::cmd);

    if (result.exitCode != 0) {
        throw std::runtime_error("Failed to open application: " + name);
    }
}

void SystemService::closeApp(const std::string& name) {
    std::string trimmedName = name;
    trimmedName.erase(trimmedName.find_last_not_of(" \n\r\t") + 1);

    bool killedSomething = false;

    auto checkSuccess = [](const CommandResult& res) {
        if (res.exitCode == 0) return true;
        std::string out = res.output;
        std::transform(out.begin(), out.end(), out.begin(), ::toupper);
        return out.find("SUCCESS") != std::string::npos;
        };

    bool hasExe = (trimmedName.length() >= 4 &&
        trimmedName.compare(trimmedName.length() - 4, 4, ".exe") == 0);

    if (hasExe) {
        std::string command = "taskkill /IM \"" + trimmedName + "\" /F /T";
        if (checkSuccess(cmd.execute(command, terminalType::cmd))) return;
    }

    try {
        std::regex regPattern(trimmedName, std::regex_constants::icase);
        for (const auto& shortcutPath : getCachedShortcuts()) {
            std::string shortcutName = shortcutPath.stem().string();
            if (std::regex_search(shortcutName, regPattern)) {
                std::string exactExe = getExecutableFromShortcut(shortcutPath);
                if (!exactExe.empty()) {
                    std::string command = "taskkill /IM \"" + exactExe + "\" /F /T";
                    if (checkSuccess(cmd.execute(command, terminalType::cmd))) {
                        killedSomething = true;
                    }
                }
            }
        }
    }
    catch (const std::regex_error&) {}

    if (!killedSomething && !hasExe) {
        std::string command = "taskkill /IM \"*" + trimmedName + "*.exe\" /F /T";
        if (checkSuccess(cmd.execute(command, terminalType::cmd))) {
            killedSomething = true;
        }
    }

    if (!killedSomething) {
        std::string command = "taskkill /FI \"WINDOWTITLE eq " + trimmedName + "\" /F /T";
        if (checkSuccess(cmd.execute(command, terminalType::cmd))) {
            killedSomething = true;
        }
    }

    if (!killedSomething) {
        throw std::runtime_error("Failed to close application: " + name);
    }
}

// Window Management Helpers
// Window Management

void SystemService::focusWindow(const std::string& name) {
    HWND hwnd = findAppWindow(name);
    if (!hwnd) throw std::runtime_error("Could not find window to focus: " + name);

    if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
    SetForegroundWindow(hwnd);
}

void SystemService::minimizeWindow(const std::string& name) {
    HWND hwnd = findAppWindow(name);
    if (!hwnd) throw std::runtime_error("Could not find window to minimize: " + name);

    ShowWindow(hwnd, SW_MINIMIZE);
}

void SystemService::maximizeWindow(const std::string& name) {
    HWND hwnd = findAppWindow(name);
    if (!hwnd) throw std::runtime_error("Could not find window to maximize: " + name);

    ShowWindow(hwnd, SW_MAXIMIZE);
}

void SystemService::restoreWindow(const std::string& name) {
    HWND hwnd = findAppWindow(name);
    if (!hwnd) throw std::runtime_error("Could not find window to restore: " + name);

    ShowWindow(hwnd, SW_RESTORE);
}

// Audio Management (Core Audio API)

bool manipulateAudio(float volumeLevel, bool setMuteState, bool muteValue) {
    HRESULT hr = CoInitialize(NULL);
    bool cleanupCOM = SUCCEEDED(hr);

    IMMDeviceEnumerator* deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);

    if (SUCCEEDED(hr)) {
        IMMDevice* defaultDevice = NULL;
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
        if (SUCCEEDED(hr)) {
            IAudioEndpointVolume* endpointVolume = NULL;
            hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
            if (SUCCEEDED(hr)) {
                if (setMuteState) {
                    endpointVolume->SetMute(muteValue, NULL);
                }
                else {
                    float clampedVol = (std::max)(0.0f, (std::min)(1.0f, volumeLevel));
                    endpointVolume->SetMasterVolumeLevelScalar(clampedVol, NULL);
                }
                endpointVolume->Release();
            }
            defaultDevice->Release();
        }
        deviceEnumerator->Release();
    }

    if (cleanupCOM) CoUninitialize();
    return SUCCEEDED(hr);
}

void SystemService::setVolume(int value) {
    float floatVol = static_cast<float>(value) / 100.0f;
    if (!manipulateAudio(floatVol, false, false)) {
        throw std::runtime_error("Failed to set system volume.");
    }
}

void SystemService::muteVolume() {
    if (!manipulateAudio(0.0f, true, true)) {
        throw std::runtime_error("Failed to mute system volume.");
    }
}

void SystemService::unmuteVolume() {
    if (!manipulateAudio(0.0f, true, false)) {
        throw std::runtime_error("Failed to unmute system volume.");
    }
}

// Power Management

void SystemService::sleep() {
    cmd.execute("rundll32.exe powrprof.dll,SetSuspendState 0,1,0", terminalType::cmd);
}

void SystemService::shutdown() {
    cmd.execute("shutdown /s /t 0", terminalType::cmd);
}

void SystemService::restart() {
    cmd.execute("shutdown /r /t 0", terminalType::cmd);
}

#endif