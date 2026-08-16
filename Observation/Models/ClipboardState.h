#ifndef ACCESSIBILITYSERVICE_CLIPBOARDSTATE_H
#define ACCESSIBILITYSERVICE_CLIPBOARDSTATE_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <ostream>

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

using DataLoader = std::function<std::vector<std::byte>()>;

class ClipboardItem {
public:
    ClipboardDataType type = ClipboardDataType::Unknown;
    std::string format; // native/MIME format name, for debugging & LLM hints
    std::size_t byteSize = 0; // size of the underlying payload

    //Inline path - light
    std::optional<std::string> text;

    //Deferred path - heavy
    std::vector<std::string> paths; // set for FileList
    DataLoader loader; // set for Image/Audio/Binary

    bool isInline()   const noexcept { return text.has_value(); }
    bool isDeferred() const noexcept { return static_cast<bool>(loader); }

    std::vector<std::byte> resolve() const;
    static ClipboardItem makeText(ClipboardDataType t, std::string value, std::string fmt);
    static ClipboardItem makeDeferred(ClipboardDataType t, std::string fmt, std::size_t size, DataLoader ld);
    static ClipboardItem makeFileList(std::vector<std::string> files, std::size_t totalSize);
    static DataLoader makeFileLoader(const std::string& path);
    ClipboardDataType getType() const;
};

class ClipboardState {
private:
    std::vector<ClipboardItem> items_;
    std::map<ClipboardDataType, std::size_t> counts_;
    std::size_t totalBytes_ = 0;
public:
    ClipboardState() = default;

    //used by Service
    void addItem(ClipboardItem item) {
        counts_[item.type] += 1;
        totalBytes_ += item.byteSize;
        items_.push_back(std::move(item));
    }

    std::size_t count() const noexcept { return counts_.size(); }
    std::size_t countOf(const ClipboardDataType t) const {
        const auto it = counts_.find(t);
        return it == counts_.end() ? 0 : it->second;
    }
    bool has(const ClipboardDataType t) const { return countOf(t) > 0; }
    bool empty() const noexcept { return items_.empty(); }
    std::size_t totalBytes() const noexcept { return totalBytes_; }

    std::vector<ClipboardDataType> presentTypes() const;
    const std::vector<ClipboardItem>& items() const noexcept { return items_; }
};

#endif //ACCESSIBILITYSERVICE_CLIPBOARDSTATE_H