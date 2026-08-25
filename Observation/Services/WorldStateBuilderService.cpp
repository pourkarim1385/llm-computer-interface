#include "WorldStateBuilderService.h"

// Include your service headers
#include "AccessibilityService.h"
#include "CaptureClipboardService.h"
#include "CaptureService.hpp"
#include "FileContextExtractorService.h"
#include "ScreenMetricsService.h"
#include "CaptureDesktopService.hpp"

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
        currentDesktop = DesktopService::getInstance().getCurrentState();
    }

    currentMetrics = ScreenMetricsService::getInstance().getCurrentState(flags.captureNewScreenMetrics);
}

bool WorldStateBuilderService::fileAnalyzeRequest(const std::string& targetPath, const fileIncludeFilter targetFilter) {
    std::lock_guard<std::mutex> lock(stateMutex);

    try {
        FileContextState fileState = FileContextExtractorService::getInstance().getCurrentState(targetPath, targetFilter);
        appendedFiles.push_back(fileState);
        return true;
    }
    catch (const FileContextException& e) {
        pushActionResult("[FileAnalyze Failed] Reading context failed: " + std::string(e.what()));
        return false;
    }
    catch (const std::exception& e) {
        pushActionResult("[FileAnalyze Failed] System/Standard error: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        pushActionResult("[FileAnalyze Failed] Fatal: Unknown unhandled exception");
        return false;
    }
}

void WorldStateBuilderService::pushActionResult(const std::string &msg) {
    actionResults.push_back(msg);
}

WorldState WorldStateBuilderService::consumeState() {
    std::lock_guard<std::mutex> lock(stateMutex);

    // 1. Build the final object using the constructor
    // Note: Swapping ScreenMetrics for DesktopState when ready
    WorldState finalState(currentAccessibility, currentClipboard, currentVision, currentMetrics, currentDesktop);

    // 2. Append all accumulated files
    for (const auto& file : appendedFiles) {
        finalState.appendFile(file);
    }

    for (const auto& result : actionResults){
        finalState.appendActionResult(result);
    }

    // 3. Clear our ingredients for the next cycle (re-assigning to defaults)
    currentAccessibility = AccessibilityState();
    currentVision = VisionState();
    currentClipboard = ClipboardState();
    currentDesktop = DesktopState();
    currentMetrics = ScreenMetricsState();
    appendedFiles.clear();
    actionResults.clear();

    // 4. Return the built state
    return finalState;
}

void WorldStateBuilderService::clearState() {
    std::lock_guard<std::mutex> lock(stateMutex);

    // Reset everything to default
    currentAccessibility = AccessibilityState();
    currentVision = VisionState();
    currentClipboard = ClipboardState();
    currentMetrics = ScreenMetricsState();
    currentDesktop = DesktopState();
    appendedFiles.clear();
    actionResults.clear();
}