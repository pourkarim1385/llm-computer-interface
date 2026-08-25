#include <iostream>

#include "Observation/Models/WorldState.h"
#include "Observation/Services/WorldStateBuilderService.h"
#include "Observation/Public.h"

using namespace std;

string IntegratedDetail(const vector <string>& Detail){
    string integratedDetail = "";
    for (auto& str : Detail){
        integratedDetail += str;
    }return integratedDetail;
}

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

        // Systemm Data manging.
        vector <string> Detail = state.getFootnotes();
        string integratedDetail = IntegratedDetail(Detail);
        ofstream outputFile("detail.txt");
    
        if (!outputFile.is_open()) {
            std::cerr << "Unable to open output file.\n";
        } else {
            outputFile << integratedDetail;
            outputFile.close();
            std::cout << "Integrated detail saved successfully.\n";
        }


        // for(auto& line : Detail)
        //     std::cout << line << std::endl;
        // for(auto& line : state.getUploadList())
        //     std::cout << line.source << " " << line.mimeType << std::endl;

    }
    catch (...) {
        std::cout << "An exception occured!\n";
    }
}