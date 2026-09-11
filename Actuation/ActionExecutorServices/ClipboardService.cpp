#include "ClipboardService.hpp"

void ClipboardService::type(std::string text) {
#if defined(_WIN32) || defined(_WIN64)
    // Convert UTF-8 to UTF-16 (Windows API uses wide chars)
    int wlen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return;

    std::vector<wchar_t> wtext(wlen);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext.data(), wlen);

    // Each character needs a KEYDOWN + KEYUP event
    std::vector<INPUT> inputs;
    inputs.reserve((wlen - 1) * 2); // -1 to exclude null terminator

    for (int i = 0; i < wlen - 1; ++i) {
        INPUT inp_down = {};
        inp_down.type       = INPUT_KEYBOARD;
        inp_down.ki.dwFlags = KEYEVENTF_UNICODE;
        inp_down.ki.wScan   = wtext[i];
        inp_down.ki.wVk     = 0;

        INPUT inp_up = inp_down;
        inp_up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

        inputs.push_back(inp_down);
        inputs.push_back(inp_up);
    }

    if (!inputs.empty()) {
        SendInput(
            static_cast<UINT>(inputs.size()),
            inputs.data(),
            sizeof(INPUT)
        );
    }
#elif defined(__linux__) || defined(__unix__)
    pid_t pid = fork();
    if (pid == 0) {
        // child process
        execl("/usr/bin/xdotool", "xdotool", "type",
            "--clearmodifiers", "--delay", "20", "--", text.c_str(), nullptr);
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, nullptr, 0);
    }
#endif
}

void ClipboardService::hotKey(std::vector<std::string> keys) {
#if defined(_WIN32) || defined(_WIN64)
    if (keys.empty()) return;

    // Build INPUT array: KEYDOWN for all keys, then KEYUP in reverse
    std::vector<INPUT> inputs;
    inputs.reserve(keys.size() * 2);

    // Press phase (in order)
    for (const auto& key : keys) {
        WORD vk = 0;

        // Map common string names to Virtual Key codes
        if      (key == "ctrl"  || key == "control") vk = VK_CONTROL;
        else if (key == "shift")                     vk = VK_SHIFT;
        else if (key == "alt")                       vk = VK_MENU;
        else if (key == "win"   || key == "super")   vk = VK_LWIN;
        else if (key == "return"|| key == "enter")   vk = VK_RETURN;
        else if (key == "tab")                       vk = VK_TAB;
        else if (key == "esc"   || key == "escape")  vk = VK_ESCAPE;
        else if (key == "space")                     vk = VK_SPACE;
        else if (key == "backspace")                 vk = VK_BACK;
        else if (key == "delete")                    vk = VK_DELETE;
        else if (key == "home")                      vk = VK_HOME;
        else if (key == "end")                       vk = VK_END;
        else if (key == "up")                        vk = VK_UP;
        else if (key == "down")                      vk = VK_DOWN;
        else if (key == "left")                      vk = VK_LEFT;
        else if (key == "right")                     vk = VK_RIGHT;
        else if (key.size() >= 2 && (key[0] == 'f' || key[0] == 'F')) {
            // F1–F12
            try {
                int n = std::stoi(key.substr(1));
                if (n >= 1 && n <= 12) vk = static_cast<WORD>(VK_F1 + n - 1);
            } catch (...) {}
        }
        else if (key.size() == 1) {
            // Single character: a-z, 0-9, etc.
            vk = static_cast<WORD>(VkKeyScanA(key[0]) & 0xFF);
        }

        if (vk == 0) continue; // Unknown key — skip

        INPUT inp = {};
        inp.type       = INPUT_KEYBOARD;
        inp.ki.wVk     = vk;
        inp.ki.dwFlags = 0; // KEYDOWN
        inputs.push_back(inp);
    }

    // Release phase (reverse order)
    for (int i = static_cast<int>(keys.size()) - 1; i >= 0; --i) {
        if (i < static_cast<int>(inputs.size())) {
            INPUT inp   = inputs[i];
            inp.ki.dwFlags = KEYEVENTF_KEYUP;
            inputs.push_back(inp);
        }
    }

    if (!inputs.empty()) {
        SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    }
#elif defined(__linux__) || defined(__unix__)
    if (keys.empty()) return;

    std::string combo;
    for (std::size_t i = 0; i < keys.size(); ++i) {
        if (i > 0) combo += '+';
        combo += keys[i];
    }

    pid_t pid = fork();
    if (pid == 0) {
        execl("/usr/bin/xdotool", "xdotool", "key", "--clearmodifiers",
            combo.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
#endif
}

void ClipboardService::keyPress(std::string Key) {
#if defined(_WIN32) || defined(_WIN64)
    WORD vk = 0;

    if (Key == "ctrl")                               vk = VK_CONTROL;
    else if (Key == "shift")                         vk = VK_SHIFT;
    else if (Key == "alt")                           vk = VK_MENU;
    else if (Key == "return" || Key == "enter")      vk = VK_RETURN;
    else if (Key == "tab")                           vk = VK_TAB;
    else if (Key == "esc" || Key == "escape")        vk = VK_ESCAPE;
    else if (Key == "space")                         vk = VK_SPACE;
    else if (Key == "Left" || Key == "left")         vk = VK_LEFT;
    else if (Key == "Right" || Key == "right")       vk = VK_RIGHT;
    else if (Key == "Up" || Key == "up")             vk = VK_UP;
    else if (Key == "Down" || Key == "down")         vk = VK_DOWN;
    else if (Key == "Home" || Key == "home")         vk = VK_HOME;
    else if (Key == "End" || Key == "end")           vk = VK_END;
    else if (Key.size() >= 2 && (Key[0] == 'f' || Key[0] == 'F')) {
        try {
            int n = std::stoi(Key.substr(1));
            if (n >= 1 && n <= 12) vk = static_cast<WORD>(VK_F1 + n - 1);
        } catch (...) {}
    }
    else if (Key.size() == 1) {
        vk = static_cast<WORD>(VkKeyScanA(Key[0]) & 0xFF);
    }

    if (vk == 0) return;

    INPUT inputs[2] = {};

    inputs[0].type       = INPUT_KEYBOARD;
    inputs[0].ki.wVk     = vk;
    inputs[0].ki.dwFlags = 0;

    inputs[1].type       = INPUT_KEYBOARD;
    inputs[1].ki.wVk     = vk;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));

#elif defined(__linux__) || defined(__unix__)
    pid_t pid = fork();
    if (pid == 0) {
        execl("/usr/bin/xdotool", "xdotool", "key", Key.c_str(), (char*)nullptr);
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, nullptr, 0);
    }
#endif
}