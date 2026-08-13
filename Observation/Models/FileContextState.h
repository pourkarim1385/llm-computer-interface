#ifndef ACCESSIBILITYSERVICE_FILECONTEXTSTATE_H
#define ACCESSIBILITYSERVICE_FILECONTEXTSTATE_H
#include <filesystem>
#include <optional>
#include <vector>

enum class FileType
{
    RegularFile,
    Directory,
    Unknown
};

struct FileSystemEntry
{
    std::filesystem::path path;
    FileType type;
    std::string extension;

    std::string toJson() const;
};

class FileContextState {
private:
    std::filesystem::path path;
    FileType type{FileType::Unknown};
    std::string extension;
    size_t fileSizeInBytes{0};

    std::filesystem::file_time_type lastModifiedTime;
    std::filesystem::perms permission;


    // TODO: Add filter and summarization policies

    std::vector<FileSystemEntry> neighbors;
    std::vector<FileSystemEntry> entries; // Tree with height of 1 (only for Directory)

    static std::string permissionsToString(std::filesystem::perms p);
public:
    // Constructor with move semantics
    FileContextState(std::filesystem::path p,
                     FileType t,
                     std::string ext,
                     size_t size = 0,
                     std::filesystem::file_time_type writeTime = {},
                     std::filesystem::perms perms = std::filesystem::perms::none)
        : path(std::move(p)),
          type(t),
          extension(std::move(ext)),
          fileSizeInBytes(size),
          lastModifiedTime(writeTime),
          permission(perms) {}

    // Setters / Modifiers
    void addEntry(FileSystemEntry entry) { entries.push_back(std::move(entry)); }
    void addNeighbor(FileSystemEntry entry) { neighbors.push_back(std::move(entry)); }

    // Getters
    [[nodiscard]] const std::filesystem::path& getPath() const noexcept { return path; }
    [[nodiscard]] FileType getType() const noexcept { return type; }
    [[nodiscard]] const std::string& getExtension() const noexcept { return extension; }
    [[nodiscard]] size_t getFileSizeInBytes() const noexcept { return fileSizeInBytes; }
    [[nodiscard]] std::filesystem::file_time_type getLastWriteTime() const noexcept { return lastModifiedTime; }
    [[nodiscard]] std::filesystem::perms getPermissions() const noexcept { return permission; }

    [[nodiscard]] const std::vector<FileSystemEntry>& getNeighbors() const noexcept { return neighbors; }
    [[nodiscard]] const std::vector<FileSystemEntry>& getEntries() const noexcept { return entries; }

    // JSON serialization for LLM prompt context
    [[nodiscard]] std::string getMetaDataJson() const;
};


#endif //ACCESSIBILITYSERVICE_FILECONTEXTSTATE_H