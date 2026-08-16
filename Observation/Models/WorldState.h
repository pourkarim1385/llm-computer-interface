#ifndef ACCESSIBILITYSERVICE_WORLDSTATE_H
#define ACCESSIBILITYSERVICE_WORLDSTATE_H

#include <utility>
#include <string>

#include "AccessibilityState.h"
#include "ClipboardState.h"
#include "VisionState.hpp"

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

class WorldState {
private:
    AccessibilityState accessibilityState;
    ClipboardState clipboardState;
    VisionState visionState;
    bool isResovled;

    std::vector<std::string> uploadList;
    std::vector<std::string> footnotes;

    void resovle();

    std::string encode_base64(const std::vector<unsigned char>& data);
public:
    explicit WorldState(AccessibilityState _aS, ClipboardState _cS, VisionState _vS) : accessibilityState(_aS) , clipboardState(std::move(_cS)) ,
                                                                                       visionState(_vS) , isResovled(false) {}
    ~WorldState() = default;

    std::vector<std::string> getUploadList() const;
    std::vector<std::string> getFootnotes() const;
};


#endif //ACCESSIBILITYSERVICE_WORLDSTATE_H
