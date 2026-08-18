#include "WorldStateBuilderService.h"

// Include your service headers
#include "AccessibilityService.h"
#include "CaptureClipboardService.h"
#include "CaptureService.hpp"
#include "FileContextExtractorService.h"
// #include "DesktopService.h"

void WorldStateBuilderService::observe(const ObservationFlags& flags) {
    std::lock_guard<std::mutex> lock(stateMutex);

    if (flags.captureFullAccessibility) {
        currentAccessibility = AccessibilityService::getInstance().captureFullState();
    }

    if(flags.captureActiveWindowAccessibility){
        currentAccessibility = AccessibilityService::getInstance().captureForegroundWindowState();
    }

    if(flags.captureTargetWindowAccessibility){
        currentAccessibility = AccessibilityService::getInstance().captureTargetWindowState(flags.targetWindow);
    }

    if (flags.captureVision) {
        currentVision = CaptureService::getInstance().capture();
    }

    if (flags.captureClipboard) {
        currentClipboard = CaptureClipboardService::getInstance().getCurrentState();
    }

    if (flags.captureDesktop) {
        // currentDesktop = DesktopService::getInstance().getCurrentState();
        // currentMetrics = currentDesktop.getMetrics(); // Or however it will be accessed
    }
}

bool WorldStateBuilderService::fileAnalyzeRequest(const std::string& targetPath, const fileIncludeFilter targetFilter) {
    std::lock_guard<std::mutex> lock(stateMutex);

    // Assuming FileContextExtractorService has a method like extractContext(path)
    // that returns an optional or throws on failure. Adjust based on your actual method!
    try {
        FileContextState fileState = FileContextExtractorService::getInstance().getCurrentState(targetPath, targetFilter);
        appendedFiles.push_back(fileState);
        return true;
    }
    catch (FileContextException& e) {
        //
        return false;
    } catch (...) {
        // You can log the error here or append a "failed to read" FileContextState
        return false;
    }
}

WorldState WorldStateBuilderService::consumeState() {
    std::lock_guard<std::mutex> lock(stateMutex);

    // 1. Build the final object using the constructor
    // Note: Swapping ScreenMetrics for DesktopState when ready
    WorldState finalState(currentAccessibility, currentClipboard, currentVision, currentMetrics);

    // 2. Append all accumulated files
    for (const auto& file : appendedFiles) {
        finalState.appendFile(file);
    }

    // 3. Clear our ingredients for the next cycle (re-assigning to defaults)
    currentAccessibility = AccessibilityState();
    currentVision = VisionState();
    currentClipboard = ClipboardState();
    // currentDesktop = DesktopState();
    currentMetrics = ScreenMetrics();
    appendedFiles.clear();

    // 4. Return the built state
    return finalState;
}

void WorldStateBuilderService::clearState() {
    std::lock_guard<std::mutex> lock(stateMutex);

    // Reset everything to default
    currentAccessibility = AccessibilityState();
    currentVision = VisionState();
    currentClipboard = ClipboardState();
    currentMetrics = ScreenMetrics();
    appendedFiles.clear();
}