#include "FileContextExtractorService.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace {
    FileType toFileType(const fs::path& p) {
        std::error_code ec;
        auto s = fs::status(p, ec);
        if (ec) return FileType::Unknown;
        if (fs::is_directory(s)) return FileType::Directory;
        if (fs::is_regular_file(s)) return FileType::RegularFile;
        return FileType::Unknown;
    }

    std::vector<FileSystemEntry> listDir(const fs::path& dir, const fs::path& exclude) {
        std::vector<FileSystemEntry> result;
        std::error_code ec;
        for (const auto& entry : fs::directory_iterator(dir, ec)) {
            if (ec) break;
            if (!exclude.empty() && entry.path().filename() == exclude.filename()) continue;
            FileSystemEntry e;
            e.path      = entry.path();
            e.type      = toFileType(entry.path());
            e.extension = (e.type == FileType::RegularFile) ? entry.path().extension().string() : "";
            result.push_back(std::move(e));
        }
        return result;
    }
}


FileContextExtractorService &FileContextExtractorService::getInstance() {
    static FileContextExtractorService instance;
    return instance;
}

bool FileContextExtractorService::isValidPath(const std::string& path) {
    std::error_code ec;
    return fs::exists(fs::path(path), ec) && !ec;
}

FileType FileContextExtractorService::typeDetection(const std::string& path) {
    return toFileType(fs::path(path));
}

std::string FileContextExtractorService::extensionDetection(const std::string& path) {
    if (typeDetection(path) != FileType::RegularFile) return "";
    return fs::path(path).extension().string();
}

std::vector<FileSystemEntry> FileContextExtractorService::neighboursListing(const std::string& path) {
    const fs::path target(path);
    fs::path parent = target.parent_path();
    if (parent.empty()) parent = ".";
    return listDir(parent, target);
}

std::vector<FileSystemEntry> FileContextExtractorService::entriesTreeListing(const std::string& path) {
    if (typeDetection(path) != FileType::Directory) return {};
    return listDir(fs::path(path), {});
}

bool FileContextExtractorService::filter(FileSystemEntry& file) {
    //TODO:
    //must be implemented when the repo of filters is ready
    //using a loop to iterate all of filters and filtering
}

FileContextState FileContextExtractorService::getCurrentState(const std::string& filePath) {
    namespace fs = std::filesystem;
    std::error_code ec;

    const fs::path p{filePath};

    if (!isValidPath(filePath)) {
        throw FileContextException("Invalid Path");
    }

    const FileType type = typeDetection(filePath);
    std::string ext = extensionDetection(filePath);

    size_t size = 0;
    if (type == FileType::RegularFile) {
        const auto s = fs::file_size(p, ec);
        if (!ec) size = static_cast<size_t>(s);
        ec.clear();
    }

    auto writeTime = fs::last_write_time(p, ec);
    if (ec) { writeTime = {}; ec.clear(); }

    auto perms = fs::perms::none;
    const auto st = fs::status(p, ec);
    if (!ec) perms = st.permissions();
    ec.clear();

    FileContextState state(p, type, std::move(ext), size, writeTime, perms);

    for (auto& n : neighboursListing(filePath)) {
        state.addNeighbor(std::move(n));
    }

    if (type == FileType::Directory) {
        for (auto& e : entriesTreeListing(filePath)) {
            state.addEntry(std::move(e));
        }
    }

    return state;
}
