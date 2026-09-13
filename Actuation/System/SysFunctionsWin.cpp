#include "SysFunctionWin.hpp"

void SysFunctionWin::shutdown_windows(DWORD delay_seconds = 0,
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