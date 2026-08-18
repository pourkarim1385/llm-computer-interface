#ifndef ACCESSIBILITYSERVICE_PUBLIC_H
#define ACCESSIBILITYSERVICE_PUBLIC_H

enum class FileType
{
    RegularFile,
    Directory,
    Unknown
};

enum class ClipboardDataType {
    Text,      // plain UTF-8 text  -> kept inline in RAM
    Html,      // html fragment      -> inline
    Rtf,       // rich text          -> inline
    Image,     // bitmap/png/etc     -> deferred (spooled)
    FileList,  // dropped file paths -> metadata only, files already on disk
    Audio,     // audio blob         -> deferred
    Binary,    // opaque bytes       -> deferred
    Video,     //will be skipped in sending to llm
    Unknown
};

enum class ImageFormat { JPG, PNG };

struct ScreenMetrics {
    int width{0};
    int height{0};
    float scaleFactor{1.0f}; //Managing DPI Scaling
};

#endif //ACCESSIBILITYSERVICE_PUBLIC_H
