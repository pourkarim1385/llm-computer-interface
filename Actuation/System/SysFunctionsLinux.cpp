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

void SysfunctionsLinux::restart(int delayMinutes = 0) {
    if (geteuid() != 0) {
        throw std::runtime_error("Need root access");
    }

    std::string delay_arg = (delayMinutes > 0)
        ? "+" + std::to_string(delayMinutes)
        : "now";

    pid_t pid = fork();
    if (pid < 0) {
        throw std::runtime_error(
            std::string("fork unsuccesful") + std::strerror(errno));
    }

    if (pid == 0) {
        // the onlu diff with the shudown is th falg (-r instead of the -h)
        char* const args[] = {
            const_cast<char*>("/sbin/shutdown"),
            const_cast<char*>("-r"),
            const_cast<char*>(delay_arg.c_str()),
            nullptr
        };
        execv("/sbin/shutdown", args);
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        throw std::runtime_error(
            std::string("wait padd unsuccesful") + std::strerror(errno));
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        throw std::runtime_error(
            "restart has done with a warning code : " +
            std::to_string(WEXITSTATUS(status)));
    }
}

// Using amixer to mute and unmute
void SysfunctionsLinux::muteVolume() {
    int ret = std::system("amixer set Master mute");
    if (ret != 0) {
        throw std::runtime_error("Failed to mute volume via amixer");
    }
}

void SysfunctionsLinux::unmuteVolume() {
    int ret = std::system("amixer set Master unmute");
    if (ret != 0) {
        throw std::runtime_error("Failed to unmute volume via amixer");
    }
}
