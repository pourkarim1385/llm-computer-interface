#include "SearchService.h"
#include "SearchProvider.h"
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <cpr/cpr.h>

// --- OS-Specific Headers ---
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
    #include <signal.h>
    #include <sys/wait.h>
    #if defined(__linux__)
        #include <sys/prctl.h> // Required for Linux parent-death signal
    #endif
#endif

namespace WebSearch {
    class SidecarProcess {
    private:
#if defined(_WIN32)
        HANDLE hProcess_ = nullptr;
        HANDLE hJob_ = nullptr; // Windows Job Object for automatic cleanup
#else
        pid_t pid_ = -1;
#endif

    public:
        explicit SidecarProcess() {
#if defined(_WIN32)
            STARTUPINFOA si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // 1. Create a Job Object to prevent orphaned processes on Windows
            hJob_ = CreateJobObjectA(nullptr, nullptr);
            if (hJob_) {
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
                jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                SetInformationJobObject(hJob_, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
            }

            // 2. Safe mutable buffer for CreateProcessA
            //std::string cmd = "python ddg_server.py";
            std::string cmd = "ddg_server.exe";
            std::vector<char> cmd_buffer(cmd.begin(), cmd.end());
            cmd_buffer.push_back('\0');

            if (CreateProcessA(nullptr, cmd_buffer.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
                hProcess_ = pi.hProcess;
                CloseHandle(pi.hThread);

                // Assign the child process to the Job Object
                if (hJob_) {
                    AssignProcessToJobObject(hJob_, hProcess_);
                }
            } else {
                std::cerr << "[SearchService] Failed to start Python sidecar (Win32).\n";
            }
#else
            pid_ = fork();
            if (pid_ == 0) {
#if defined(__linux__)
                // Prevent orphaned process on Linux by tying child to parent's lifecycle
                prctl(PR_SET_PDEATHSIG, SIGTERM);
#endif
                freopen("/dev/null", "w", stdout);
                freopen("/dev/null", "w", stderr);

                execlp("./ddg_server", "ddg_server", nullptr);
                exit(1);
            } else if (pid_ < 0) {
                std::cerr << "[SearchService] Failed to fork Python sidecar process (POSIX).\n";
            }
#endif
        }

        ~SidecarProcess() {
#if defined(_WIN32)
            if (hProcess_) {
                TerminateProcess(hProcess_, 0);
                CloseHandle(hProcess_);
            }
            if (hJob_) {
                CloseHandle(hJob_);
            }
#else
            if (pid_ > 0) {
                kill(pid_, SIGTERM);
                waitpid(pid_, nullptr, 0);
            }
#endif
        }
    };

    struct SearchService::Impl {
        std::vector<std::unique_ptr<SearchProvider>> providers;
        std::unique_ptr<SidecarProcess> sidecar_process;

        explicit Impl(const SearchConfig& config) {
            if (!config.c_api_key.empty()) {
                providers.push_back(std::make_unique<TavilySearchProvider>(
                        config.c_api_key,
                        config.c_credit_limit
                ));
            }

            if (config.enable_ddg_fallback) {
                sidecar_process = std::make_unique<SidecarProcess>();

                // 3. Active Health Polling (Replaces fragile hardcoded sleep)
                bool is_ready = false;
                for (int i = 0; i < 50; ++i) { // Maximum 5-second timeout (50 * 100ms)
                    auto r = cpr::Get(cpr::Url{"http://127.0.0.1:8000/health"}, cpr::Timeout{100});
                    if (r.status_code == 200) {
                        is_ready = true;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                if (!is_ready) {
                    std::cerr << "[SearchService] Warning: Python sidecar failed to report healthy on port 8000.\n";
                }

                providers.push_back(std::make_unique<DuckDuckGoSearchProvider>(
                        config.ddg_sidecar_url,
                        UNLIMITED_CREDITS
                ));
            }
        }

        SearchResponse execute(const SearchRequest& request) {
            if (providers.empty()) {
                throw SearchException("SearchService misconfigured: No providers available.");
            }

            std::vector<std::pair<std::string, std::string>> failure_log;

            for (auto& provider : providers) {
                int64_t cost = (request.search_depth == "advanced") ? 2 : 1;

                if (!provider->hasCredits(cost)) {
                    failure_log.emplace_back(provider->getName(), "Insufficient credits.");
                    continue;
                }

                try {
                    return provider->search(request);
                }
                catch (const CreditExhaustedException& e) {
                    failure_log.emplace_back(provider->getName(), e.what());
                }
                catch (const NetworkException& e) {
                    failure_log.emplace_back(provider->getName(), e.what());
                }
                catch (const ParseException& e) {
                    failure_log.emplace_back(provider->getName(), e.what());
                }
                catch (const std::exception& e) {
                    failure_log.emplace_back(provider->getName(), std::string("Unexpected error: ") + e.what());
                }
            }

            throw AllProvidersFailedException(failure_log);
        }
    };

    SearchService::SearchService(const SearchConfig& config)
            : pimpl_(std::make_unique<Impl>(config)) {}

    SearchService::~SearchService() = default;
    SearchService::SearchService(SearchService&&) noexcept = default;
    SearchService& SearchService::operator=(SearchService&&) noexcept = default;

    SearchResponse SearchService::search(const std::string& query, int max_results) {
        SearchRequest req;
        req.query = query;
        req.max_results = max_results;
        return pimpl_->execute(req);
    }

    SearchResponse SearchService::search(const SearchRequest& request) {
        return pimpl_->execute(request);
    }

}