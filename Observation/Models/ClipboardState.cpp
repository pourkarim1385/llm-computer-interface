#include "ClipboardState.h"

#include <fstream>

std::vector<std::byte> ClipboardItem::resolve() const {
        if (loader) return loader();
        if (text) {
            const auto* p = reinterpret_cast<const std::byte*>(text->data());
            return std::vector<std::byte>(p, p + text->size());
        }
        return {};
}

ClipboardItem ClipboardItem::makeText(ClipboardDataType t, std::string value, std::string fmt) {
    ClipboardItem it;
    it.type     = t;
    it.format   = std::move(fmt);
    it.byteSize = value.size();
    it.text     = std::move(value);
    return it;
}

ClipboardItem ClipboardItem::makeDeferred(ClipboardDataType t, std::string fmt, std::size_t size, DataLoader ld) {
    ClipboardItem it;
    it.type     = t;
    it.format   = std::move(fmt);
    it.byteSize = size;
    it.loader   = std::move(ld);
    return it;
}

ClipboardItem ClipboardItem::makeFileList(std::vector<std::string> files, std::size_t totalSize) {
    ClipboardItem it;
    it.type     = ClipboardDataType::FileList;
    it.format   = "file-list";
    it.byteSize = totalSize;
    it.paths    = std::move(files);
    return it;
}

std::vector<ClipboardDataType> ClipboardState::presentTypes() const {
    std::vector<ClipboardDataType> out;
    out.reserve(counts_.size());
    for (const auto& [type, n] : counts_)
        if (n > 0) out.push_back(type);
    return out;
}

DataLoader ClipboardItem::makeFileLoader(const std::string& path) {
    return [path]() -> std::vector<std::byte> {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
            return {};

        const auto size = file.tellg();
        if (size <= 0)
            return {};

        file.seekg(0);

        std::vector<std::byte> buffer(static_cast<size_t>(size));
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    };
}

ClipboardDataType ClipboardItem::getType() const {
    return type
}