#include "CPURAMLinux.hpp"

namespace fs = std::filesystem;

namespace {

struct ProcSnapshot {
    std::uint64_t ticks = 0;
    std::chrono::steady_clock::time_point last_sample{};
};

long read_sysconf_long(int name, long fallback) {
    errno = 0;
    long value = sysconf(name);
    if (value <= 0 || errno != 0) return fallback;
    return value;
}

std::string trim_copy(const std::string& s) {
    const auto begin = s.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos) return {};
    const auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(begin, end - begin + 1);
}

bool read_status_name_and_rss(int pid, std::string& name, std::uint64_t& rss_kb) {
    const std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream file(path);
    if (!file) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("Name:", 0) == 0) {
            name = trim_copy(line.substr(5));
        } else if (line.rfind("VmRSS:", 0) == 0) {
            std::istringstream iss(line.substr(6));
            iss >> rss_kb;
        }
    }
    if (name.empty()) name = "unknown";
    return true;
}

bool read_stat_ticks(int pid, std::uint64_t& ticks) {
    const std::string path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(path);
    if (!file) return false;

    std::string content;
    std::getline(file, content);
    if (content.empty()) return false;

    const auto rparen = content.rfind(')');
    if (rparen == std::string::npos) return false;

    std::string after = content.substr(rparen + 2);
    std::istringstream iss(after);

    std::string state;
    unsigned long ppid = 0, pgrp = 0, session = 0, tty_nr = 0, tpgid = 0;
    unsigned long flags = 0, minflt = 0, cminflt = 0, majflt = 0, cmajflt = 0;
    unsigned long long utime = 0, stime = 0;

    iss >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags
        >> minflt >> cminflt >> majflt >> cmajflt
        >> utime >> stime;
    if (!iss) return false;

    ticks = utime + stime;
    return true;
}

} // namespace

struct ProcessMonitor::Impl {
    std::map<int, ProcSnapshot> previous;
    long clock_ticks = read_sysconf_long(_SC_CLK_TCK, 100);
    long cpu_count = read_sysconf_long(_SC_NPROCESSORS_ONLN, 1);
};

ProcessMonitor::ProcessMonitor() : impl_(new Impl()) {}
ProcessMonitor::~ProcessMonitor() { delete impl_; }

std::vector<ProcessInfo> ProcessMonitor::refresh() {
    std::vector<ProcessInfo> result;

    const auto now = std::chrono::steady_clock::now();
    const std::string proc_root = "/proc";

    for (const auto& entry : fs::directory_iterator(proc_root)) {
        if (!entry.is_directory()) continue;
        const std::string filename = entry.path().filename().string();
        if (filename.empty() || !std::all_of(filename.begin(), filename.end(), ::isdigit)) continue;

        const int pid = std::stoi(filename);
        std::string name;
        std::uint64_t rss_kb = 0;
        std::uint64_t ticks = 0;

        if (!read_status_name_and_rss(pid, name, rss_kb)) continue;
        if (!read_stat_ticks(pid, ticks)) continue;

        double cpu_percent = 0.0;
        auto& prev = impl_->previous[pid];
        if (prev.last_sample.time_since_epoch().count() != 0) {
            const auto elapsed = std::chrono::duration<double>(now - prev.last_sample).count();
            const std::uint64_t delta_ticks = (ticks >= prev.ticks) ? (ticks - prev.ticks) : 0;
            const double seconds = static_cast<double>(delta_ticks) / static_cast<double>(impl_->clock_ticks);
            if (elapsed > 0.0 && impl_->cpu_count > 0) {
                cpu_percent = (seconds / elapsed) * 100.0 / static_cast<double>(impl_->cpu_count);
            }
        }

        prev.ticks = ticks;
        prev.last_sample = now;

        ProcessInfo info;
        info.pid = pid;
        info.name = std::move(name);
        info.cpu_percent = cpu_percent;
        info.ram_kb = rss_kb;
        result.push_back(std::move(info));
    }

    // Remove stale entries for exited processes.
    std::map<int, ProcSnapshot> alive;
    for (const auto& p : result) {
        alive[p.pid] = impl_->previous[p.pid];
    }
    impl_->previous.swap(alive);

    return result;
}
