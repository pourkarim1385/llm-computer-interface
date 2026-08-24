#include "AsyncCommand.h"
#include <vector>
#include <future>
#include <chrono>

#ifdef _WIN32
// windows.h already included
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#endif

AsyncCommand::AsyncCommand() {
#ifdef _WIN32
    m_activeProcess = nullptr;
#else
    m_activePid = -1;
#endif
}

AsyncCommand::~AsyncCommand() {
    kill();
}

CommandResult AsyncCommand::execute(const std::string& command, terminalType type) {
    std::lock_guard<std::mutex> lock(m_execMutex);

    std::future<CommandResult> futureResult = std::async(std::launch::async, [this, command, type]() {
#ifdef _WIN32
        return execute_WIN(command, type);
#else
        return execute_Linux(command);
#endif
    });

    if (max_timeout_cycles == 0) {
        CommandResult res = futureResult.get();
        res.elapsedCycles = 0;
        return res;
    }

    size_t current_cycle = 0;
    const auto cycle_duration = std::chrono::milliseconds(500);

    while (futureResult.wait_for(cycle_duration) != std::future_status::ready) {
        current_cycle++;
        if (current_cycle >= max_timeout_cycles) {
            kill();
            CommandResult res = futureResult.get();
            res.timedOut = true;
            res.output += "\n[System: Process Terminated due to Timeout]";
            res.elapsedCycles = current_cycle;
            return res;
        }
    }

    CommandResult finalRes = futureResult.get();
    finalRes.elapsedCycles = current_cycle;
    return finalRes;
}

void AsyncCommand::kill() {
#ifdef _WIN32
    HANDLE hProc = m_activeProcess.load();
    if (hProc != nullptr) {
        TerminateProcess(hProc, 1);
    }
#else
    pid_t pid = m_activePid.load();
    if (pid > 0) {
        ::kill(pid, SIGKILL);
    }
#endif
}

CommandResult AsyncCommand::execute_WIN(const std::string& command, terminalType type) {
    CommandResult result{"", -1, false, 0};
#ifdef _WIN32
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        result.output = "Error: Pipe creation failed.";
        return result;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hWritePipe;
    si.hStdOutput = hWritePipe;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::string fullCmd;
    if(type == terminalType::cmd) {
        fullCmd = "cmd.exe /c " + command;
    } else {
        fullCmd = "powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command \"" + command + "\"";
    }

    std::vector<char> cmdBuffer(fullCmd.begin(), fullCmd.end());
    cmdBuffer.push_back('\0');

    if (!CreateProcessA(NULL, cmdBuffer.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
        result.output = "Error: CreateProcess failed.";
        return result;
    }

    m_activeProcess = pi.hProcess;

    CloseHandle(hWritePipe);
    CloseHandle(pi.hThread);

    char buffer[128];
    DWORD bytesRead;
    while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result.output += buffer;
    }

    CloseHandle(hReadPipe);
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    result.exitCode = static_cast<int>(exitCode);

    m_activeProcess = nullptr;
    CloseHandle(pi.hProcess);

    return result;
#else
    return result;
#endif
}

CommandResult AsyncCommand::execute_Linux(const std::string& command) {
    CommandResult result{"", -1, false, 0};
#ifndef _WIN32
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        result.output = "Error: Pipe failed.";
        return result;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(1);
    }

    m_activePid = pid;
    close(pipefd[1]);

    char buffer[128];
    ssize_t bytesRead;
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        result.output += buffer;
    }

    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result.exitCode = 128 + WTERMSIG(status); // Standard mapping for killed signals
    }

    m_activePid = -1;
    return result;
#else
    return result;
#endif
}