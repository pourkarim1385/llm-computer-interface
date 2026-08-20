#include <iostream>

#include "Observation/Models/WorldState.h"
#include "Observation/Services/WorldStateBuilderService.h"
#include "Observation/Public.h"

using namespace std;

std::ostream& operator<<(std::ostream& os, ClipboardDataType type) {
    switch (type) {
        case ClipboardDataType::Text:     return os << "Text";
        case ClipboardDataType::Html:     return os << "Html";
        case ClipboardDataType::Rtf:      return os << "Rtf";
        case ClipboardDataType::Image:    return os << "Image";
        case ClipboardDataType::FileList: return os << "FileList";
        case ClipboardDataType::Audio:    return os << "Audio";
        case ClipboardDataType::Binary:   return os << "Binary";
        case ClipboardDataType::Video:    return os << "Video";
        default:                          return os << "Unknown";
    }
}

int main(int argc, const char * argv[]) {
    try {
        WorldStateBuilderService& service = WorldStateBuilderService::getInstance();
        service.observe();
        WorldState state = service.consumeState();
        for(auto& line : state.getFootnotes())
            std::cout << line << std::endl;
        for(auto& line : state.getUploadList())
            std::cout << line.source << " " << line.mimeType << std::endl;

    }
    catch (...) {
        std::cout << "An exception occured!\n";
    }
}