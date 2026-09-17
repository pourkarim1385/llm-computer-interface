#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <string>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
#endif

// Observation Headers
#include "Observation/Services/WorldStateBuilderService.h"
#include "Observation/Models/WorldState.h"

using Clock = std::chrono::steady_clock;

struct BenchmarkResult {
    std::string testName;
    int iterations;
    double meanMs;
    double p50Ms;
    double p95Ms;
    double p99Ms;
    double minMs;
    double maxMs;
    double totalMs;
};

BenchmarkResult runBenchmark(const std::string& name, int iterations, int warmup, const std::function<void()>& op) {
    // 1. Warm-up
    for (int i = 0; i < warmup; ++i) {
        op();
    }

    std::vector<double> durations;
    durations.reserve(iterations);

    auto totalStart = Clock::now();

    // 2. Main Measurement Loop
    for (int i = 0; i < iterations; ++i) {
        auto start = Clock::now();
        op();
        auto end = Clock::now();

        double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
        durations.push_back(elapsed);
    }

    auto totalEnd = Clock::now();
    double totalTime = std::chrono::duration<double, std::milli>(totalEnd - totalStart).count();

    // 3. Percentiles and Summary
    std::sort(durations.begin(), durations.end());
    double sum = std::accumulate(durations.begin(), durations.end(), 0.0);
    double mean = sum / iterations;

    auto getPercentile = [&](double p) {
        size_t idx = static_cast<size_t>(p * (durations.size() - 1));
        return durations[idx];
    };

    return BenchmarkResult{
        name,
        iterations,
        mean,
        getPercentile(0.50),
        getPercentile(0.95),
        getPercentile(0.99),
        durations.front(),
        durations.back(),
        totalTime
    };
}

void printResultTable(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n" << std::string(105, '=') << "\n";
    std::cout << "| " << std::left << std::setw(38) << "Benchmark Workload"
              << "| " << std::setw(6) << "Iters"
              << "| " << std::setw(9) << "Mean(ms)"
              << "| " << std::setw(8) << "p50(ms)"
              << "| " << std::setw(8) << "p95(ms)"
              << "| " << std::setw(8) << "p99(ms)"
              << "| " << std::setw(8) << "Min(ms)"
              << "| " << std::setw(8) << "Max(ms)" << " |\n";
    std::cout << std::string(105, '-') << "\n";

    for (const auto& r : results) {
        std::cout << "| " << std::left << std::setw(38) << r.testName
                  << "| " << std::setw(6) << r.iterations
                  << "| " << std::fixed << std::setprecision(3) << std::setw(9) << r.meanMs
                  << "| " << std::setw(8) << r.p50Ms
                  << "| " << std::setw(8) << r.p95Ms
                  << "| " << std::setw(8) << r.p99Ms
                  << "| " << std::setw(8) << r.minMs
                  << "| " << std::setw(8) << r.maxMs << " |\n";
    }
    std::cout << std::string(105, '=') << "\n\n";
}

int main() {
#ifdef _WIN32
    // برای پایداری سرویس‌های UI Automation و Clipboard در تست مستقل
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    std::cout << "====================================================\n";
    std::cout << "    Observation Pipeline Performance Benchmark      \n";
    std::cout << "====================================================\n";

    auto& builder = WorldStateBuilderService::getInstance();
    std::vector<BenchmarkResult> results;

    constexpr int OS_ITERS = 50;
    constexpr int OS_WARMUP = 5;

    // -------------------------------------------------------------
    // فاز ۱: تست ایزوله تک‌تک سرویس‌ها
    // -------------------------------------------------------------
    std::cout << "\n[1/3] Benchmarking Isolated Subservices...\n";

    // 1.1 Vision (Screenshot capture)
    ObservationFlags visionFlags{};
    visionFlags.captureVision = true;
    visionFlags.captureFullAccessibility = false;
    visionFlags.captureActiveWindowAccessibility = false;
    visionFlags.captureClipboard = false;
    visionFlags.captureDesktop = false;
    visionFlags.captureNewScreenMetrics = false;

    std::cout << " -> Running Vision (Screenshot)...\n";
    results.push_back(runBenchmark("Subservice: Vision (Screenshot)", OS_ITERS, OS_WARMUP, [&]() {
        builder.observe(visionFlags);
        builder.clearState();
    }));

    // 1.2 Active Window Accessibility
    ObservationFlags activeA11yFlags{};
    activeA11yFlags.captureVision = false;
    activeA11yFlags.captureFullAccessibility = false;
    activeA11yFlags.captureActiveWindowAccessibility = true;
    activeA11yFlags.captureClipboard = false;
    activeA11yFlags.captureDesktop = false;

    std::cout << " -> Running Active Window Accessibility...\n";
    results.push_back(runBenchmark("Subservice: Active Window A11y", OS_ITERS, OS_WARMUP, [&]() {
        builder.observe(activeA11yFlags);
        builder.clearState();
    }));

    // 1.3 Full Desktop Accessibility Tree
    ObservationFlags fullA11yFlags{};
    fullA11yFlags.captureVision = false;
    fullA11yFlags.captureFullAccessibility = true;
    fullA11yFlags.captureActiveWindowAccessibility = false;
    fullA11yFlags.captureClipboard = false;
    fullA11yFlags.captureDesktop = false;

    std::cout << " -> Running Full Accessibility Tree (UIA)...\n";
    results.push_back(runBenchmark("Subservice: Full Desktop A11y Tree", OS_ITERS, OS_WARMUP, [&]() {
        builder.observe(fullA11yFlags);
        builder.clearState();
    }));

    // 1.4 Clipboard
    ObservationFlags clipFlags{};
    clipFlags.captureVision = false;
    clipFlags.captureFullAccessibility = false;
    clipFlags.captureClipboard = true;
    clipFlags.captureDesktop = false;

    std::cout << " -> Running Clipboard Service...\n";
    results.push_back(runBenchmark("Subservice: Clipboard Capture", 100, 10, [&]() {
        builder.observe(clipFlags);
        builder.clearState();
    }));

    // 1.5 Desktop State (OS/RAM/Processes)
    ObservationFlags desktopFlags{};
    desktopFlags.captureVision = false;
    desktopFlags.captureFullAccessibility = false;
    desktopFlags.captureClipboard = false;
    desktopFlags.captureDesktop = true;

    std::cout << " -> Running Desktop Context...\n";
    results.push_back(runBenchmark("Subservice: Desktop Context Info", 100, 10, [&]() {
        builder.observe(desktopFlags);
        builder.clearState();
    }));

    // -------------------------------------------------------------
    // فاز ۲: پایپ‌لاین‌های ترکیبی و سناریوهای واقعی
    // -------------------------------------------------------------
    std::cout << "\n[2/3] Benchmarking Combined Pipelines...\n";

    // 2.1 Fast Path (No Vision, Active Window A11y + Desktop + Clipboard)
    ObservationFlags quickFlags{};
    quickFlags.captureVision = false;
    quickFlags.captureFullAccessibility = false;
    quickFlags.captureActiveWindowAccessibility = true;
    quickFlags.captureClipboard = true;
    quickFlags.captureDesktop = true;
    quickFlags.captureNewScreenMetrics = false;

    std::cout << " -> Running Fast Context Pipeline...\n";
    results.push_back(runBenchmark("Pipeline: Fast Context (Active Window)", OS_ITERS, OS_WARMUP, [&]() {
        builder.observe(quickFlags);
        builder.clearState();
    }));

    // 2.2 Full Multimodal Observation (Vision + Full A11y + Clipboard + Desktop + Metrics)
    ObservationFlags fullFlags{};
    fullFlags.captureVision = true;
    fullFlags.captureFullAccessibility = true;
    fullFlags.captureActiveWindowAccessibility = false;
    fullFlags.captureClipboard = true;
    fullFlags.captureDesktop = true;
    fullFlags.captureNewScreenMetrics = true;

    std::cout << " -> Running Full Multimodal Pipeline...\n";
    results.push_back(runBenchmark("Pipeline: Full Observation (All Flags)", OS_ITERS, OS_WARMUP, [&]() {
        builder.observe(fullFlags);
        builder.clearState();
    }));

    // -------------------------------------------------------------
    // فاز ۳: عملیات حافظه و رزولوشن کانتکست (Base64 + Prompt Assembly)
    // -------------------------------------------------------------
    std::cout << "\n[3/3] Benchmarking In-Memory Context Resolution...\n";

    builder.observe(fullFlags);
    WorldState stateForBenchmark = builder.consumeState();

    results.push_back(runBenchmark("Resolution: LLM Context Formatting", 100, 10, [&]() {
        WorldState testCopy = stateForBenchmark;
        testCopy.getFootnotes();
        testCopy.getUploadList();
    }));

    printResultTable(results);

#ifdef _WIN32
    CoUninitialize();
#endif

    return 0;
}