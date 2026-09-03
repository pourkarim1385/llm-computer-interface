#pragma once

#include <string>
#include <atomic>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#undef CreateFile
#undef DeleteFile
#undef CopyFile
#undef MoveFile
#undef FAR
#endif

enum class terminalType {
    shell, cmd
};

// Struct to give the Agent precise execution details
struct CommandResult {
    std::string output;
    int exitCode;
    bool timedOut;
    size_t elapsedCycles;
};

class AsyncCommand {
public:
    AsyncCommand();
    ~AsyncCommand();

    size_t max_timeout_cycles = 10;

    CommandResult execute(const std::string& command, terminalType type);

    // Forcefully kills the process if it is currently running
    void kill();

private:
    // OS-specific implementations
    CommandResult execute_WIN(const std::string& command, terminalType type);
    CommandResult execute_Linux(const std::string& command);

    std::mutex m_execMutex;

#ifdef _WIN32
    std::atomic<HANDLE> m_activeProcess;
#else
    std::atomic<pid_t> m_activePid;
#endif
};