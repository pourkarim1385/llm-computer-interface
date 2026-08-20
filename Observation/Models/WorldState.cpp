#include "WorldState.h"
#include <algorithm>
#include <unordered_map>

namespace {
    static const char BASE64_CHARS[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
}

std::string WorldState::encode_base64_raw(const unsigned char* data, size_t size) {
    if (size == 0 || data == nullptr) return "";

    std::string encoded;
    encoded.reserve(((size + 2) / 3) * 4);

    int val = 0, valb = -6;
    for (size_t i = 0; i < size; ++i) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(BASE64_CHARS[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(BASE64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    return encoded;
}

std::string WorldState::encode_base64(const std::vector<unsigned char>& data) {
    return encode_base64_raw(data.data(), data.size());
}

std::string WorldState::encode_base64(const std::vector<std::byte>& data) {
    return encode_base64_raw(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

std::string WorldState::getMimeType(ImageFormat fmt) {
    switch (fmt) {
        case ImageFormat::JPG: return "image/jpeg";
        case ImageFormat::PNG: return "image/png";
        default:               return "image/png";
    }
}

std::string WorldState::getMimeType(const std::string& extension) {
    static const std::unordered_map<std::string, std::string> mimeMap = {
            {".png",  "image/png"},
            {".jpg",  "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".webp", "image/webp"},
            {".pdf",  "application/pdf"},
            {".mp3",  "audio/mp3"},
            {".wav",  "audio/wav"}
    };

    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    auto it = mimeMap.find(ext);
    if (it != mimeMap.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

void WorldState::resolve() {
    if (isResolved) return;
    isResolved = true;

    // ScreenMetrics
    if (screenMetricsState.getWidth() > 0 && screenMetricsState.getHeight() > 0) {
        footnotes.push_back("[Screen Context]: " + screenMetricsState.resolve());
    }

    // VisionState
    const auto& visionBytes = visionState.getContentBinary();
    if (!visionBytes.empty()) {
        uploadList.push_back({
                                     "screen",
                                     getMimeType(visionState.getFormat()),
                                     encode_base64(visionBytes)
                             });
    }

    // AccessibilityState
    std::string a11yJson = AccessibilityState::generateJsonPrompt(accessibilityState);
    if (!a11yJson.empty()) {
        footnotes.push_back("[Accessibility Tree]:\n" + a11yJson);
    }

    // ClipboardState
    for (const auto& item : clipboardState.items()) {
        if (item.isInline()) {
            footnotes.push_back("[Clipboard Text (" + item.format + ")]:\n" + *item.text);
            continue;
        }

        if (item.type == ClipboardDataType::FileList || !item.paths.empty()) {
            std::string fileListStr = "[Clipboard Copied Files]:\n";
            for (const auto& path : item.paths) {
                fileListStr += "- " + path + "\n";
            }
            footnotes.push_back(fileListStr);
            continue;
        }

        if (item.isDeferred()) {
            if (item.type == ClipboardDataType::Image) {
                std::vector<std::byte> bytes = item.resolve();
                if (!bytes.empty()) {
                    uploadList.push_back({
                                                 "clipboard_image",
                                                 item.format.empty() ? "image/png" : item.format,
                                                 encode_base64(bytes)
                                         });
                }
            }
            else if (item.type == ClipboardDataType::Audio) {
                std::vector<std::byte> bytes = item.resolve();
                if (!bytes.empty()) {
                    uploadList.push_back({
                                                 "clipboard_audio",
                                                 item.format.empty() ? "audio/wav" : item.format,
                                                 encode_base64(bytes)
                                         });
                }
            }
        }
    }

    // DesktopState
    footnotes.push_back("[System Desktop Context]: " + desktopState.resolve());

    // Appended FileContextSates
    for (const auto& file : appendedFiles) {
        std::string metaJson = file.getMetaDataJson();
        if (!metaJson.empty()) {
            footnotes.push_back("[Attached File Meta (" + file.getPath().filename().string() + ")]:\n" + metaJson);
        }

        //Skipping Directories
        if (file.getType() != FileType::RegularFile) {
            continue;
        }

        std::string ext = file.getExtension();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        size_t size = file.getFileSizeInBytes();

        //Text Files Handling
        if (FileContextState::isTextExtension(ext)) {
            if (size <= 500 * 1024) {
                std::string content = readFileContent(file.getPath());
                if (!content.empty()) {
                    footnotes.push_back("[Attached File Content: " + file.getPath().filename().string() + "]:\n```\n" + content + "\n```");
                }
            } else {
                footnotes.push_back("[Attached File Content: " + file.getPath().filename().string() + " is too large to inline (>500KB)]");
            }
            continue;
        }

        // Media Files Handling
        std::string mime = getMimeType(ext);
        if (mime != "application/octet-stream") {
            //Check file size
            if (size > 0 && size <= maxUploadSize) {
                std::vector<unsigned char> bytes = readFileBinary(file.getPath());
                if (!bytes.empty()) {
                    uploadList.push_back({
                                                 "attached_file:" + file.getPath().filename().string(),
                                                 mime,
                                                 encode_base64(bytes)
                                         });
                }
            }
        }
    }
}

const std::vector<MediaPayload>& WorldState::getUploadList() {
    if (!isResolved) resolve();
    return uploadList;
}

const std::vector<std::string>& WorldState::getFootnotes() {
    if (!isResolved) resolve();
    return footnotes;
}

void WorldState::appendFile(const FileContextState &fcs) {
    appendedFiles.push_back(fcs);
}

std::vector<unsigned char> WorldState::readFileBinary(const std::filesystem::path& p) {
    std::ifstream file(p, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }
    return {};
}

std::string WorldState::readFileContent(const std::filesystem::path& p) {
    std::ifstream file(p, std::ios::binary);
    if (!file.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}