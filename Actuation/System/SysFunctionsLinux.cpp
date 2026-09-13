#include "SysFunctionsLinux.hpp"

// Shut down function needs root access.
void SysfunctionsLinux::shutDown(int delay_minutes = 0){
    if (geteuid() != 0) {
        throw std::runtime_error("Need root access");
    }

    // building the delayed argument for safty.
    std::string delay_arg = (delay_minutes > 0)
        ? "+" + std::to_string(delay_minutes)
        : "now";

    pid_t pid = fork();
    if (pid < 0) {
        throw std::runtime_error(
            std::string("fork unsuccesful") + std::strerror(errno));
    }

    if (pid == 0) {
        // shutting dow without doing the child proccess.
        char* const args[] = {
            const_cast<char*>("/sbin/shutdown"),
            const_cast<char*>("-h"),
            const_cast<char*>(delay_arg.c_str()),
            nullptr
        };
        execv("/sbin/shutdown", args);

        // returning error.
        _exit(127);
    }

    // Waiting child proccess to end.
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        throw std::runtime_error(
            std::string("wait padd unsuccesful") + std::strerror(errno));
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        throw std::runtime_error(
            "Shut down with a warning code : " +
            std::to_string(WEXITSTATUS(status)));
    }
}
