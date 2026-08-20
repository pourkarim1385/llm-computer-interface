#ifndef WORLDSTATEBUILDERSERVICE_H
#define WORLDSTATEBUILDERSERVICE_H

#include <string>
#include <vector>
#include <mutex>
#include "../Models/WorldState.h"

// Included based on your service list
#include "../Models/AccessibilityState.h"
#include "../Models/VisionState.hpp"
#include "../Models/ClipboardState.h"
#include "../Models/FileContextState.h"
#include "FileContextExtractorService.h"
#include "../Models/ScreenMetricsState.h"
#include "DesktopState.h"

struct ObservationFlags {
    bool captureVision{true};
    bool captureFullAccessibility{true};
    bool captureActiveWindowAccessibility{false};
    bool captureTargetWindowAccessibility{false};
    std::string targetWindow;
    bool captureClipboard{true};
    bool captureDesktop{true}; // For the upcoming DesktopService
    bool captureNewScreenMetrics{false};
};

class WorldStateBuilderService {
private:
    std::mutex stateMutex;

    AccessibilityState currentAccessibility;
    VisionState currentVision;
    ClipboardState currentClipboard;
    DesktopState currentDesktop;
    ScreenMetricsState currentMetrics;

    std::vector<FileContextState> appendedFiles;

    WorldStateBuilderService() = default;
    ~WorldStateBuilderService() = default;

    WorldStateBuilderService(const WorldStateBuilderService&) = delete;
    WorldStateBuilderService& operator=(const WorldStateBuilderService&) = delete;

public:
    static WorldStateBuilderService& getInstance() {
        static WorldStateBuilderService instance;
        return instance;
    }

    void observe(const ObservationFlags& flags = ObservationFlags{});
    bool fileAnalyzeRequest(const std::string& targetPath, const fileIncludeFilter targetFilter = fileIncludeFilter());

    WorldState consumeState();
    void clearState();
};

#endif // WORLDSTATEBUILDERSERVICE_H