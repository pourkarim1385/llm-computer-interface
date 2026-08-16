#ifndef ACCESSIBILITYSERVICE_WORLDSTATE_H
#define ACCESSIBILITYSERVICE_WORLDSTATE_H

#include <utility>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

#include "AccessibilityState.h"
#include "ClipboardState.h"
#include "VisionState.hpp"
#include "FileContextState.h"
#include "../Public.h"

struct MediaPayload {
    std::string source;
    std::string mimeType;
    std::string base64;
};

struct ScreenMetrics {
    int width{0};
    int height{0};
    float scaleFactor{1.0f}; //Managing DPI Scaling
};

class WorldState {
private:
    AccessibilityState accessibilityState;
    ClipboardState clipboardState;
    VisionState visionState;
    ScreenMetrics screenMetrics;
    std::vector<FileContextState> appendedFiles;
    static const size_t maxUploadSize = 20 * 1024 * 1024;

    bool isResolved{false};

    std::vector<MediaPayload> uploadList;
    std::vector<std::string> footnotes;


    void resolve();

    static std::string encode_base64_raw(const unsigned char* data, size_t size);
    static std::string encode_base64(const std::vector<unsigned char>& data);
    static std::string encode_base64(const std::vector<std::byte>& data);
    static std::string getMimeType(ImageFormat fmt);
    static std::string getMimeType(const std::string& extension);
    static std::vector<unsigned char> readFileBinary(const std::filesystem::path& p);
    static std::string readFileContent(const std::filesystem::path& p);

public:
    WorldState() = default;
    explicit WorldState(AccessibilityState aS, ClipboardState cS, VisionState vS, ScreenMetrics metrics = {})
            : accessibilityState(std::move(aS)),
              clipboardState(std::move(cS)),
              visionState(std::move(vS)),
              screenMetrics(metrics),
              isResolved(false) {}

    ~WorldState() = default;

    const std::vector<MediaPayload>& getUploadList();
    const std::vector<std::string>& getFootnotes();

    const AccessibilityState& getAccessibilityState() const { return accessibilityState; }
    const ScreenMetrics& getScreenMetrics() const { return screenMetrics; }

    void appendFile(const FileContextState& fcs);
};

#endif // ACCESSIBILITYSERVICE_WORLDSTATE_H