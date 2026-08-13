#include <iostream>

#include "Observation/Services/AccessibilityService.h"
#include "Observation/Models/AccessibilityState.h"
#include "Observation/Services/CaptureClipboardService.h"
#include "Observation/Models/ClipboardState.h"
#include "Observation/Services/FileContextExtractorService.h"

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
        auto& service1 = AccessibilityService::getInstance();
        auto& service2 = CaptureClipboardService::getInstance();
        auto& service3 = FileContextExtractorService::getInstance();
        //const AccessibilityState state = service1.captureForegroundWindowState();
        //const ClipboardState state2 = service2.getCurrentState();
        string randomPath;
        std::getline(std::cin, randomPath);
        if (!randomPath.empty() && randomPath.back() == '\r')
            randomPath.pop_back();

        const FileContextState state3 = service3.getCurrentState(randomPath);

        cout << state3.getMetaDataJson() << endl;
        //std::cout << AccessibilityState::generateJsonPrompt(state) << "\n";
        //for (auto item : state2.presentTypes()) {
        //    std::cout << item << "\n";
        //}
    } catch (const AccessibilityException& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    catch (const FileContextException& e) {
        std::cerr << e.what() << "\n";
    }
    catch (...) {
        std::cout << "An unknown exception occured!\n";
    }
}

//TODO:
/**
* سه نکته که ارزش نگاه‌کردن دارند:

۱. "file_size":0 برای directory

برای مسیرهای غیر regular_file اندازه معنا ندارد. الان 0 می‌فرستی که برای LLM شبیه «پوشه خالی» تفسیر می‌شود. بهتر است null باشد:

cpp
if (type == FileType::regular_file)
    oss << "\"file_size\":" << fileSize << ",";
else
    oss << "\"file_size\":null,";
۲. "permission":"rwxrwxrwx"

این مقدار روی ویندوز واقعی نیست — std::filesystem روی NTFS فقط read-only bit را می‌بیند و بقیه را ثابت پر می‌کند. یعنی همیشه rwxrwxrwx یا r-xr-xr-x می‌گیری، مستقل از ACL واقعی. اگر تصمیم‌گیری Agent به این فیلد وابسته می‌شود، گمراه‌کننده است. دو گزینه:

فیلد را نگه دار ولی در مستندات schema علامت بزن که روی ویندوز approximate است.
یا فقط یک flag ساده‌تر و صادقانه‌تر بفرست: "writable": true/false.
۳. timezone در last_modified

پسوند Z یعنی UTC. مطمئن شو در fileTimeToIso8601 از gmtime_s استفاده کرده‌ای نه localtime_s. با ساعت 17:23:43 و توجه به اینکه Iran از UTC جلوتر است، اگر localtime باشد داری زمان محلی را با برچسب UTC می‌فرستی — یک باگ بی‌صدا که فقط وقتی روی سرور با TZ دیگری اجرا کنی خودش را نشان می‌دهد.

بقیه‌اش تمیز است.
 */