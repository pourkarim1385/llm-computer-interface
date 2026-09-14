#include "SysFunctionsLinux.hpp"

// Shut down function needs root access.
void SysfunctionsLinux::shutDown(int delay_minutes){
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

void SysfunctionsLinux::restart(int delayMinutes) {
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

void SysfunctionsLinux::setVolume(float volume) {
    if (volume < 0.0f || volume > 1.0f)
        throw std::invalid_argument("Volume must be in [0.0, 1.0]");

    char cmd[64];
    std::snprintf(cmd, sizeof(cmd),
        "wpctl set-volume @DEFAULT_AUDIO_SINK@ %.2f", volume);

    if (std::system(cmd) != 0)
        throw std::runtime_error("wpctl set-volume failed");
}

std::optional<std::string> SysfunctionsLinux::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return std::nullopt;
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Returns all PIDs whose /proc/<pid>/comm matches the given name exactly.
// If matchCmdline is true, also checks the full command line for a substring match.
std::vector<pid_t> SysfunctionsLinux::findPIDs(const std::string& name, bool matchCmdline) {
    std::vector<pid_t> result;

    DIR* dir = opendir("/proc");
    if (!dir) return result;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Only numeric directories
        const std::string dname(entry->d_name);
        if (dname.find_first_not_of("0123456789") != std::string::npos) continue;

        pid_t pid = static_cast<pid_t>(std::stoi(dname));
        if (pid == getpid()) continue; // skip self

        // Check /proc/<pid>/comm (process name, max 15 chars)
        auto comm = readFile("/proc/" + dname + "/comm");
        if (comm) {
            std::string trimmed = *comm;
            while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == '\r'))
                trimmed.pop_back();
            if (trimmed == name) {
                result.push_back(pid);
                continue;
            }
        }

        // Optional: match against full cmdline
        if (matchCmdline) {
            auto cmdline = readFile("/proc/" + dname + "/cmdline");
            if (cmdline && cmdline->find(name) != std::string::npos) {
                result.push_back(pid);
            }
        }
    }
    closedir(dir);
    return result;
}

KillResult SysfunctionsLinux::killProcess(
    const std::string& name,
    int  sig,
    bool matchCmdline,
    bool waitForExit,
    int  waitMs
) {
    KillResult res;

    auto pids = findPIDs(name, matchCmdline);
    if (pids.empty()) return res;

    res.found = true;

    for (pid_t pid : pids) {
        if (kill(pid, sig) == 0) {
            ++res.killed;
        } else {
            std::cerr << "[killProcess] kill(" << pid << ", " << sig
                      << ") failed: " << strerror(errno) << "\n";
            ++res.failed;
            continue;
        }

        // ── Grace period + escalation ─────────────────────────────────────
        if (waitForExit && sig == SIGTERM) {
            const int sleepUs  = 100'000; // 100 ms increments
            int       elapsed  = 0;

            while (elapsed < waitMs * 1000) {
                usleep(static_cast<useconds_t>(sleepUs));
                elapsed += sleepUs / 1000;

                // Process no longer exists → done
                if (kill(pid, 0) != 0 && errno == ESRCH) goto next_pid;
            }

            // Still alive → SIGKILL
            std::cerr << "[killProcess] PID " << pid
                      << " did not exit in " << waitMs
                      << " ms; escalating to SIGKILL\n";
            if (kill(pid, SIGKILL) != 0 && errno != ESRCH) {
                std::cerr << "[killProcess] SIGKILL(" << pid
                          << ") failed: " << strerror(errno) << "\n";
            }
            waitpid(pid, nullptr, WNOHANG);
        }
        next_pid:;
    }

    return res;
}

// To see the supported models cat /sys/power/state
bool SysfunctionsLinux::suspendSystem() {
    const std::string path = "/sys/power/state";

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open " << path
                  << " — " << std::strerror(errno) << "\n"
                  << "Make sure you are running as root.\n";
        return false;
    }

    file << "mem";

    if (file.fail()) {
        std::cerr << "Error: write to " << path
                  << " failed — " << std::strerror(errno) << "\n";
        return false;
    }

    file.close();
    // Execution resumes here after the system wakes up
    return true;
}

/**
 * @param programName path or the app name
 * @return true if the operation was successful
 */ 
// Thw name should be the operation name not the pacakge name
// for instance the operational name of the visual studio code is code :)
// This one should ne told to the llm.
bool SysfunctionsLinux::launchProgram(const std::string& programName) {
    if (programName.empty()) {
        std::cerr << "The program's name cannot be empty.\n";
        return false;
    }

    // Wrap in quotes to handle spaces in name/path, run in background
    std::string command = "\"" + programName + "\" &";

    int result = std::system(command.c_str());

    if (result == -1) {
        std::cerr << "Failed to launch '" << programName << "': "
                  << std::strerror(errno) << "\n";
        return false;
    }

    if (WEXITSTATUS(result) == 127) {
        std::cerr << "Program not found or could not be executed: '"
                  << programName << "'\n";
        return false;
    }

    return true;
}

bool SysfunctionsLinux::closeApp(const std::string& processName, bool force) {
    // finding process from PID.
    DIR* dir = opendir("/proc");
    if (!dir) {
        std::cerr << "Cannot open /proc\n";
        return false;
    }

    bool found = false;
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {

        std::string pidStr = entry->d_name;
        if (!std::all_of(pidStr.begin(), pidStr.end(), ::isdigit))
            continue;

        // reading process name from /proc/[pid]/comm
        std::string commPath = "/proc/" + pidStr + "/comm";
        std::ifstream commFile(commPath);
        if (!commFile.is_open())
            continue;

        std::string name;
        std::getline(commFile, name);

        if (name == processName) {
            pid_t pid = static_cast<pid_t>(std::stoi(pidStr));
            int sig = force ? SIGKILL : SIGTERM;

            if (kill(pid, sig) == 0) {
                std::cout << "Signal " << sig << " sent to PID " << pid
                          << " (" << name << ")\n";
                found = true;
            } else {
                perror(("kill failed for PID " + pidStr).c_str());
            }
        }
    }

    closedir(dir);

    if (!found)
        std::cerr << "Process '" << processName << "' not found.\n";

    return found;
}
